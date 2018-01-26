#pragma once
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/Audio.hpp"

class App;

extern AudioSystem* g_theAudio;
extern InputSystem* g_theInput;
extern Renderer* g_theRenderer;
extern App* g_theApp;

extern bool g_drawDebug;

extern float g_GAME_WIDTH;
extern float g_GAME_HEIGHT;

extern int g_NUM_SELECTION_BLOCKS;
extern float g_HUD_BLOCK_WIDTH;
extern float g_HUD_BLOCK_HEIGHT;

extern int g_GAME_VERSION;

extern std::string g_PLAYER_SAVE_FILE_PATH;

constexpr float PLAYER_HEIGHT = 1.86f;
constexpr float PLAYER_EYE_HEIGHT = 1.62f;
constexpr float PLAYER_RADIUS = 0.3f;

constexpr float PLAYER_SPEED = 4.2f;
constexpr float PLAYER_JUMP_SPEED = 5.f;

constexpr float PLAYER_FOOTSTEP_SOUND_THRESHOLD = 0.5f;
constexpr float PLAYER_FOOTSTEP_SOUND_THRESHOLD_VARIANCE = 0.1f;

constexpr float SURFACE_FRICTION = 5.f;
constexpr float FLYING_AIR_RESISTANCE = 3.f;
constexpr float GRAVITY = 9.8f;

constexpr float CAMERA_THIRD_PERSON_DISTANCE = 4.f;
constexpr float CAMERA_FIXED_ANGLE_YAW = 45.f;
constexpr float CAMERA_FIXED_ANGLE_PITCH = 20.f;

constexpr float HOOK_MAX_DISTANCE = 25.f;
constexpr float HOOK_ATTACH_DISTANCE = 20.f;

extern int g_NUM_START_CHUNKS_X;
extern int g_NUM_START_CHUNKS_Y;

extern SpriteSheet* g_blockSprites;
extern BitmapFont* g_squirrelFont;

constexpr int LIGHT_MASK = 0b00001111;
constexpr int LIGHT_DIRTY_MASK = 0b10000000;
constexpr int IS_SKY_MASK = 0b01000000;
constexpr int IS_OPAQUE_MASK = 0b00100000;
constexpr int IS_SOLID_MASK = 0b00010000;
constexpr int IS_OPAQUE_AND_SOLID_MASK = IS_OPAQUE_MASK | IS_SOLID_MASK;

extern const std::string BLOCK_SELECTION_SOUND_PATH;

extern const int AIR_SPRITE_INDEX;

extern const int DIRT_SPRITE_INDEX;
extern const std::string DIRT_BREAK_1_SOUND_PATH;
extern const std::string DIRT_BREAK_2_SOUND_PATH;
extern const std::string DIRT_PLACE_1_SOUND_PATH;
extern const std::string DIRT_PLACE_2_SOUND_PATH;
extern const std::string DIRT_FOOTSTEP_1_SOUND_PATH;
extern const std::string DIRT_FOOTSTEP_2_SOUND_PATH;

extern const int GRASS_TOP_SPRITE_INDEX;
extern const int GRASS_SIDE_SPRITE_INDEX;
extern const int GRASS_BOTTOM_SPRITE_INDEX;
extern const std::string GRASS_BREAK_1_SOUND_PATH;
extern const std::string GRASS_BREAK_2_SOUND_PATH;
extern const std::string GRASS_PLACE_1_SOUND_PATH;
extern const std::string GRASS_PLACE_2_SOUND_PATH;
extern const std::string GRASS_FOOTSTEP_1_SOUND_PATH;
extern const std::string GRASS_FOOTSTEP_2_SOUND_PATH;


