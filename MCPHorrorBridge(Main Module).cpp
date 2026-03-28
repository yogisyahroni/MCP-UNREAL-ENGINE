#include "MCPHorrorBridge.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "HttpServerResponse.h"
#include "JsonObjectConverter.h"
#include "Engine/World.h"
#include "Editor.h"
#include "UnrealEd.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeComponent.h"
#include "ExponentialHeightFog.h"
#include "VolumetricCloudComponent.h"
#include "SkyAtmosphereComponent.h"
#include "PostProcessVolume.h"
#include "DirectionalLight.h"
#include "SkyLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "TriggerBox.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SoftObjectPtr.h"

#define LOCTEXT_NAMESPACE "FMCPHorrorBridgeModule"

void FMCPHorrorBridgeModule::StartupModule()
{
    // Start HTTP Server
    FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
    HttpRouter = HttpServerModule.GetHttpRouter(ServerPort);
    
    if (HttpRouter.IsValid())
    {
        // Register Routes
        HttpRouter->BindRoute(FHttpPath("/api/status"), EHttpServerRequestVerbs::VERB_GET,
            FHttpRequestHandler::CreateRaw(this, &FMCPHorrorBridgeModule::HandleStatusRequest));
        
        HttpRouter->BindRoute(FHttpPath("/api/terrain"), EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateRaw(this, &FMCPHorrorBridgeModule::HandleTerrainRequest));
        
        HttpRouter->BindRoute(FHttpPath("/api/atmosphere"), EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateRaw(this, &FMCPHorrorBridgeModule::HandleAtmosphereRequest));
        
        HttpRouter->BindRoute(FHttpPath("/api/player/mechanics"), EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateRaw(this, &FMCPHorrorBridgeModule::HandlePlayerMechanicsRequest));
        
        HttpRouter->BindRoute(FHttpPath("/api/horror/hide"), EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateRaw(this, &FMCPHorrorBridgeModule::HandleHideMechanicsRequest));
        
        HttpRouter->BindRoute(FHttpPath("/api/horror/jumpscare"), EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateRaw(this, &FMCPHorrorBridgeModule::HandleJumpScareRequest));
        
        HttpRouter->BindRoute(FHttpPath("/api/horror/inventory"), EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateRaw(this, &FMCPHorrorBridgeModule::HandleInventoryRequest));
        
        HttpRouter->BindRoute(FHttpPath("/api/horror/enemy"), EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateRaw(this, &FMCPHorrorBridgeModule::HandleEnemyAIRequest));
        
        HttpRouter->BindRoute(FHttpPath("/api/audio"), EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateRaw(this, &FMCPHorrorBridgeModule::HandleAudioRequest));
        
        HttpServerModule.StartAllListeners();
        bIsServerRunning = true;
        
        UE_LOG(LogTemp, Log, TEXT("MCP Horror Bridge Server started on port %d"), ServerPort);
    }
}

void FMCPHorrorBridgeModule::ShutdownModule()
{
    if (bIsServerRunning)
    {
        FHttpServerModule::Get().StopAllListeners();
        bIsServerRunning = false;
    }
}

// ============================================================================
// ROUTE HANDLERS
// ============================================================================

bool FMCPHorrorBridgeModule::HandleStatusRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField("status", "running");
    JsonObject->SetStringField("engine_version", FEngineVersion::Current().ToString());
    JsonObject->SetNumberField("port", ServerPort);
    
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (World)
    {
        JsonObject->SetStringField("current_level", World->GetMapName());
        JsonObject->SetNumberField("actor_count", World->GetActorCount());
    }
    
    return SendSuccessResponse(OnComplete, "MCP Horror Bridge is running", JsonObject);
}

