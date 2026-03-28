Step 1: Setup Unreal Engine Plugin
# 1. Buat folder plugins di project UE5 Anda
mkdir -p C:\UnrealProjects\MyHorrorGame\Plugins\MCPHorrorBridge

# 2. Copy semua file C++ plugin ke folder tersebut
# Struktur:
# MyHorrorGame/
# └── Plugins/
#     └── MCPHorrorBridge/
#         ├── MCPHorrorBridge.uplugin
#         └── Source/
#             └── MCPHorrorBridge/
#                 ├── MCPHorrorBridge.Build.cs
#                 ├── Public/
#                 │   └── (header files)
#                 └── Private/
#                     └── (cpp files)

# 3. Generate Visual Studio project files
# Right-click .uproject file -> "Generate Visual Studio project files"

# 4. Build plugin
# Buka .sln file, set configuration ke "Development Editor", build
======================================

Step 2: Setup Python Environment
# Buat virtual environment
python -m venv unreal_mcp_env

# Activate
# Windows:
unreal_mcp_env\Scripts\activate
# macOS/Linux:
source unreal_mcp_env/bin/activate

# Install dependencies
pip install mcp httpx

# Save requirements
pip freeze > requirements.txt
===================================

Step 3: Konfigurasi MCP Client
# Edit config file sesuai dengan MCP client yang digunakan

# Untuk Claude Desktop:
# Edit: %APPDATA%\Claude\claude_desktop_config.json (Windows)
# Edit: ~/Library/Application Support/Claude/claude_desktop_config.json (macOS)
# Edit: ~/.config/Claude/claude_desktop_config.json (Linux)

# Untuk Cursor:
# Buat file: .cursor/mcp.json di project folder

# Untuk Cline (VS Code):
# Edit: cline_mcp_settings.json
==================================================

4.2 Workflow Penggunaan
Workflow 1: Quick Start - Game Lengkap dengan Satu Command
User ke AI: "Buatkan game horror lengkap di pegunungan berkabut 3x3 km dengan 5 enemy, 
            8 tempat bersembunyi, dan semua mekanik movement"

AI akan menjalankan:
create_complete_horror_game(
    game_name="MountainHorror",
    terrain_type="mountain",
    size_km=3.0,
    horror_intensity="extreme",
    include_mechanics=["crouch", "prone", "vault", "lean", "hide", "inventory"],
    enemy_count=5,
    jump_scare_count=8,
    hide_spot_count=8
)

Output yang dihasilkan:
🎮 CREATING COMPLETE HORROR GAME: MountainHorror
============================================================
🏔️ STEP 1: GENERATING TERRAIN...
✅ Terrain created: 3x3 km mountain
🌙 STEP 2: SETTING UP ATMOSPHERE...
✅ Atmosphere applied: Foggy night with volumetric lighting
🚶 STEP 3: CONFIGURING PLAYER MECHANICS...
✅ Crouch/Prone configured
✅ Vault/Mantle configured
✅ Lean/Peek configured
🛌 STEP 4: CREATING 8 HIDE SPOTS...
✅ Hide spot 1: bed at (234, -567, 120)
✅ Hide spot 2: locker at (-890, 123, 200)
...
👻 STEP 5: SETTING UP 8 JUMP SCARES...
✅ Jump scare 1: enemy_spawn
✅ Jump scare 2: cinematic
...
🎒 STEP 6: SETTING UP INVENTORY...
✅ RE-style inventory system configured
👹 STEP 7: SPAWNING 5 ENEMIES...
✅ Enemy 1: stalker at (456, -789, 150)
✅ Enemy 2: chaser at (-123, 456, 300)
...
🔊 STEP 8: CONFIGURING AUDIO...
✅ Dynamic music system active
============================================================
🎮 GAME CREATION COMPLETE!
Project: MountainHorror
Terrain: 3x3 km mountain
Intensity: EXTREME
Enemies: 5 | Jump Scares: 8 | Hide Spots: 8
============================================================

================================================
Workflow 2: Development Bertahap (Granular Control)
Step 1: Buat Terrain Dasar
User: "Buatkan terrain pegunungan 4x4 km"

AI: create_terrain(
    terrain_type="mountain",
    name="CursedPeaks",
    size_km=4.0,
    darkness_level=0.9
)
Step 2: Setup Atmosphere
User: "Aktifkan atmosphere malam berkabut dengan fog tebal"

AI: setup_atmosphere(
    atmosphere_type="foggy_night",
    darkness_level=0.9,
    fog_density=0.08
)
Step 3: Tambahkan Player Mechanics
User: "Enable crouch, prone, dan vault untuk player"

AI: setup_player_movement(
    enable_crouch=True,
    enable_prone=True,
    enable_vault=True,
    enable_lean=True
)
Step 4: Buat Hide Spots Strategis
User: "Buat hide spot under bed di koordinat 1000, 500, 100"