extern const int SAND_SPRITE_INDEX;
extern const std::string SAND_BREAK_1_SOUND_PATH;
extern const std::string SAND_BREAK_2_SOUND_PATH;
extern const std::string SAND_PLACE_1_SOUND_PATH;
extern const std::string SAND_PLACE_2_SOUND_PATH;
extern const std::string SAND_FOOTSTEP_1_SOUND_PATH;
extern const std::string SAND_FOOTSTEP_2_SOUND_PATH;


extern const int STONE_SPRITE_INDEX;
extern const std::string STONE_BREAK_1_SOUND_PATH;
extern const std::string STONE_BREAK_2_SOUND_PATH;
extern const std::string STONE_PLACE_1_SOUND_PATH;
extern const std::string STONE_PLACE_2_SOUND_PATH;
extern const std::string STONE_FOOTSTEP_1_SOUND_PATH;
extern const std::string STONE_FOOTSTEP_2_SOUND_PATH;

extern const int GRAVEL_SPRITE_INDEX;
extern const std::string GRAVEL_BREAK_1_SOUND_PATH;
extern const std::string GRAVEL_BREAK_2_SOUND_PATH;
extern const std::string GRAVEL_PLACE_1_SOUND_PATH;
extern const std::string GRAVEL_PLACE_2_SOUND_PATH;
extern const std::string GRAVEL_FOOTSTEP_1_SOUND_PATH;
extern const std::string GRAVEL_FOOTSTEP_2_SOUND_PATH;

extern const int COBBLESTONE_SPRITE_INDEX;
extern const std::string COBBLESTONE_BREAK_1_SOUND_PATH;
extern const std::string COBBLESTONE_BREAK_2_SOUND_PATH;
extern const std::string COBBLESTONE_PLACE_1_SOUND_PATH;
extern const std::string COBBLESTONE_PLACE_2_SOUND_PATH;
extern const std::string COBBLESTONE_FOOTSTEP_1_SOUND_PATH;
extern const std::string COBBLESTONE_FOOTSTEP_2_SOUND_PATH;

extern const int GLOWSTONE_SPRITE_INDEX;
extern const std::string GLOWSTONE_BREAK_1_SOUND_PATH;
extern const std::string GLOWSTONE_BREAK_2_SOUND_PATH;
extern const std::string GLOWSTONE_PLACE_1_SOUND_PATH;
extern const std::string GLOWSTONE_PLACE_2_SOUND_PATH;
extern const std::string GLOWSTONE_FOOTSTEP_1_SOUND_PATH;
extern const std::string GLOWSTONE_FOOTSTEP_2_SOUND_PATH;

extern const int WOOD_PLANKS_SPRITE_INDEX;
extern const std::string WOOD_PLANKS_BREAK_1_SOUND_PATH;
extern const std::string WOOD_PLANKS_BREAK_2_SOUND_PATH;
extern const std::string WOOD_PLANKS_PLACE_1_SOUND_PATH;
extern const std::string WOOD_PLANKS_PLACE_2_SOUND_PATH;
extern const std::string WOOD_PLANKS_FOOTSTEP_1_SOUND_PATH;
extern const std::string WOOD_PLANKS_FOOTSTEP_2_SOUND_PATH;

extern const int WOOD_LOG_SIDE_SPRITE_INDEX;
extern const int WOOD_LOG_TOP_BOTTOM_SPRITE_INDEX;
extern const std::string WOOD_LOG_BREAK_1_SOUND_PATH;
extern const std::string WOOD_LOG_BREAK_2_SOUND_PATH;
extern const std::string WOOD_LOG_PLACE_1_SOUND_PATH;
extern const std::string WOOD_LOG_PLACE_2_SOUND_PATH;
extern const std::string WOOD_LOG_FOOTSTEP_1_SOUND_PATH;
extern const std::string WOOD_LOG_FOOTSTEP_2_SOUND_PATH;