bool FMCPHorrorBridgeModule::HandleTerrainRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
    TSharedPtr<FJsonObject> JsonObject = ParseJsonBody(Request.Body);
    if (!JsonObject.IsValid())
    {
        return SendErrorResponse(OnComplete, "Invalid JSON body");
    }
    
    FString TerrainType = JsonObject->GetStringField("terrain_type");
    FString Name = JsonObject->GetStringField("name");
    int32 SizeKm = JsonObject->GetIntegerField("size_km");
    float DarknessLevel = JsonObject->GetNumberField("darkness_level");
    
    // Calculate landscape size
    int32 ComponentsX = SizeKm * 8; // 8 components per km
    int32 ComponentsY = SizeKm * 8;
    int32 SectionsPerComponent = 2;
    int32 QuadsPerSection = 63;
    int32 QuadsPerComponent = SectionsPerComponent * QuadsPerSection;
    int32 SizeX = ComponentsX * QuadsPerComponent + 1;
    int32 SizeY = ComponentsY * QuadsPerComponent + 1;
    
    // Generate heightmap
    TArray<uint16> HeightData = GenerateProceduralHeightmap(TerrainType, SizeX, SizeY);
    
    // Create landscape
    ALandscape* Landscape = CreateLandscapeActor(Name, HeightData, ComponentsX, ComponentsY);
    
    if (Landscape)
    {
        // Setup PCG for asset placement
        SetupPCGAssetPlacement(Landscape, TerrainType);
        
        TSharedPtr<FJsonObject> ResponseData = MakeShared<FJsonObject>();
        ResponseData->SetStringField("landscape_name", Name);
        ResponseData->SetNumberField("size_x", SizeX);
        ResponseData->SetNumberField("size_y", SizeY);
        ResponseData->SetNumberField("components_x", ComponentsX);
        ResponseData->SetNumberField("components_y", ComponentsY);
        
        return SendSuccessResponse(OnComplete, 
            FString::Printf(TEXT("%s terrain '%s' created successfully"), *TerrainType, *Name), 
            ResponseData);
    }
    
    return SendErrorResponse(OnComplete, "Failed to create landscape");
}

bool FMCPHorrorBridgeModule::HandleAtmosphereRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
    TSharedPtr<FJsonObject> JsonObject = ParseJsonBody(Request.Body);
    if (!JsonObject.IsValid())
    {
        return SendErrorResponse(OnComplete, "Invalid JSON body");
    }
    
    FString AtmosphereType = JsonObject->GetStringField("atmosphere_type");
    float DarknessLevel = JsonObject->GetNumberField("darkness_level");
    float FogDensity = JsonObject->GetNumberField("fog_density");
    
    // Setup lighting
    SetupHorrorLighting(DarknessLevel);
    
    // Setup fog
    FLinearColor FogColor(0.02f, 0.03f, 0.05f); // Dark blue-black
    if (AtmosphereType == "blood_moon")
    {
        FogColor = FLinearColor(0.1f, 0.02f, 0.02f); // Red tint
    }
    else if (AtmosphereType == "sickly")
    {
        FogColor = FLinearColor(0.05f, 0.1f, 0.05f); // Green tint
    }
    
    SetupExponentialFog(FogDensity, FogColor);
    SetupVolumetricFog();
    SetupPostProcessing();
    SetupSkyAtmosphere();
    
    return SendSuccessResponse(OnComplete, 
        FString::Printf(TEXT("Horror atmosphere '%s' applied"), *AtmosphereType));
}