AI: create_hide_spot(
    hide_type="bed",
    x=1000.0,
    y=500.0,
    z=100.0,
    name="BedroomHide",
    enemy_can_search=True
)
Step 5: Setup Jump Scares
User: "Buat jump scare enemy spawn di tengah map"

AI: create_jump_scare(
    scare_type="enemy_spawn",
    x=0.0,
    y=0.0,
    z=200.0,
    name="CentralAmbush"
)
Step 6: Spawn Enemies
User: "Spawn 3 stalker enemy di perimeter"

AI: (loop 3x)
create_enemy(
    enemy_type="stalker",
    x=random(-2000, 2000),
    y=random(-2000, 2000),
    z=150.0,
    name="Stalker_01"
)
Step 7: Finalisasi dengan Inventory
User: "Setup inventory RE style"

AI: setup_inventory(
    slot_count=8,
    allow_examination=True,
    allow_combining=True
)
===============================================

Workflow 3: Custom Complex Game
User: "Saya ingin membuat game horror psychological thriller dengan setting 
      asylum terbengkalai di pegunungan bersalju. Butuh:
      - Terrain berliku dengan visibility sangat rendah
      - Enemy yang stalk dari kejauhan (tidak langsung chase)
      - Banyak hide spots di locker dan closet
      - Jump scare yang lebih psychological (fake-outs, sounds) 
      - Sanity system yang turun saat melihat enemy atau di tempat gelap"

AI akan menjalankan:

1. create_terrain(
    terrain_type="mountain",
    name="AsylumPeaks",
    size_km=2.5,
    darkness_level=0.95  # Sangat gelap
)

2. setup_atmosphere(
    atmosphere_type="snowy_night",  # Custom atmosphere
    darkness_level=0.95,
    fog_density=0.1  # Sangat berkabut
)

3. setup_player_movement(
    enable_crouch=True,
    enable_prone=True,  # Untuk crawl through vents
    enable_vault=False,  # Tidak perlu vault di asylum
    enable_lean=True
)

4. (Loop 12x untuk hide spots)
create_hide_spot(
    hide_type=random(["locker", "closet"]),  # Asylum appropriate
    x=..., y=..., z=...,
    enemy_can_search=True,
    hold_breath_minigame=True
)

5. (Loop 6x untuk jump scares - mostly psychological)
create_jump_scare(
    scare_type=random(["fake_out", "audio_visual"]),  # Psychological horror
    ...
)

6. (Loop 4x untuk enemies - all stalkers)
create_enemy(
    enemy_type="stalker",  # Psychological - tidak langsung attack
    sight_range=2000.0,  # Dapat lihat dari jauh
    hearing_range=1000.0,
    fov=120.0  # Wide FOV untuk peripheral detection
)

7. setup_sanity_system(
    drain_rate=5,  # Per detik di tempat gelap
    enemy_sight_drain=20,  # Saat lihat enemy
    recovery_rate=2,  # Di tempat terang
    hallucination_threshold=30  # Start hallucinating below 30 sanity
)

8. setup_dynamic_music(
    layers=["ambient_asylum", "tension_stalker", "chase_rare", "psychotic_break"]
)
===============================================================================

4.3 Command Reference Table
| Command                       | Fungsi                            | Contoh Penggunaan                    |
| ----------------------------- | --------------------------------- | ------------------------------------ |
| `check_status`                | Cek koneksi ke UE5                | "Apakah Unreal sudah terhubung?"     |
| `create_complete_horror_game` | Buat game lengkap satu kali       | "Buat game horror lengkap"           |
| `create_terrain`              | Generate terrain procedural       | "Buat pegunungan 3x3 km"             |
| `setup_atmosphere`            | Setup lighting, fog, post-process | "Aktifkan malam berkabut"            |
| `setup_player_movement`       | Konfigurasi movement mechanics    | "Enable crouch dan vault"            |
| `create_hide_spot`            | Buat tempat bersembunyi           | "Buat hide spot locker di x100 y200" |
| `create_jump_scare`           | Setup jump scare trigger          | "Buat jump scare di koridor"         |
| `create_enemy`                | Spawn enemy AI                    | "Spawn stalker di belakang gedung"   |
| `setup_inventory`             | Konfigurasi inventory system      | "Setup inventory 8 slot"             |

4.4 Troubleshooting
Problem: "Cannot connect to Unreal Engine"
Solusi:
# 1. Pastikan UE5 Editor sedang running
# 2. Pastikan plugin sudah di-build dan di-enable
# 3. Cek port 8091 tidak digunakan aplikasi lain
# 4. Restart UE5 Editor dan coba lagi

# Test koneksi manual:
curl http://localhost:8091/api/status

Problem: "Plugin not found in UE5"
Solusi:
# 1. Enable plugin di .uproject file atau UE5 Editor
# Edit -> Plugins -> Project -> MCP Horror Bridge -> Enable
# 2. Restart UE5 Editor
# 3. Rebuild project jika perlu