extern const int LEAVES_SPRITE_INDEX;
extern const std::string LEAVES_BREAK_1_SOUND_PATH;
extern const std::string LEAVES_BREAK_2_SOUND_PATH;
extern const std::string LEAVES_PLACE_1_SOUND_PATH;
extern const std::string LEAVES_PLACE_2_SOUND_PATH;
extern const std::string LEAVES_FOOTSTEP_1_SOUND_PATH;
extern const std::string LEAVES_FOOTSTEP_2_SOUND_PATH;

extern const int WATER_SPRITE_INDEX;
extern const std::string WATER_BREAK_1_SOUND_PATH;
extern const std::string WATER_BREAK_2_SOUND_PATH;
extern const std::string WATER_PLACE_1_SOUND_PATH;
extern const std::string WATER_PLACE_2_SOUND_PATH;
extern const std::string WATER_FOOTSTEP_1_SOUND_PATH;
extern const std::string WATER_FOOTSTEP_2_SOUND_PATH;

extern const int SNOW_SPRITE_INDEX;
extern const std::string SNOW_BREAK_1_SOUND_PATH;
extern const std::string SNOW_BREAK_2_SOUND_PATH;
extern const std::string SNOW_PLACE_1_SOUND_PATH;
extern const std::string SNOW_PLACE_2_SOUND_PATH;
extern const std::string SNOW_FOOTSTEP_1_SOUND_PATH;
extern const std::string SNOW_FOOTSTEP_2_SOUND_PATH;


extern const Vertex3D g_bottomFaceVertexes[];
extern const Vertex3D g_topFaceVertexes[];
extern const Vertex3D g_northFaceVertexes[];
extern const Vertex3D g_southFaceVertexes[];
extern const Vertex3D g_eastFaceVertexes[];
extern const Vertex3D g_westFaceVertexes[];

constexpr int MAXIMUM_CHUNKS = 500;
constexpr int IDEAL_CHUNKS = 490;
constexpr float VISIBILITY_RANGE = 175.f;

constexpr unsigned int SKY_LIGHT_VALUE = 15;

constexpr unsigned char LIGHT_RGBA_VALUES[16] = 
{
	20, 35, 50, 65, 80, 95, 110, 125, 140, 155, 175, 195, 210, 225, 240, 255
};

constexpr int CHUNK_X_BITS = 4;
constexpr int CHUNK_Y_BITS = 4;
constexpr int CHUNK_Z_BITS = 7;
constexpr int CHUNK_XY_BITS = CHUNK_X_BITS + CHUNK_Y_BITS;

constexpr int CHUNK_X = BIT(CHUNK_X_BITS);
constexpr int CHUNK_Y = BIT(CHUNK_Y_BITS);
constexpr int CHUNK_Z = BIT(CHUNK_Z_BITS);
constexpr int X_MASK_BITS = CHUNK_X - 1;
constexpr int Y_MASK_BITS = (CHUNK_Y - 1) << CHUNK_X_BITS;
constexpr int Z_MASK_BITS = (CHUNK_Z - 1) << CHUNK_XY_BITS;

constexpr int BLOCKS_PER_LAYER = CHUNK_X * CHUNK_Y;
constexpr int BLOCKS_PER_CHUNK = BLOCKS_PER_LAYER * CHUNK_Z;

constexpr int SEA_LEVEL = CHUNK_Z / 2;
constexpr int STONE_OFFSET = 0;
constexpr int DIRT_OFFSET = 4;
constexpr int GRASS_OFFSET = DIRT_OFFSET + 1;

constexpr float DESERT_TEMPERATURE_MINIMUM = 80.f;
constexpr float DESERT_WETNESS_MAXIMUM = 0.5f;
constexpr float SNOW_TEMPERATURE_MAXIMUM = 40.f;

bool operator <(IntVector2 lhs, IntVector2 rhs);


extern std::vector<Vertex3D> g_debugPoints;

void AddDebugPoint(const Vector3& position, const Rgba& color);
void DrawDebugPoints(float pointSize, bool enableDepthTesting);
void ClearDebugPoints();