bool FMCPHorrorBridgeModule::HandlePlayerMechanicsRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
    TSharedPtr<FJsonObject> JsonObject = ParseJsonBody(Request.Body);
    if (!JsonObject.IsValid())
    {
        return SendErrorResponse(OnComplete, "Invalid JSON body");
    }
    
    FString MechanicType = JsonObject->GetStringField("mechanic_type");
    FString CharacterName = JsonObject->GetStringField("character_name");
    
    // Find character in world
    UWorld* World = GEditor->GetEditorWorldContext().World();
    ACharacter* Character = nullptr;
    for (TActorIterator<ACharacter> It(World); It; ++It)
    {
        if (It->GetName().Contains(CharacterName))
        {
            Character = *It;
            break;
        }
    }
    
    if (!Character)
    {
        return SendErrorResponse(OnComplete, FString::Printf(TEXT("Character '%s' not found"), *CharacterName));
    }
    
    if (MechanicType == "crouch_prone")
    {
        float CrouchSpeed = JsonObject->GetNumberField("crouch_speed");
        float ProneSpeed = JsonObject->GetNumberField("prone_speed");
        float CrouchHeight = JsonObject->GetNumberField("crouch_height");
        float ProneHeight = JsonObject->GetNumberField("prone_height");
        
        SetupCrouchMechanics(Character, CrouchSpeed, CrouchHeight);
        SetupProneMechanics(Character, ProneSpeed, ProneHeight);
    }
    else if (MechanicType == "vault_mantle")
    {
        float MaxVaultHeight = JsonObject->GetNumberField("max_vault_height");
        bool bAllowMantle = JsonObject->GetBoolField("allow_mantle");
        
        SetupVaultMechanics(Character, MaxVaultHeight, bAllowMantle);
    }
    else if (MechanicType == "lean_peek")
    {
        float LeanAngle = JsonObject->GetNumberField("lean_angle");
        SetupLeanMechanics(Character, LeanAngle);
    }
    
    return SendSuccessResponse(OnComplete, 
        FString::Printf(TEXT("Player mechanics '%s' applied to %s"), *MechanicType, *CharacterName));
}

bool FMCPHorrorBridgeModule::HandleHideMechanicsRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
    TSharedPtr<FJsonObject> JsonObject = ParseJsonBody(Request.Body);
    if (!JsonObject.IsValid())
    {
        return SendErrorResponse(OnComplete, "Invalid JSON body");
    }
    
    FString HideType = JsonObject->GetStringField("hide_type");
    FString Name = JsonObject->GetStringField("name");
    bool bEnemyCanSearch = JsonObject->GetBoolField("properties").GetObject()->GetBoolField("enemy_can_search");
    
    FVector Location;
    Location.X = JsonObject->GetObjectField("location")->GetNumberField("x");
    Location.Y = JsonObject->GetObjectField("location")->GetNumberField("y");
    Location.Z = JsonObject->GetObjectField("location")->GetNumberField("z");
    
    CreateHideVolume(HideType, Location, Name, bEnemyCanSearch);
    
    return SendSuccessResponse(OnComplete, 
        FString::Printf(TEXT("Hide spot '%s' (%s) created"), *Name, *HideType));
}

bool FMCPHorrorBridgeModule::HandleJumpScareRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
    TSharedPtr<FJsonObject> JsonObject = ParseJsonBody(Request.Body);
    if (!JsonObject.IsValid())
    {
        return SendErrorResponse(OnComplete, "Invalid JSON body");
    }
    
    FString ScareType = JsonObject->GetStringField("scare_type");
    FString Name = JsonObject->GetStringField("name");
    
    FVector Location;
    Location.X = JsonObject->GetObjectField("trigger")->GetObjectField("location")->GetNumberField("x");
    Location.Y = JsonObject->GetObjectField("trigger")->GetObjectField("location")->GetNumberField("y");
    Location.Z = JsonObject->GetObjectField("trigger")->GetObjectField("location")->GetNumberField("z");
    
    float Radius = JsonObject->GetObjectField("trigger")->GetNumberField("radius");
    TSharedPtr<FJsonObject> Config = JsonObject->GetObjectField("config");
    
    CreateJumpScareTrigger(ScareType, Location, Radius, Config);
    
    return SendSuccessResponse(OnComplete, 
        FString::Printf(TEXT("Jump scare '%s' (%s) created"), *Name, *ScareType));
}

bool FMCPHorrorBridgeModule::HandleInventoryRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
    TSharedPtr<FJsonObject> JsonObject = ParseJsonBody(Request.Body);
    if (!JsonObject.IsValid())
    {
        return SendErrorResponse(OnComplete, "Invalid JSON body");
    }
    
    FString InventoryType = JsonObject->GetStringField("inventory_type");
    int32 SlotCount = JsonObject->GetIntegerField("slot_count");
    bool bAllowExamination = JsonObject->GetObjectField("features")->GetBoolField("examination");
    bool bAllowCombining = JsonObject->GetObjectField("features")->GetBoolField("combining");
    
    SetupInventorySystem(InventoryType, SlotCount, bAllowExamination, bAllowCombining);
    
    return SendSuccessResponse(OnComplete, 
        FString::Printf(TEXT("Inventory system '%s' with %d slots created"), *InventoryType, SlotCount));
}

