#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "HttpServerResponse.h"

class FHttpServerResponse;
class FHttpResultCallback;

class FMCPHorrorBridgeModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
private:
    // HTTP Server
    TSharedPtr<IHttpRouter> HttpRouter;
    uint16 ServerPort = 8091;
    bool bIsServerRunning = false;
    
    // Route Handlers
    bool HandleStatusRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
    bool HandleTerrainRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
    bool HandleAtmosphereRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
    bool HandlePlayerMechanicsRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
    bool HandleHideMechanicsRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
    bool HandleJumpScareRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
    bool HandleInventoryRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
    bool HandleEnemyAIRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
    bool HandleAudioRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
    
    // Helper Functions
    bool SendSuccessResponse(const FHttpResultCallback& OnComplete, const FString& Message, TSharedPtr<FJsonObject> Data = nullptr);
    bool SendErrorResponse(const FHttpResultCallback& OnComplete, const FString& ErrorMessage);
    TSharedPtr<FJsonObject> ParseJsonBody(const FString& Body);
    
    // Terrain Generation
    void GenerateHorrorTerrain(const FString& TerrainType, int32 SizeX, int32 SizeY, float DarknessLevel);
    TArray<uint16> GenerateProceduralHeightmap(const FString& TerrainType, int32 Width, int32 Height);
    ALandscape* CreateLandscapeActor(const FString& Name, const TArray<uint16>& HeightData, int32 SizeX, int32 SizeY);
    void SetupPCGAssetPlacement(ALandscape* Landscape, const FString& TerrainType);
    
    // Atmosphere
    void SetupHorrorLighting(float DarknessLevel);
    void SetupExponentialFog(float Density, const FLinearColor& Color);
    void SetupVolumetricFog();
    void SetupPostProcessing();
    void SetupSkyAtmosphere();
    
    // Player Mechanics
    void SetupCrouchMechanics(ACharacter* Character, float CrouchSpeed, float CrouchHeight);
    void SetupProneMechanics(ACharacter* Character, float ProneSpeed, float ProneHeight);
    void SetupVaultMechanics(ACharacter* Character, float MaxVaultHeight, bool bAllowMantle);
    void SetupLeanMechanics(ACharacter* Character, float LeanAngle);
    
    // Hide System
    void CreateHideVolume(const FString& HideType, const FVector& Location, const FString& Name, bool bEnemyCanSearch);
    
    // Jump Scare
    void CreateJumpScareTrigger(const FString& ScareType, const FVector& Location, float Radius, const TSharedPtr<FJsonObject>& Config);
    
    // Inventory
    void SetupInventorySystem(const FString& InventoryType, int32 SlotCount, bool bAllowExamination, bool bAllowCombining);
    
    // Enemy AI
    void CreateHorrorEnemy(const FString& EnemyType, const FVector& Location, const FString& Name, 
                          float SightRange, float HearingRange, float FOVAngle);
    
    // Audio
    void SetupDynamicMusicSystem();
    void SetupAmbientSounds(const FString& AmbientType);
};
