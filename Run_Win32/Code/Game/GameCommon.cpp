#include "Game/GameCommon.hpp"

InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Renderer* g_theRenderer = nullptr;
App* g_theApp = nullptr;

extern bool g_drawDebug = false;

float g_GAME_WIDTH = 1600.f;
float g_GAME_HEIGHT = 900.f;

int g_NUM_SELECTION_BLOCKS = 10;
float g_HUD_BLOCK_WIDTH = g_GAME_WIDTH / (2.f * g_NUM_SELECTION_BLOCKS);
float g_HUD_BLOCK_HEIGHT = g_HUD_BLOCK_WIDTH;

int g_GAME_VERSION = 1;

std::string g_PLAYER_SAVE_FILE_PATH = "Data/Save/Player.sav";

int g_NUM_START_CHUNKS_X = 7;
int g_NUM_START_CHUNKS_Y = 7;

SpriteSheet* g_blockSprites = nullptr;
BitmapFont* g_squirrelFont = nullptr;

const std::string BLOCK_SELECTION_SOUND_PATH = "Data/Audio/BlockSelection.wav";

const int AIR_SPRITE_INDEX = 0;

const int DIRT_SPRITE_INDEX = (8 * 16) + 7;
const std::string DIRT_BREAK_1_SOUND_PATH = "Data/Audio/DirtFootstep1.wav";
const std::string DIRT_BREAK_2_SOUND_PATH = "Data/Audio/DirtFootstep2.wav";
const std::string DIRT_PLACE_1_SOUND_PATH = "Data/Audio/DirtFootstep1.wav";
const std::string DIRT_PLACE_2_SOUND_PATH = "Data/Audio/DirtFootstep2.wav";
const std::string DIRT_FOOTSTEP_1_SOUND_PATH = "Data/Audio/DirtFootstep1.wav";
const std::string DIRT_FOOTSTEP_2_SOUND_PATH = "Data/Audio/DirtFootstep2.wav";

const int GRASS_TOP_SPRITE_INDEX = (8 * 16) + 9;
const int GRASS_SIDE_SPRITE_INDEX = (8 * 16) + 8;
const int GRASS_BOTTOM_SPRITE_INDEX = (8 * 16) + 7;
const std::string GRASS_BREAK_1_SOUND_PATH = "Data/Audio/DirtFootstep1.wav";
const std::string GRASS_BREAK_2_SOUND_PATH = "Data/Audio/DirtFootstep2.wav";
const std::string GRASS_PLACE_1_SOUND_PATH = "Data/Audio/DirtFootstep1.wav";
const std::string GRASS_PLACE_2_SOUND_PATH = "Data/Audio/DirtFootstep2.wav";
const std::string GRASS_FOOTSTEP_1_SOUND_PATH = "Data/Audio/DirtFootstep1.wav";
const std::string GRASS_FOOTSTEP_2_SOUND_PATH = "Data/Audio/DirtFootstep2.wav";

const int STONE_SPRITE_INDEX = (10 * 16) + 2;
const std::string STONE_BREAK_1_SOUND_PATH = "Data/Audio/Wood1.wav";
const std::string STONE_BREAK_2_SOUND_PATH = "Data/Audio/Wood2.wav";
const std::string STONE_PLACE_1_SOUND_PATH = "Data/Audio/Wood1.wav";
const std::string STONE_PLACE_2_SOUND_PATH = "Data/Audio/Wood2.wav";
const std::string STONE_FOOTSTEP_1_SOUND_PATH = "Data/Audio/Wood1.wav";
const std::string STONE_FOOTSTEP_2_SOUND_PATH = "Data/Audio/Wood2.wav";

const int SAND_SPRITE_INDEX = (8 * 16) + 1;
const std::string SAND_BREAK_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string SAND_BREAK_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string SAND_PLACE_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string SAND_PLACE_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string SAND_FOOTSTEP_1_SOUND_PATH = "Data/Audio/GravelWalk1.wav";
const std::string SAND_FOOTSTEP_2_SOUND_PATH = "Data/Audio/GravelWalk2.wav";