bool FMCPHorrorBridgeModule::HandleEnemyAIRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
    TSharedPtr<FJsonObject> JsonObject = ParseJsonBody(Request.Body);
    if (!JsonObject.IsValid())
    {
        return SendErrorResponse(OnComplete, "Invalid JSON body");
    }
    
    FString EnemyType = JsonObject->GetStringField("enemy_type");
    FString Name = JsonObject->GetStringField("name");
    
    FVector Location;
    Location.X = JsonObject->GetObjectField("location")->GetNumberField("x");
    Location.Y = JsonObject->GetObjectField("location")->GetNumberField("y");
    Location.Z = JsonObject->GetObjectField("location")->GetNumberField("z");
    
    float SightRange = JsonObject->GetObjectField("perception")->GetNumberField("sight_range");
    float HearingRange = JsonObject->GetObjectField("perception")->GetNumberField("hearing_range");
    float FOVAngle = JsonObject->GetObjectField("perception")->GetNumberField("fov");
    
    CreateHorrorEnemy(EnemyType, Location, Name, SightRange, HearingRange, FOVAngle);
    
    return SendSuccessResponse(OnComplete, 
        FString::Printf(TEXT("Enemy '%s' (%s) spawned"), *Name, *EnemyType));
}

bool FMCPHorrorBridgeModule::HandleAudioRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
    TSharedPtr<FJsonObject> JsonObject = ParseJsonBody(Request.Body);
    if (!JsonObject.IsValid())
    {
        return SendErrorResponse(OnComplete, "Invalid JSON body");
    }
    
    FString AudioType = JsonObject->GetStringField("audio_type");
    
    if (AudioType == "dynamic_music")
    {
        SetupDynamicMusicSystem();
    }
    else if (AudioType == "ambient")
    {
        FString AmbientType = JsonObject->GetStringField("ambient_type");
        SetupAmbientSounds(AmbientType);
    }
    
    return SendSuccessResponse(OnComplete, 
        FString::Printf(TEXT("Audio system '%s' configured"), *AudioType));
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

bool FMCPHorrorBridgeModule::SendSuccessResponse(const FHttpResultCallback& OnComplete, 
    const FString& Message, TSharedPtr<FJsonObject> Data)
{
    TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
    Response->SetStringField("status", "success");
    Response->SetStringField("message", Message);
    
    if (Data.IsValid())
    {
        Response->SetObjectField("data", Data);
    }
    
    FString ResponseBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseBody);
    FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);
    
    OnComplete(FHttpServerResponse::Create(ResponseBody, "application/json"));
    return true;
}

bool FMCPHorrorBridgeModule::SendErrorResponse(const FHttpResultCallback& OnComplete, const FString& ErrorMessage)
{
    TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
    Response->SetStringField("status", "error");
    Response->SetStringField("message", ErrorMessage);
    
    FString ResponseBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseBody);
    FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);
    
    OnComplete(FHttpServerResponse::Create(ResponseBody, "application/json"));
    return false;
}

TSharedPtr<FJsonObject> FMCPHorrorBridgeModule::ParseJsonBody(const FString& Body)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        return JsonObject;
    }
    
    return nullptr;
}

// ============================================================================
// TERRAIN GENERATION
// ============================================================================