Problem: Python module not found
Solusi:
# Install dependencies
pip install mcp httpx

# Atau jika menggunakan virtual environment:
python -m venv venv
source venv/bin/activate  # atau venv\Scripts\activate di Windows
pip install -r requirements.txt

BAGIAN 6: CHECKLIST IMPLEMENTASI
Pre-Implementation
[ ] UE5.3+ terinstall
[ ] Visual Studio 2022 dengan C++ workload
[ ] Python 3.10+ terinstall
[ ] MCP client (Claude/Cursor/Cline) terinstall
Plugin Setup
[ ] Copy plugin files ke folder Plugins
[ ] Generate VS project files
[ ] Build plugin dalam Development Editor
[ ] Enable plugin di UE5 Editor
[ ] Restart UE5 Editor
[ ] Verify HTTP server running di port 8091
Python Server Setup
[ ] Install Python dependencies
[ ] Test run server: python unreal_horror_mcp_server.py
[ ] Configure MCP client config
[ ] Test connection: check_status
First Game Creation
[ ] Run create_complete_horror_game dengan parameter basic
[ ] Verify terrain muncul di viewport
[ ] Check lighting dan atmosphere
[ ] Test player movement di PIE
[ ] Verify enemies spawn dengan AI behavior
[ ] Test hide spots dan jump scares

BAGIAN 7: ADVANCED FEATURES (Future Roadmap)
7.1 Planned Features
| Feature            | Status  | Deskripsi                                         |
| ------------------ | ------- | ------------------------------------------------- |
| Multiplayer Horror | Planned | Co-op survival horror (4 players)                 |
| VR Support         | Planned | Full VR immersion dengan hand tracking            |
| Procedural Story   | Planned | AI-generated narrative berdasarkan player actions |
| Dynamic Difficulty | Planned | AI Director yang menyesuaikan horror intensity    |
| Mod Support        | Planned | Steam Workshop integration untuk custom maps      |
| Raytracing         | Planned | Full RT untuk realistic shadows dan reflections   |


7.2 Integration dengan Tools Lain
# Houdini Engine untuk procedural content
@mcp.tool()
async def setup_houdini_integration():
    """Integrate Houdini untuk advanced procedural generation."""

# MetaHuman untuk realistic characters  
@mcp.tool()
async def create_metahuman_enemy():
    """Spawn enemy dengan MetaHuman rig."""

# RealityCapture untuk photogrammetry
@mcp.tool()
async def import_photogrammetry_scan():
    """Import 3D scan untuk realistic environments."""


 STRUKTUR PROJECT
 UnrealHorrorMCP/
├── MCPBridgePlugin/                          # C++ UE5 Plugin
│   ├── Source/
│   │   ├── MCPHorrorBridge/
│   │   │   ├── MCPHorrorBridge.Build.cs
│   │   │   ├── Public/
│   │   │   │   ├── MCPHorrorBridge.h
│   │   │   │   ├── MCPHorrorBridgeBPLibrary.h
│   │   │   │   ├── MCPHorrorGameMode.h
│   │   │   │   ├── MCPHorrorCharacter.h
│   │   │   │   ├── MCPHorrorEnemyAI.h
│   │   │   │   ├── MCPHorrorInventoryComponent.h
│   │   │   │   ├── MCPHorrorHideVolume.h
│   │   │   │   ├── MCPHorrorJumpScareTrigger.h
│   │   │   │   ├── MCPHorrorTerrainGenerator.h
│   │   │   │   └── MCPHorrorAtmosphereManager.h
│   │   │   └── Private/
│   │   │       ├── MCPHorrorBridge.cpp
│   │   │       ├── MCPHorrorBridgeBPLibrary.cpp
│   │   │       ├── MCPHorrorGameMode.cpp
│   │   │       ├── MCPHorrorCharacter.cpp
│   │   │       ├── MCPHorrorEnemyAI.cpp
│   │   │       ├── MCPHorrorInventoryComponent.cpp
│   │   │       ├── MCPHorrorHideVolume.cpp
│   │   │       ├── MCPHorrorJumpScareTrigger.cpp
│   │   │       ├── MCPHorrorTerrainGenerator.cpp
│   │   │       └── MCPHorrorAtmosphereManager.cpp
│   │   └── MCPHorrorBridgeServer.cpp          # HTTP Server
│   └── MCPHorrorBridge.uplugin
├── PythonServer/                             # Python MCP Server
│   ├── unreal_horror_mcp_server.py           # Main server
│   ├── requirements.txt
│   └── config.json
├── Blueprints/                               # Blueprint templates
│   ├── BP_HorrorPlayer.uasset
│   ├── BP_HorrorEnemy.uasset
│   ├── BP_HideVolume.uasset
│   └── BP_JumpScareTrigger.uasset
└── README.md                                 # Dokumentasi lengkap