const int GRAVEL_SPRITE_INDEX = (10 * 16) + 0;
const std::string GRAVEL_BREAK_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string GRAVEL_BREAK_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string GRAVEL_PLACE_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string GRAVEL_PLACE_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string GRAVEL_FOOTSTEP_1_SOUND_PATH = "Data/Audio/GravelWalk1.wav";
const std::string GRAVEL_FOOTSTEP_2_SOUND_PATH = "Data/Audio/GravelWalk2.wav";

const int COBBLESTONE_SPRITE_INDEX = (10 * 16) + 3;
const std::string COBBLESTONE_BREAK_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string COBBLESTONE_BREAK_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string COBBLESTONE_PLACE_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string COBBLESTONE_PLACE_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string COBBLESTONE_FOOTSTEP_1_SOUND_PATH = "Data/Audio/GravelWalk1.wav";
const std::string COBBLESTONE_FOOTSTEP_2_SOUND_PATH = "Data/Audio/GravelWalk2.wav";

const int GLOWSTONE_SPRITE_INDEX = (11 * 16) + 4;
const std::string GLOWSTONE_BREAK_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string GLOWSTONE_BREAK_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string GLOWSTONE_PLACE_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string GLOWSTONE_PLACE_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string GLOWSTONE_FOOTSTEP_1_SOUND_PATH = "Data/Audio/GravelWalk1.wav";
const std::string GLOWSTONE_FOOTSTEP_2_SOUND_PATH = "Data/Audio/GravelWalk2.wav";

const int WOOD_PLANKS_SPRITE_INDEX = (8 * 16) + 13;
const std::string WOOD_PLANKS_BREAK_1_SOUND_PATH = "Data/Audio/Wood1.wav";
const std::string WOOD_PLANKS_BREAK_2_SOUND_PATH = "Data/Audio/Wood2.wav";
const std::string WOOD_PLANKS_PLACE_1_SOUND_PATH = "Data/Audio/Wood1.wav";
const std::string WOOD_PLANKS_PLACE_2_SOUND_PATH = "Data/Audio/Wood2.wav";
const std::string WOOD_PLANKS_FOOTSTEP_1_SOUND_PATH = "Data/Audio/Wood1.wav";
const std::string WOOD_PLANKS_FOOTSTEP_2_SOUND_PATH = "Data/Audio/Wood2.wav";

const int WOOD_LOG_SIDE_SPRITE_INDEX = (8 * 16) + 14;
const int WOOD_LOG_TOP_BOTTOM_SPRITE_INDEX = (8 * 16) + 15;
const std::string WOOD_LOG_BREAK_1_SOUND_PATH = "Data/Audio/Wood1.wav";
const std::string WOOD_LOG_BREAK_2_SOUND_PATH = "Data/Audio/Wood2.wav";
const std::string WOOD_LOG_PLACE_1_SOUND_PATH = "Data/Audio/Wood1.wav";
const std::string WOOD_LOG_PLACE_2_SOUND_PATH = "Data/Audio/Wood2.wav";
const std::string WOOD_LOG_FOOTSTEP_1_SOUND_PATH = "Data/Audio/Wood1.wav";
const std::string WOOD_LOG_FOOTSTEP_2_SOUND_PATH = "Data/Audio/Wood2.wav";

const int LEAVES_SPRITE_INDEX = (8 * 16) + 12;
const std::string LEAVES_BREAK_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string LEAVES_BREAK_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string LEAVES_PLACE_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string LEAVES_PLACE_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string LEAVES_FOOTSTEP_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string LEAVES_FOOTSTEP_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";

const int WATER_SPRITE_INDEX = (11 * 16) + 8;
const std::string WATER_BREAK_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string WATER_BREAK_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string WATER_PLACE_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string WATER_PLACE_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string WATER_FOOTSTEP_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string WATER_FOOTSTEP_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";