TArray<uint16> FMCPHorrorBridgeModule::GenerateProceduralHeightmap(const FString& TerrainType, int32 Width, int32 Height)
{
    TArray<uint16> HeightData;
    HeightData.SetNum(Width * Height);
    
    const int32 SeaLevel = 32768;
    
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            float NormalizedX = (float)X / (float)Width;
            float NormalizedY = (float)Y / (float)Height;
            float HeightValue = 0.0f;
            
            if (TerrainType == "mountain" || TerrainType == "horror_mountain")
            {
                // Multi-octave noise untuk pegunungan
                float Noise1 = FMath::PerlinNoise2D(FVector2D(NormalizedX * 4.0f, NormalizedY * 4.0f));
                float Noise2 = FMath::PerlinNoise2D(FVector2D(NormalizedX * 8.0f, NormalizedY * 8.0f)) * 0.5f;
                float Noise3 = FMath::PerlinNoise2D(FVector2D(NormalizedX * 16.0f, NormalizedY * 16.0f)) * 0.25f;
                
                HeightValue = (Noise1 + Noise2 + Noise3) / 1.75f;
                
                // Peaks di beberapa areas
                float PeakNoise = FMath::PerlinNoise2D(FVector2D(NormalizedX * 2.0f, NormalizedY * 2.0f));
                if (PeakNoise > 0.6f)
                {
                    HeightValue += PeakNoise * 0.5f;
                }
                
                // Jagged cliffs untuk horror
                float CliffNoise = FMath::PerlinNoise2D(FVector2D(NormalizedX * 32.0f, NormalizedY * 32.0f));
                if (FMath::Abs(CliffNoise) > 0.7f)
                {
                    HeightValue += CliffNoise * 0.3f;
                }
            }
            else if (TerrainType == "hills")
            {
                // Hills lebih smooth
                float Noise1 = FMath::PerlinNoise2D(FVector2D(NormalizedX * 2.0f, NormalizedY * 2.0f));
                float Noise2 = FMath::PerlinNoise2D(FVector2D(NormalizedX * 4.0f, NormalizedY * 4.0f)) * 0.5f;
                HeightValue = (Noise1 + Noise2) / 1.5f * 0.5f;
            }
            else if (TerrainType == "city")
            {
                // Flat dengan slight variation
                HeightValue = FMath::PerlinNoise2D(FVector2D(NormalizedX * 1.0f, NormalizedY * 1.0f)) * 0.1f;
            }
            
            HeightData[Y * Width + X] = FMath::Clamp(SeaLevel + (int32)(HeightValue * 32767.0f), 0, 65535);
        }
    }
    
    return HeightData;
}

ALandscape* FMCPHorrorBridgeModule::CreateLandscapeActor(const FString& Name, 
    const TArray<uint16>& HeightData, int32 ComponentsX, int32 ComponentsY)
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return nullptr;
    
    int32 SectionsPerComponent = 2;
    int32 QuadsPerSection = 63;
    int32 QuadsPerComponent = SectionsPerComponent * QuadsPerSection;
    int32 SizeX = ComponentsX * QuadsPerComponent + 1;
    int32 SizeY = ComponentsY * QuadsPerComponent + 1;
    
    TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
    HeightDataPerLayers.Add(FGuid(), HeightData);
    
    TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
    TArray<FLandscapeImportLayerInfo> MaterialImportLayers;
    MaterialLayerDataPerLayers.Add(FGuid(), MoveTemp(MaterialImportLayers));
    
    ALandscape* Landscape = World->SpawnActor<ALandscape>();
    Landscape->bCanHaveLayersContent = true;
    Landscape->SetActorLabel(FName(*Name));
    
    Landscape->Import(
        FGuid::NewGuid(),
        0, 0, SizeX - 1, SizeY - 1,
        SectionsPerComponent,
        QuadsPerSection,
        HeightDataPerLayers,
        nullptr,
        MaterialLayerDataPerLayers,
        ELandscapeImportAlphamapType::Additive
    );
    
    Landscape->StaticLightingLOD = FMath::DivideAndRoundUp(
        FMath::CeilLogTwo((SizeX * SizeY) / (2048 * 2048) + 1),
        (uint32)2
    );
    
    ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
    if (LandscapeInfo)
    {
        LandscapeInfo->UpdateLayerInfoMap(Landscape);
    }
    
    Landscape->RegisterAllComponents();
    
    return Landscape;
}