const int SNOW_SPRITE_INDEX = (8 * 16) + 0;
const std::string SNOW_BREAK_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string SNOW_BREAK_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string SNOW_PLACE_1_SOUND_PATH = "Data/Audio/GravelPlace1.wav";
const std::string SNOW_PLACE_2_SOUND_PATH = "Data/Audio/GravelPlace2.wav";
const std::string SNOW_FOOTSTEP_1_SOUND_PATH = "Data/Audio/GravelWalk1.wav";
const std::string SNOW_FOOTSTEP_2_SOUND_PATH = "Data/Audio/GravelWalk2.wav";

const Vertex3D g_bottomFaceVertexes[] =
{
	Vertex3D(Vector3(1.f, 0.f, 0.f), Vector2(1, 0)),
	Vertex3D(Vector3(0.f, 0.f, 0.f), Vector2(0, 0)),
	Vertex3D(Vector3(0.f, 1.f, 0.f), Vector2(0, 1)),
	Vertex3D(Vector3(1.f, 1.f, 0.f), Vector2(1, 1))
};

const Vertex3D g_topFaceVertexes[] = 
{
	Vertex3D(Vector3(0.f, 0.f, 1.f), Vector2(0, 1)),
	Vertex3D(Vector3(1.f, 0.f, 1.f), Vector2(1, 1)),
	Vertex3D(Vector3(1.f, 1.f, 1.f), Vector2(1, 0)),
	Vertex3D(Vector3(0.f, 1.f, 1.f), Vector2(0, 0))
};

const Vertex3D g_northFaceVertexes[] = 
{
	Vertex3D(Vector3(1.f, 1.f, 0.f), Vector2(0, 1)),
	Vertex3D(Vector3(0.f, 1.f, 0.f), Vector2(1, 1)),
	Vertex3D(Vector3(0.f, 1.f, 1.f), Vector2(1, 0)),
	Vertex3D(Vector3(1.f, 1.f, 1.f), Vector2(0, 0))

};

const Vertex3D g_southFaceVertexes[] = 
{
	Vertex3D(Vector3(0.f, 0.f, 0.f), Vector2(0, 1)),
	Vertex3D(Vector3(1.f, 0.f, 0.f), Vector2(1, 1)),
	Vertex3D(Vector3(1.f, 0.f, 1.f), Vector2(1, 0)),
	Vertex3D(Vector3(0.f, 0.f, 1.f), Vector2(0, 0))
};

const Vertex3D g_eastFaceVertexes[] = 
{
	Vertex3D(Vector3(1.f, 0.f, 0.f), Vector2(0, 1)),
	Vertex3D(Vector3(1.f, 1.f, 0.f), Vector2(1, 1)),
	Vertex3D(Vector3(1.f, 1.f, 1.f), Vector2(1, 0)),
	Vertex3D(Vector3(1.f, 0.f, 1.f), Vector2(0, 0))
};

const Vertex3D g_westFaceVertexes[] = 
{
	Vertex3D(Vector3(0.f, 1.f, 0.f), Vector2(0, 1)),
	Vertex3D(Vector3(0.f, 0.f, 0.f), Vector2(1, 1)),
	Vertex3D(Vector3(0.f, 0.f, 1.f), Vector2(1, 0)),
	Vertex3D(Vector3(0.f, 1.f, 1.f), Vector2(0, 0))
};



std::vector<Vertex3D> g_debugPoints;

void AddDebugPoint(const Vector3& position, const Rgba& color)
{
	g_debugPoints.push_back(Vertex3D(position, Vector2::ZERO, color));
}

void DrawDebugPoints(float pointSize, bool enableDepthTesting)
{
	if (g_theRenderer)
	{
		g_theRenderer->SetDepthTesting(enableDepthTesting);
		g_theRenderer->DrawPoints(g_debugPoints.data(), g_debugPoints.size(), pointSize);
	}
}

void ClearDebugPoints()
{
	g_debugPoints.erase(g_debugPoints.begin(), g_debugPoints.end());
}

bool operator<(IntVector2 lhs, IntVector2 rhs)
{
	//Compare x values
	if (lhs.x < rhs.x)
		return true;
	if (rhs.x < lhs.x)
		return false;

	//If x values are equal, compare y values
	if (lhs.y < rhs.y)
		return true;

	return false;
}