void FMCPHorrorBridgeModule::SetupPCGAssetPlacement(ALandscape* Landscape, const FString& TerrainType)
{
    if (!Landscape) return;
    
    // Buat PCG Component
    UPCGComponent* PCGComp = NewObject<UPCGComponent>(Landscape);
    PCGComp->RegisterComponent();
    
    // Load atau buat PCG Graph
    FString PCGPath = FString::Printf(TEXT("/Game/PCG/PCG_%s_Terrain"), *TerrainType);
    UPCGGraph* PCGGraph = LoadObject<UPCGGraph>(nullptr, *PCGPath);
    
    if (!PCGGraph)
    {
        // Buat PCG Graph baru jika tidak ada
        FString PackagePath = FString::Printf(TEXT("/Game/PCG/AutoGen_%s"), *TerrainType);
        UPackage* Package = CreatePackage(*PackagePath);
        PCGGraph = NewObject<UPCGGraph>(Package, UPCGGraph::StaticClass(), 
            FName(*FString::Printf(TEXT("PCG_%s"), *TerrainType)));
        
        // Setup basic PCG graph nodes untuk horror terrain
        // Ini akan diisi dengan Surface Sampler, Normal To Density, dll
        
        FAssetRegistryModule::AssetCreated(PCGGraph);
        Package->MarkPackageDirty();
    }
    
    PCGComp->SetGraph(PCGGraph);
    PCGComp->Generate();
}

// ============================================================================
// ATMOSPHERE SETUP
// ============================================================================

void FMCPHorrorBridgeModule::SetupHorrorLighting(float DarknessLevel)
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;
    
    // Directional Light sebagai Bulan
    ADirectionalLight* MoonLight = World->SpawnActor<ADirectionalLight>();
    MoonLight->SetActorRotation(FRotator(-70, 45, 0));
    UDirectionalLightComponent* MoonComp = MoonLight->GetDirectionalLightComponent();
    MoonComp->SetIntensity(0.5f * (1.0f - DarknessLevel));
    MoonComp->SetLightColor(FLinearColor(0.7f, 0.8f, 1.0f));
    MoonComp->bAtmosphereSunLight = true;
    MoonComp->bCastVolumetricShadow = true;
    
    // Sky Light
    ASkyLight* SkyLight = World->SpawnActor<ASkyLight>();
    USkyLightComponent* SkyComp = SkyLight->GetSkyLightComponent();
    SkyComp->SetIntensity(0.3f);
    SkyComp->SetLightColor(FLinearColor(0.2f, 0.25f, 0.4f));
    
    // Mysterious Point Lights
    TArray<FVector> LightLocations = {
        FVector(1000, 2000, 300),
        FVector(-1500, 800, 450),
        FVector(500, -1200, 200)
    };
    
    for (const FVector& Loc : LightLocations)
    {
        APointLight* PointLight = World->SpawnActor<APointLight>(Loc, FRotator::ZeroRotator);
        UPointLightComponent* LightComp = PointLight->GetPointLightComponent();
        LightComp->SetIntensity(50.0f);
        LightComp->SetAttenuationRadius(800.0f);
        LightComp->SetLightColor(FLinearColor(1.0f, 0.6f, 0.3f));
        LightComp->SetCastShadows(true);
    }
}

void FMCPHorrorBridgeModule::SetupExponentialFog(float Density, const FLinearColor& Color)
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;
    
    AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>();
    UExponentialHeightFogComponent* FogComp = Fog->GetComponent();
    
    FogComp->SetFogDensity(Density);
    FogComp->SetFogInscatteringColor(Color);
    FogComp->SetFogHeightFalloff(0.02f);
    FogComp->SetFogMaxOpacity(0.9f);
    
    // Second fog layer
    FExponentialHeightFogData SecondFog;
    SecondFog.FogDensity = Density * 0.5f;
    SecondFog.FogHeightFalloff = 0.01f;
    SecondFog.FogDataHeight = -200.0f;
    FogComp->SecondFogData = SecondFog;
    
    // Volumetric fog
    FogComp->SetVolumetricFog(true);
    FogComp->SetVolumetricFogScatteringDistribution(0.8f);
    FogComp->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.95f));
    FogComp->SetVolumetricFogViewDistance(5000.0f);
}

void FMCPHorrorBridgeModule::SetupVolumetricFog()
{
    // Setup tambahan untuk volumetric fog sudah di HandleAtmosphere
}

void FMCPHorrorBridgeModule::SetupPostProcessing()
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;
    
    APostProcessVolume* PPVolume = World->SpawnActor<APostProcessVolume>();
    PPVolume->bUnbound = true;
    
    UPostProcessComponent* PPComp = PPVolume->GetPostProcessComponent();
    
    // Film Grain
    PPComp->Settings.SetFilmGrainIntensity(0.3f);
    PPComp->Settings.SetFilmGrainIntensityShadows(0.5f);
    
    // Vignette
    PPComp->Settings.SetVignetteIntensity(1.2f);
    
    // Color Grading - Cold blue
    PPComp->Settings.SetColorContrast(FVector4(1.1f, 1.05f, 1.0f, 1.0f));
    PPComp->Settings.SetColorGamma(FVector4(1.0f, 1.0f, 1.05f, 1.0f));
    PPComp->Settings.SetColorGain(FVector4(0.9f, 0.95f, 1.1f, 1.0f));
    PPComp->Settings.SetColorOffset(FVector4(0.01f, 0.02f, 0.05f, 0.0f));
    
    // Tone Mapping
    PPComp->Settings.SetToneMapperType(ETonemapperType::TM_Filmic);
    
    // Bloom
    PPComp->Settings.SetBloomIntensity(0.5f);
    PPComp->Settings.SetBloomThreshold(2.0f);
    
    // Auto Exposure
    PPComp->Settings.SetAutoExposureMethod(AEM_Manual);
    PPComp->Settings.SetAutoExposureBias(-2.0f);
    
    // Depth of Field
    PPComp->Settings.SetDepthOfFieldFocalDistance(1000.0f);
    PPComp->Settings.SetDepthOfFieldFstop(2.0f);
    
    // Chromatic Aberration
    PPComp->Settings.SetSceneFringeIntensity(0.3f);
}

void FMCPHorrorBridgeModule::SetupSkyAtmosphere()
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;
    
    ASkyAtmosphere* SkyAtmosphere = World->SpawnActor<ASkyAtmosphere>();
    USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetSkyAtmosphereComponent();
    
    // Night atmosphere
    SkyComp->TransformMode = ESkyAtmosphereTransformMode::PlanetTopAtAbsoluteWorldOrigin;
    SkyComp->BottomRadius = 6360.0f;
    SkyComp->AtmosphereHeight = 60.0f;
    SkyComp->TraceSampleCountScale = 2.0f;
}

// ============================================================================
// PLAYER MECHANICS
// ============================================================================

void FMCPHorrorBridgeModule::SetupCrouchMechanics(ACharacter* Character, float CrouchSpeed, float CrouchHeight)
{
    if (!Character) return;
    
    UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
    MoveComp->NavAgentProps.bCanCrouch = true;
    MoveComp->MaxWalkSpeedCrouched = CrouchSpeed;
    MoveComp->SetCrouchedHalfHeightHeight(CrouchHeight);
    
    // Setup input binding via Blueprint extension
    // Ini akan di-handle oleh Blueprint yang di-generate
}

void FMCPHorrorBridgeModule::SetupProneMechanics(ACharacter* Character, float ProneSpeed, float ProneHeight)
{
    if (!Character) return;
    
    // Prone mechanics require custom implementation
    // Buat Blueprint component untuk handle prone state
    
    // Implementation via Blueprint creation:
    // 1. Buat Blueprint ActorComponent
    // 2. Add boolean IsProne
    // 3. Handle capsule resize
    // 4. Animation state management
}

void FMCPHorrorBridgeModule::SetupVaultMechanics(ACharacter* Character, float MaxVaultHeight, bool bAllowMantle)
{
    if (!Character) return;
    
    // Vault mechanics menggunakan Motion Warping
    // Setup trace detection untuk vaultable objects
    
    // Implementation:
    // 1. Line trace forward untuk detect obstacle
    // 2. Check height dan depth
    // 3. Motion Warping untuk snap animation
    // 4. Root motion untuk movement
}

void FMCPHorrorBridgeModule::SetupLeanMechanics(ACharacter* Character, float LeanAngle)
{
    if (!Character) return;
    
    // Lean mechanics menggunakan camera offset
    // Q/E untuk lean left/right
    
    // Implementation via Blueprint:
    // 1. Camera component offset
    // 2. Smooth interpolation
    // 3. Collision check untuk prevent lean through walls
}

// ============================================================================
// HIDE SYSTEM
// ============================================================================

void FMCPHorrorBridgeModule::CreateHideVolume(const FString& HideType, const FVector& Location, 
    const FString& Name, bool bEnemyCanSearch)
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;
    
    // Spawn Trigger Volume
    ATriggerBox* HideVolume = World->SpawnActor<ATriggerBox>(Location, FRotator::ZeroRotator);
    HideVolume->SetActorLabel(FName(*Name));
    
    UBoxComponent* BoxComp = HideVolume->GetCollisionComponent();
    BoxComp->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
    BoxComp->SetCollisionProfileName(FName("Trigger"));
    
    // Setup hide logic via Blueprint
    // Ini akan di-generate sebagai Blueprint dengan:
    // - OnBeginOverlap: Show "Press E to Hide"
    // - OnInteract: Teleport player, disable collision, set hidden
    // - OnExit: Restore state
    // - Peek functionality
    // - Hold breath minigame
}

// ============================================================================
// JUMP SCARE
// ============================================================================

void FMCPHorrorBridgeModule::CreateJumpScareTrigger(const FString& ScareType, const FVector& Location, 
    float Radius, const TSharedPtr<FJsonObject>& Config)
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;
    
    ATriggerBox* TriggerBox = World->SpawnActor<ATriggerBox>(Location, FRotator::ZeroRotator);
    
    UBoxComponent* BoxComp = TriggerBox->GetCollisionComponent();
    BoxComp->SetBoxExtent(FVector(Radius, Radius, 200.0f));
    BoxComp->SetCollisionProfileName(FName("Trigger"));
    
    // Setup jump scare logic berdasarkan tipe
    if (ScareType == "enemy_spawn")
    {
        // Spawn enemy di belakang player
        // Play scare sound
        // Force camera look
    }
    else if (ScareType == "cinematic")
    {
        // Play Level Sequence
        // Disable player input
        // Trigger events
    }
    else if (ScareType == "audio_visual")
    {
        // Flash image
        // Play scream
        // Screen shake
    }
}

// ============================================================================
// INVENTORY
// ============================================================================

void FMCPHorrorBridgeModule::SetupInventorySystem(const FString& InventoryType, int32 SlotCount, 
    bool bAllowExamination, bool bAllowCombining)
{
    // Buat Blueprint Inventory Component
    // Setup UI widget
    // Configure grid size
    
    if (InventoryType == "resident_evil")
    {
        // Grid-based inventory
        // Items take different slot sizes
        // Rotation allowed
        // Examination mode
        // Combining system
    }
    else if (InventoryType == "amnesia")
    {
        // Physics-based
        // Drag and drop
        // Weight limit
    }
}

// ============================================================================
// ENEMY AI
// ============================================================================

void FMCPHorrorBridgeModule::CreateHorrorEnemy(const FString& EnemyType, const FVector& Location,
    const FString& Name, float SightRange, float HearingRange, float FOVAngle)
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;
    
    // Spawn enemy character
    // Setup AI Controller
    // Configure Behavior Tree
    // Setup Perception
    
    // Enemy types:
    // - stalker: Follow dari kejauhan
    // - chaser: Direct chase
    // - investigator: Search patterns
    // - ambusher: Wait in shadows
}

// ============================================================================
// AUDIO
// ============================================================================

void FMCPHorrorBridgeModule::SetupDynamicMusicSystem()
{
    // Setup MetaSounds untuk dynamic music
    // Parameters: intensity, tension, chase state
    // Layers: ambient, tension, chase, combat
}

void FMCPHorrorBridgeModule::SetupAmbientSounds(const FString& AmbientType)
{
    // Setup ambient sounds berdasarkan tipe
    // - wind: Angin howling
    // - forest: Suara hutan
    // - industrial: Suara mesin
    // - cave: Echo, water drops
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMCPHorrorBridgeModule, MCPHorrorBridge)
