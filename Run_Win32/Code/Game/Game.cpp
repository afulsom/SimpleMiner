#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ProfileLogScope.hpp"
#include "TreeDefinition.hpp"



Game::Game()
	: m_theCamera()
	, m_thePlayer()
	, isGamePaused(false)
	, isScreenShaking(false)
	, screenShakeLifetime(0.f)
	, m_theWorld()
	, m_selectedBlockIndex(0)
	, m_inventoryBlocks()
{
	Texture* spriteAtlas = g_theRenderer->CreateOrGetTexture("Data/Images/SimpleMinerAtlas.png");
	g_blockSprites = new SpriteSheet(spriteAtlas, 16, 16);

	g_squirrelFont = g_theRenderer->CreateOrGetFont("SquirrelFixedFont.png");

	m_selectionSoundID = g_theAudio->CreateOrGetSound(BLOCK_SELECTION_SOUND_PATH);

	InitializeBlockTypes();
	LoadSounds();

	InitializeTrees();

	std::vector<unsigned char> inputBuffer;
	if (ReadBufferFromFile(inputBuffer, g_PLAYER_SAVE_FILE_PATH))
	{
		LoadPlayerState(inputBuffer);
	}
	else
	{
		m_theCamera.m_yaw = 0.f;
		m_theCamera.m_pitch = 0.f;
		m_theCamera.m_roll = 0.f;

		m_thePlayer.SetCenterPosition(Vector3(0.f, 0.f, 80.f));
	}
	

	m_theWorld = new World();
}

Game::~Game()
{

}


void Game::Update(float deltaSeconds)
{
	DeveloperInput();
	UpdatePlayer(deltaSeconds);
	m_theWorld->Update(deltaSeconds, m_thePlayer.GetCenterPosition());
	UpdateCamera(deltaSeconds);
}

void Game::Render() const
{	
	//3D rendering
	SetUpCamera();
	DrawWorld();

	g_theRenderer->SetDepthTesting(false);
	if(m_theCamera.m_mode != CAMERA_FIRST_PERSON)
	{
		m_thePlayer.Render();
	}

	Vector3 forwardDirection = m_thePlayer.GetForwardXYZ();
	RaycastResult rayResults = Raycast(m_thePlayer.GetEyePosition(), forwardDirection.GetNormalized(), 8.f);
	HighlightTargettedBlock(rayResults);

	if(m_thePlayer.m_isAttached)
		DrawHookLine();

	if(g_drawDebug)
	{
		DrawWorldAxes();
		DrawTargettingLine(m_thePlayer.GetEyePosition(), m_thePlayer.GetEyePosition() + (forwardDirection * 8.f));
		DrawDebugPoints(5.f, true);
	}

	//Start drawing in 2D
	g_theRenderer->SetOrtho(Vector2::ZERO, Vector2(g_GAME_WIDTH, g_GAME_HEIGHT));
	g_theRenderer->SetDepthTesting(false);
	g_theRenderer->BindTexture(nullptr);

	DrawCrosshair();
	DrawBlockSelectionHUD();
	DrawDebugText();
}

void Game::LoadSounds()
{
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_DIRT]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(DIRT_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_DIRT]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(DIRT_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_DIRT]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(DIRT_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_DIRT]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(DIRT_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_DIRT]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(DIRT_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_DIRT]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(DIRT_FOOTSTEP_2_SOUND_PATH));

	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRASS]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(GRASS_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRASS]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(GRASS_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRASS]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(GRASS_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRASS]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(GRASS_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRASS]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(GRASS_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRASS]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(GRASS_FOOTSTEP_2_SOUND_PATH));

	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SAND]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(SAND_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SAND]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(SAND_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SAND]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(SAND_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SAND]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(SAND_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SAND]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(SAND_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SAND]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(SAND_FOOTSTEP_2_SOUND_PATH));

	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRAVEL]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(GRAVEL_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRAVEL]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(GRAVEL_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRAVEL]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(GRAVEL_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRAVEL]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(GRAVEL_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRAVEL]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(GRAVEL_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRAVEL]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(GRAVEL_FOOTSTEP_2_SOUND_PATH));

	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_STONE]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(STONE_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_STONE]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(STONE_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_STONE]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(STONE_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_STONE]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(STONE_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_STONE]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(STONE_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_STONE]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(STONE_FOOTSTEP_2_SOUND_PATH));

	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_COBBLESTONE]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(COBBLESTONE_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_COBBLESTONE]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(COBBLESTONE_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_COBBLESTONE]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(COBBLESTONE_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_COBBLESTONE]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(COBBLESTONE_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_COBBLESTONE]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(COBBLESTONE_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_COBBLESTONE]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(COBBLESTONE_FOOTSTEP_2_SOUND_PATH));

	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GLOWSTONE]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(GLOWSTONE_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GLOWSTONE]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(GLOWSTONE_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GLOWSTONE]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(GLOWSTONE_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GLOWSTONE]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(GLOWSTONE_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GLOWSTONE]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(GLOWSTONE_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GLOWSTONE]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(GLOWSTONE_FOOTSTEP_2_SOUND_PATH));

	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_PLANKS]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_PLANKS_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_PLANKS]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_PLANKS_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_PLANKS]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_PLANKS_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_PLANKS]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_PLANKS_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_PLANKS]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_PLANKS_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_PLANKS]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_PLANKS_FOOTSTEP_2_SOUND_PATH));

	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_LOG]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_LOG_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_LOG]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_LOG_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_LOG]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_LOG_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_LOG]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_LOG_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_LOG]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_LOG_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_LOG]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(WOOD_LOG_FOOTSTEP_2_SOUND_PATH));

	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_LEAVES]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(LEAVES_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_LEAVES]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(LEAVES_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_LEAVES]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(LEAVES_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_LEAVES]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(LEAVES_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_LEAVES]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(LEAVES_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_LEAVES]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(LEAVES_FOOTSTEP_2_SOUND_PATH));

	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WATER]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(WATER_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WATER]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(WATER_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WATER]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(WATER_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WATER]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(WATER_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WATER]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(WATER_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WATER]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(WATER_FOOTSTEP_2_SOUND_PATH));

	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SNOW]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(SNOW_BREAK_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SNOW]->m_breakSounds.push_back(g_theAudio->CreateOrGetSound(SNOW_BREAK_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SNOW]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(SNOW_PLACE_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SNOW]->m_placeSounds.push_back(g_theAudio->CreateOrGetSound(SNOW_PLACE_2_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SNOW]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(SNOW_FOOTSTEP_1_SOUND_PATH));
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SNOW]->m_footstepSounds.push_back(g_theAudio->CreateOrGetSound(SNOW_FOOTSTEP_2_SOUND_PATH));
}

void Game::PlayFootstepSound()
{
	if(m_thePlayer.m_footstepSoundDebt >= m_thePlayer.m_footstepSoundDebtThreshold && m_thePlayer.m_isGrounded)
	{
		BlockInfo blockBelowPlayer = m_theWorld->GetBlockInfoFromWorldCoords(m_thePlayer.GetBottomPosition());
		blockBelowPlayer.MoveDown();
		if(blockBelowPlayer.m_chunk && blockBelowPlayer.GetBlock()->GetBlockType() != BLOCK_TYPE_AIR)
		{
			g_theAudio->PlaySound(BlockDefinition::s_blockDefinitions[blockBelowPlayer.GetBlock()->GetBlockType()]->GetRandomFootstepSound(), 0.5f);

			m_thePlayer.m_footstepSoundDebt = 0.f;
			m_thePlayer.m_footstepSoundDebtThreshold = m_thePlayer.GetRandomFootstepSoundThreshold();
		}
	}
}

void Game::InitializeBlockTypes()
{
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_AIR] = new BlockDefinition(AIR_SPRITE_INDEX, AIR_SPRITE_INDEX, AIR_SPRITE_INDEX, AIR_SPRITE_INDEX, AIR_SPRITE_INDEX, AIR_SPRITE_INDEX, false, false, 0);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_DIRT] = new BlockDefinition(DIRT_SPRITE_INDEX, DIRT_SPRITE_INDEX, DIRT_SPRITE_INDEX, DIRT_SPRITE_INDEX, DIRT_SPRITE_INDEX, DIRT_SPRITE_INDEX, true, true, 0);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRASS] = new BlockDefinition(GRASS_TOP_SPRITE_INDEX, GRASS_BOTTOM_SPRITE_INDEX, GRASS_SIDE_SPRITE_INDEX, GRASS_SIDE_SPRITE_INDEX, GRASS_SIDE_SPRITE_INDEX, GRASS_SIDE_SPRITE_INDEX, true, true, 0);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SAND] = new BlockDefinition(SAND_SPRITE_INDEX, SAND_SPRITE_INDEX, SAND_SPRITE_INDEX, SAND_SPRITE_INDEX, SAND_SPRITE_INDEX, SAND_SPRITE_INDEX, true, true, 0);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GRAVEL] = new BlockDefinition(GRAVEL_SPRITE_INDEX, GRAVEL_SPRITE_INDEX, GRAVEL_SPRITE_INDEX, GRAVEL_SPRITE_INDEX, GRAVEL_SPRITE_INDEX, GRAVEL_SPRITE_INDEX, true, true, 0);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_STONE] = new BlockDefinition(STONE_SPRITE_INDEX, STONE_SPRITE_INDEX, STONE_SPRITE_INDEX, STONE_SPRITE_INDEX, STONE_SPRITE_INDEX, STONE_SPRITE_INDEX, true, true, 0);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_COBBLESTONE] = new BlockDefinition(COBBLESTONE_SPRITE_INDEX, COBBLESTONE_SPRITE_INDEX, COBBLESTONE_SPRITE_INDEX, COBBLESTONE_SPRITE_INDEX, COBBLESTONE_SPRITE_INDEX, COBBLESTONE_SPRITE_INDEX, true, true, 0);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_GLOWSTONE] = new BlockDefinition(GLOWSTONE_SPRITE_INDEX, GLOWSTONE_SPRITE_INDEX, GLOWSTONE_SPRITE_INDEX, GLOWSTONE_SPRITE_INDEX, GLOWSTONE_SPRITE_INDEX, GLOWSTONE_SPRITE_INDEX, true, true, 12);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_PLANKS] = new BlockDefinition(WOOD_PLANKS_SPRITE_INDEX, WOOD_PLANKS_SPRITE_INDEX, WOOD_PLANKS_SPRITE_INDEX, WOOD_PLANKS_SPRITE_INDEX, WOOD_PLANKS_SPRITE_INDEX, WOOD_PLANKS_SPRITE_INDEX, true, true, 0);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WOOD_LOG] = new BlockDefinition(WOOD_LOG_TOP_BOTTOM_SPRITE_INDEX, WOOD_LOG_TOP_BOTTOM_SPRITE_INDEX, WOOD_LOG_SIDE_SPRITE_INDEX, WOOD_LOG_SIDE_SPRITE_INDEX, WOOD_LOG_SIDE_SPRITE_INDEX, WOOD_LOG_SIDE_SPRITE_INDEX, true, true, 0);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_LEAVES] = new BlockDefinition(LEAVES_SPRITE_INDEX, LEAVES_SPRITE_INDEX, LEAVES_SPRITE_INDEX, LEAVES_SPRITE_INDEX, LEAVES_SPRITE_INDEX, LEAVES_SPRITE_INDEX, true, true, 0);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_WATER] = new BlockDefinition(WATER_SPRITE_INDEX, WATER_SPRITE_INDEX, WATER_SPRITE_INDEX, WATER_SPRITE_INDEX, WATER_SPRITE_INDEX, WATER_SPRITE_INDEX, true, true, 0);
	BlockDefinition::s_blockDefinitions[BLOCK_TYPE_SNOW] = new BlockDefinition(SNOW_SPRITE_INDEX, SNOW_SPRITE_INDEX, SNOW_SPRITE_INDEX, SNOW_SPRITE_INDEX, SNOW_SPRITE_INDEX, SNOW_SPRITE_INDEX, true, true, 0);

	m_inventoryBlocks.push_back(BLOCK_TYPE_DIRT);
	m_inventoryBlocks.push_back(BLOCK_TYPE_GRASS);
	m_inventoryBlocks.push_back(BLOCK_TYPE_SAND);
	m_inventoryBlocks.push_back(BLOCK_TYPE_GRAVEL);
	m_inventoryBlocks.push_back(BLOCK_TYPE_STONE);
	m_inventoryBlocks.push_back(BLOCK_TYPE_COBBLESTONE);
	m_inventoryBlocks.push_back(BLOCK_TYPE_GLOWSTONE);
	m_inventoryBlocks.push_back(BLOCK_TYPE_WOOD_PLANKS);
	m_inventoryBlocks.push_back(BLOCK_TYPE_WOOD_LOG);
	m_inventoryBlocks.push_back(BLOCK_TYPE_WATER);

}

void Game::UpdatePlayerActionsFromMouse()
{
	if (g_theApp->HasFocus())
	{
		Vector3 forwardDirection = m_thePlayer.GetForwardXYZ();
		RaycastResult rayResults = Raycast(m_thePlayer.GetEyePosition(), forwardDirection.GetNormalized(), 8.f);

		if (rayResults.m_didImpact)
		{
			if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
			{
				DestroyTargettedBlock(rayResults);
			}

			if (g_theInput->WasKeyJustPressed(KEYCODE_RMB))
			{
				PlaceBlock(m_inventoryBlocks[m_selectedBlockIndex], rayResults);
			}
		}

		if (g_theInput->GetMouseWheelRotation() > 0)
		{
			--m_selectedBlockIndex;
			if (m_selectedBlockIndex < 0)
				m_selectedBlockIndex = g_NUM_SELECTION_BLOCKS - 1;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}
		else if (g_theInput->GetMouseWheelRotation() < 0)
		{
			++m_selectedBlockIndex;
			m_selectedBlockIndex %= g_NUM_SELECTION_BLOCKS;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}

		if (g_theInput->WasKeyJustPressed('1'))
		{
			m_selectedBlockIndex = 0;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}
		else if (g_theInput->WasKeyJustPressed('2'))
		{
			m_selectedBlockIndex = 1;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}
		else if (g_theInput->WasKeyJustPressed('3'))
		{
			m_selectedBlockIndex = 2;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}
		else if (g_theInput->WasKeyJustPressed('4'))
		{
			m_selectedBlockIndex = 3;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}
		else if (g_theInput->WasKeyJustPressed('5'))
		{
			m_selectedBlockIndex = 4;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}
		else if (g_theInput->WasKeyJustPressed('6'))
		{
			m_selectedBlockIndex = 5;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}
		else if (g_theInput->WasKeyJustPressed('7'))
		{
			m_selectedBlockIndex = 6;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}
		else if (g_theInput->WasKeyJustPressed('8'))
		{
			m_selectedBlockIndex = 7;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}
		else if (g_theInput->WasKeyJustPressed('9'))
		{
			m_selectedBlockIndex = 8;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}
		else if (g_theInput->WasKeyJustPressed('0'))
		{
			m_selectedBlockIndex = 9;

			g_theAudio->PlaySound(m_selectionSoundID, 0.5f);
		}

	}
}

void Game::UpdatePlayerFromController(float deltaSeconds)
{
	float playerVelocity = 2.5f;

	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		playerVelocity *= 8.f;
	}

	XboxController controller = g_theInput->GetControllerState(0);
	if (controller.m_isConnected)
	{
		if (controller.m_isConnected && controller.m_leftStick.m_magnitude > 0.f)
		{
			m_theCamera.m_position += m_theCamera.GetForwardXY() * CosDegrees(controller.m_leftStick.m_degrees - 90.f) * playerVelocity * deltaSeconds;
			m_theCamera.m_position += m_theCamera.GetLeftXY() * SinDegrees(controller.m_leftStick.m_degrees - 90.f) * playerVelocity * deltaSeconds;
		}

		if (controller.m_isConnected && controller.m_rightStick.m_magnitude > 0.f)
		{
			float yaw = -CosDegrees(controller.m_rightStick.m_degrees) * 90.f * deltaSeconds;
			float pitch = -SinDegrees(controller.m_rightStick.m_degrees) * 90.f * deltaSeconds;
			m_theCamera.m_yaw += yaw;
			m_theCamera.m_pitch += pitch;
		}

		if (controller.IsButtonDown(XBOX_RIGHT_SHOULDER))
		{
			m_theCamera.m_position += Vector3::Z_AXIS * playerVelocity * deltaSeconds;
		}
		else if (controller.IsButtonDown(XBOX_LEFT_SHOUDLER))
		{
			m_theCamera.m_position -= Vector3::Z_AXIS * playerVelocity * deltaSeconds;
		}
	}
}

void Game::UpdatePlayerMovementFromKeyboard(float deltaSeconds)
{
	float playerSpeed = PLAYER_SPEED;

	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		playerSpeed *= 8.f;
	}

	bool didPlayerInputMove = false;
	switch (m_thePlayer.m_mode)
	{
	case PLAYER_WALKING:
	{

		//Forward Back
		if (g_theInput->IsKeyDown('W'))

		{
			m_thePlayer.m_velocity += m_thePlayer.GetForwardXY() * playerSpeed * deltaSeconds;
			didPlayerInputMove = true;
		}
		else if (g_theInput->IsKeyDown('S'))
		{
			m_thePlayer.m_velocity -= m_thePlayer.GetForwardXY() * playerSpeed * deltaSeconds;
			didPlayerInputMove = true;
		}

		//Left Right
		if (g_theInput->IsKeyDown('A'))
		{
			m_thePlayer.m_velocity += m_thePlayer.GetLeftXY() * playerSpeed * deltaSeconds;
			didPlayerInputMove = true;
		}
		else if (g_theInput->IsKeyDown('D'))
		{
			m_thePlayer.m_velocity -= m_thePlayer.GetLeftXY() * playerSpeed * deltaSeconds;
			didPlayerInputMove = true;
		}

		if (g_theInput->WasKeyJustPressed('E'))
		{
			AttachHook(m_thePlayer.GetEyePosition(), m_thePlayer.GetForwardXYZ(), HOOK_ATTACH_DISTANCE);
		}

		if (m_thePlayer.m_isAttached && g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
		{
			DetachHook();
			if(!m_thePlayer.m_isGrounded)
			{
				m_thePlayer.m_velocity.z += PLAYER_JUMP_SPEED;
			}
		}

		float playerSpeedBeforeCorrection = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f).CalcLength();
		if (playerSpeedBeforeCorrection > playerSpeed)
		{
			Vector3 playerXYVelocity(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
			playerXYVelocity.SetLength(playerSpeed);
			m_thePlayer.m_velocity.x = playerXYVelocity.x;
			m_thePlayer.m_velocity.y = playerXYVelocity.y;
		}

		if(m_thePlayer.m_isGrounded)
		{
			//Jumping
			if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
			{
				m_thePlayer.m_velocity.z += PLAYER_JUMP_SPEED;
				break;
			}

			if (didPlayerInputMove == false)
			{
				m_thePlayer.m_velocity -= Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f) * SURFACE_FRICTION * deltaSeconds;
				if (Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f).CalcLength() < playerSpeed * deltaSeconds * 0.5f)
				{
					m_thePlayer.m_velocity.x = 0.f;
					m_thePlayer.m_velocity.y = 0.f;
				}
			}

			m_thePlayer.m_footstepSoundDebt += deltaSeconds * RangeMapFloat(m_thePlayer.m_velocity.CalcLength(), 0.f, PLAYER_SPEED, 0.f, 1.f);
		}
	}
		break;

	case PLAYER_FLYING:
		//Forward Back
		if (g_theInput->IsKeyDown('W'))
		{
			m_thePlayer.m_velocity += m_thePlayer.GetForwardXY() * playerSpeed * deltaSeconds;
		}
		else if (g_theInput->IsKeyDown('S'))
		{
			m_thePlayer.m_velocity -= m_thePlayer.GetForwardXY() * playerSpeed * deltaSeconds;
		}

		//Left Right
		if (g_theInput->IsKeyDown('A'))
		{
			m_thePlayer.m_velocity += m_thePlayer.GetLeftXY() * playerSpeed * deltaSeconds;
		}
		else if (g_theInput->IsKeyDown('D'))
		{
			m_thePlayer.m_velocity -= m_thePlayer.GetLeftXY() * playerSpeed * deltaSeconds;
		}

		//Up Down
		if (g_theInput->IsKeyDown(KEYCODE_SPACE))
		{
			m_thePlayer.m_velocity += Vector3::Z_AXIS * playerSpeed * deltaSeconds;
		}
		else if (g_theInput->IsKeyDown(KEYCODE_CTRL))
		{
			m_thePlayer.m_velocity -= Vector3::Z_AXIS * playerSpeed * deltaSeconds;
		}
		break;
	}
	
}

void Game::UpdatePlayerOrientationFromKeyboard(float deltaSeconds)
{
	//Look up / down
	if (g_theInput->IsKeyDown(KEYCODE_UP))
	{
		m_thePlayer.m_pitch -= 90.f * deltaSeconds;
	}
	else if (g_theInput->IsKeyDown(KEYCODE_DOWN))
	{
		m_thePlayer.m_pitch += 90.f * deltaSeconds;
	}

	//Look left / right
	if (g_theInput->IsKeyDown(KEYCODE_LEFT))
	{
		m_thePlayer.m_yaw += 90.f * deltaSeconds;
	}
	else if (g_theInput->IsKeyDown(KEYCODE_RIGHT))
	{
		m_thePlayer.m_yaw -= 90.f * deltaSeconds;
	}

	m_thePlayer.m_pitch = ClampFloat(m_thePlayer.m_pitch, -89.99f, 89.99f);
}

void Game::UpdatePlayerViewFromMouse()
{
	if (g_theApp->HasFocus())
	{
		IntVector2 mouseScreenPos = g_theInput->GetCursorScreenPos();
		IntVector2 screenCenter = g_theInput->GetScreenCenter();

		IntVector2 mouseDeltaMove = mouseScreenPos - screenCenter;

		g_theInput->SetCursorScreenPos(screenCenter);

		float mouseMovedX = (float)mouseDeltaMove.x;
		float mouseMovedY = (float)mouseDeltaMove.y;
		float mouseSensitivityY = 0.03f;
		float mouseSensitivityX = 0.03f;
		m_thePlayer.m_yaw -= mouseMovedX * mouseSensitivityX;
		m_thePlayer.m_pitch += mouseMovedY * mouseSensitivityY;

		m_thePlayer.m_pitch = ClampFloat(m_thePlayer.m_pitch, -89.99f, 89.99f);
	}
}

void Game::DrawCrosshair() const
{
	Vector2 screenCenter = Vector2(g_GAME_WIDTH / 2, g_GAME_HEIGHT / 2);
	g_theRenderer->DrawLine(screenCenter - Vector2(10.f, 0.f), screenCenter + Vector2(10.f, 0.f));
	g_theRenderer->DrawLine(screenCenter - Vector2(0.f, 10.f), screenCenter + Vector2(0.f, 10.f));
}

void Game::DrawBlockSelectionHUD() const
{
	Vector2 screenCenter = Vector2(g_GAME_WIDTH / 2, g_GAME_HEIGHT / 2);

	float halfNumBlocks = (float)(g_NUM_SELECTION_BLOCKS / 2);
	float halfHUDWidth = g_HUD_BLOCK_WIDTH * (halfNumBlocks);

	Vector2 HUDBackgroundMins(screenCenter.x - halfHUDWidth, 0.f);
	Vector2 HUDBackgroundMaxs(screenCenter.x + halfHUDWidth, g_HUD_BLOCK_HEIGHT);

	g_theRenderer->DrawAABB2(AABB2(HUDBackgroundMins, HUDBackgroundMaxs), Rgba(0, 0, 0, 90));

	g_theRenderer->SetTexturing(true);
	Vector2 blockHUDOffset(g_HUD_BLOCK_WIDTH * 0.1f, g_HUD_BLOCK_HEIGHT * 0.1f);

	for (int blockSelectionIndex = 0; blockSelectionIndex < g_NUM_SELECTION_BLOCKS; ++blockSelectionIndex)
	{
		Vector2 inventoryBlockMins = Vector2(HUDBackgroundMins.x + (g_HUD_BLOCK_WIDTH * blockSelectionIndex), HUDBackgroundMins.y) + blockHUDOffset;
		Vector2 inventoryBlockMaxs = Vector2(HUDBackgroundMins.x + (g_HUD_BLOCK_WIDTH * (blockSelectionIndex + 1)), HUDBackgroundMaxs.y) - blockHUDOffset;
		
		if (m_selectedBlockIndex == blockSelectionIndex)
		{
			g_theRenderer->SetTexturing(false);
			AABB2 highlightQuad(inventoryBlockMins - (blockHUDOffset / 2), inventoryBlockMaxs + (blockHUDOffset / 2));
			g_theRenderer->DrawAABB2(highlightQuad, Rgba(255, 255, 0, 130));
			g_theRenderer->SetTexturing(true);
		}

		BlockType currentInventoryBlock = m_inventoryBlocks[blockSelectionIndex];
		AABB2 blockFaceTexCoords = g_blockSprites->GetTexCoordsForSpriteIndex(BlockDefinition::s_blockDefinitions[currentInventoryBlock]->m_northSpriteIndex);

		g_theRenderer->DrawTexturedAABB2(AABB2(inventoryBlockMins, inventoryBlockMaxs), *g_blockSprites->GetTexture(), blockFaceTexCoords.mins, blockFaceTexCoords.maxs, Rgba::WHITE);
	}
	g_theRenderer->SetTexturing(false);
}

void Game::ScreenShake() const
{
	float offset = GetRandomFloatInRange(-screenShakeMagnitude, screenShakeMagnitude);
	g_theRenderer->TranslateCoordinates2D(offset, offset);
}

void Game::SetScreenShake(float shakeLifetime, float shakeMagnitude)
{
	isScreenShaking = true;
	SetScreenShakeMagnitude(shakeMagnitude);
	SetScreenShakeLifetime(shakeLifetime);
}

void Game::SetScreenShakeMagnitude(float shakeMagnitude)
{
	screenShakeMagnitude = shakeMagnitude;
}

void Game::SetScreenShakeLifetime(float shakeLifetime)
{
	screenShakeLifetime = shakeLifetime;
}

void Game::DecrementShakeTime(float deltaSeconds)
{
	screenShakeLifetime -= deltaSeconds;
	if (screenShakeLifetime <= 0.f)
	{
		isScreenShaking = false;
	}
}

Vector2 Game::GetRandomOffscreenPosition(float entityRadius) const
{
	Vector2 position;
	int randomSide = GetRandomIntLessThan(4);
	if (randomSide == 0) //Left
	{
		position.x = -entityRadius;
		position.y = GetRandomFloatInRange(0.f, g_GAME_HEIGHT);
	}
	else if (randomSide == 1) //Right
	{
		position.x = g_GAME_WIDTH + entityRadius;
		position.y = GetRandomFloatInRange(0.f, g_GAME_HEIGHT);
	}
	else if (randomSide == 2) //Bottom
	{
		position.x = GetRandomFloatInRange(0.f, g_GAME_WIDTH);
		position.y = -entityRadius;
	}
	else //Top
	{
		position.x = GetRandomFloatInRange(0.f, g_GAME_WIDTH);
		position.y = g_GAME_HEIGHT + entityRadius;
	}
	return position;
}

void Game::UpdatePlayer(float deltaSeconds)
{
	if(m_theCamera.m_mode != CAMERA_NO_CLIP)
	{
		m_thePlayer.m_isGrounded = CheckIfPlayerIsOnGround();
		UpdatePlayerMovementFromKeyboard(deltaSeconds);
		UpdatePlayerOrientationFromKeyboard(deltaSeconds);
		UpdatePlayerViewFromMouse();
	}
	ApplyGlobalForces(deltaSeconds);
	UpdatePlayerActionsFromMouse();

	if(m_thePlayer.m_isAttached)
	{
		CorrectForHook();
	}
	m_thePlayer.Update(deltaSeconds);

	ApplyPhysicsToPlayer();
	PlayFootstepSound();
}

void Game::ApplyCameraTransform(const Camera3D& transformCamera) const
{
	g_theRenderer->RotateCoordinates3D(-transformCamera.m_roll, Vector3::X_AXIS);
	g_theRenderer->RotateCoordinates3D(-transformCamera.m_pitch, Vector3::Y_AXIS);
	g_theRenderer->RotateCoordinates3D(-transformCamera.m_yaw, Vector3::Z_AXIS);

	g_theRenderer->TranslateCoordinates3D(Vector3(-transformCamera.m_position.x, -transformCamera.m_position.y, -transformCamera.m_position.z));
}

void Game::DrawDebugText() const
{
	float textHeight = 15.f;
	float textAspectRatio = 1.f;

	if(g_drawDebug)
	{
		Vector3 playerPosition = m_thePlayer.GetCenterPosition();

		std::string xText = "X: " + std::to_string(playerPosition.x);
		std::string yText = " Y: " + std::to_string(playerPosition.y);
		std::string zText = " Z: " + std::to_string(playerPosition.z);

		std::string yawText = "Yaw: " + std::to_string(m_thePlayer.m_yaw);
		std::string pitchText = " Pitch: " + std::to_string(m_thePlayer.m_pitch);
		std::string rollText = " Roll: " + std::to_string(m_theCamera.m_roll);

		Vector2 positionInformationPos = Vector2(0.f, g_GAME_HEIGHT - textHeight);
		Vector2 orientationInformationPos = Vector2(0.f, g_GAME_HEIGHT - (textHeight * 2));
		g_theRenderer->DrawText2D(positionInformationPos, xText + yText + zText, textHeight, Rgba::WHITE, textAspectRatio, g_squirrelFont);
		g_theRenderer->DrawText2D(orientationInformationPos, yawText + pitchText + rollText, textHeight, Rgba::WHITE, textAspectRatio, g_squirrelFont);

		std::string movementMode = "";
		switch (m_thePlayer.m_mode)
		{
		case PLAYER_WALKING:
			movementMode = "Walking";
			break;

		case PLAYER_FLYING:
			movementMode = "Flying";
			break;
		}

		std::string cameraMode = "";
		switch (m_theCamera.m_mode)
		{
		case CAMERA_FIRST_PERSON:
			cameraMode = "First-Person";
			break;

		case CAMERA_FROM_BEHIND:
			cameraMode = "From-Behind";
			break;

		case CAMERA_FIXED_ANGLE:
			cameraMode = "Fixed-Angle";
			break;

		case CAMERA_NO_CLIP:
			cameraMode = "No-Clip";
			break;
		}

		Vector2 modeInformationPos = Vector2(0.f, g_GAME_HEIGHT - (textHeight * 3));
		g_theRenderer->DrawText2D(modeInformationPos, movementMode + " " + cameraMode, textHeight, Rgba::WHITE, textAspectRatio, g_squirrelFont);
	}
	else
	{
		Vector2 controlInformationPos = Vector2(0.f, g_GAME_HEIGHT - textHeight);
		g_theRenderer->DrawText2D(controlInformationPos, std::string("WASD: Move, Mouse: Look, E: Grappling Hook, F1: Toggle Debug, F5: Camera Mode, F6: Movement Mode"), textHeight, Rgba::WHITE, textAspectRatio, g_squirrelFont);
	}

}


RaycastResult Game::Raycast(const Vector3& startPosition, const Vector3& direction, float maxDistance) const
{
	int numSteps = 1000;
	RaycastResult result;
	Vector3 endPosition = startPosition + (direction * maxDistance);
	Vector3 displacement = endPosition - startPosition;
	Vector3 singleStep = displacement / (float)numSteps;
	for (int stepIndex = 1; stepIndex < numSteps; stepIndex++)
	{
		Vector3 previousPosition = startPosition + (singleStep * (float)(stepIndex - 1));
		Vector3 currentPosition = startPosition + (singleStep * (float)stepIndex);
		BlockInfo currentBlock = m_theWorld->GetBlockInfoFromWorldCoords(currentPosition);
		if (currentBlock.m_chunk && BlockDefinition::s_blockDefinitions[currentBlock.GetBlock()->GetBlockType()]->m_isOpaque)
		{
			result.m_didImpact = true;
			result.m_impactedBlock = currentBlock;

			Vector3 currentDisplacement = currentPosition - startPosition;
			result.m_impactFraction = currentDisplacement.CalcLength() / maxDistance;

			result.m_pointBeforeImpact = previousPosition;
			result.m_impactPosition = currentPosition;

			Vector3 previousPositionBlockCoords(ceilf(previousPosition.x), ceilf(previousPosition.y), ceilf(previousPosition.z));
			Vector3 currentPositionBlockCoords(ceilf(currentPosition.x), ceilf(currentPosition.y), ceilf(currentPosition.z));

			Vector3 blockDisplacement = previousPositionBlockCoords - currentPositionBlockCoords;

			float xDisplacement = fabs(blockDisplacement.x);
			float yDisplacement = fabs(blockDisplacement.y);
			float zDisplacement = fabs(blockDisplacement.z);
			if (xDisplacement == 1.f && yDisplacement == 1.f)
			{
				blockDisplacement.y = 0.f;
			}
			else if (xDisplacement == 1.f && zDisplacement == 1.f)
			{
				blockDisplacement.x = 0.f;
			}
			else if (yDisplacement == 1.f && zDisplacement == 1.f)
			{
				blockDisplacement.y = 0.f;
			}

			result.m_blockBeforeImpact = m_theWorld->GetBlockInfoFromWorldCoords(currentPosition + blockDisplacement);

			result.m_impactNormal = blockDisplacement.GetNormalized();

			return result;
		}
	}
	result.m_didImpact = false;
	result.m_impactedBlock.m_chunk = nullptr;
	result.m_impactedBlock.m_blockIndex = 0;
	result.m_impactFraction = 1.0f;
	result.m_impactPosition = startPosition + (direction * maxDistance);
	result.m_pointBeforeImpact = result.m_impactPosition;
	return result;
}

void Game::HighlightTargettedBlock(const RaycastResult& rayResults) const
{
	if (rayResults.m_didImpact)
	{
		g_theRenderer->SetDepthTesting(true);
		Vector3 extrusionVector = rayResults.m_impactNormal * 0.01f;

		Vector3 blockCoords = rayResults.m_impactedBlock.m_chunk->GetChunkWorldMins() + Vector3(rayResults.m_impactedBlock.m_chunk->GetBlockCoordsForBlockIndex(rayResults.m_impactedBlock.m_blockIndex));
		Vertex3D faceHighlights[4];
		if (rayResults.m_impactNormal.x > 0.f)	//East
		{
			faceHighlights[0] = Vertex3D(blockCoords + g_eastFaceVertexes[0].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[1] = Vertex3D(blockCoords + g_eastFaceVertexes[1].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[2] = Vertex3D(blockCoords + g_eastFaceVertexes[2].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[3] = Vertex3D(blockCoords + g_eastFaceVertexes[3].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
		}
		else if (rayResults.m_impactNormal.x < 0.f)	//West
		{
			faceHighlights[0] = Vertex3D(blockCoords + g_westFaceVertexes[0].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[1] = Vertex3D(blockCoords + g_westFaceVertexes[1].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[2] = Vertex3D(blockCoords + g_westFaceVertexes[2].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[3] = Vertex3D(blockCoords + g_westFaceVertexes[3].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
		}
		else if (rayResults.m_impactNormal.y > 0.f)	//North
		{
			faceHighlights[0] = Vertex3D(blockCoords + g_northFaceVertexes[0].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[1] = Vertex3D(blockCoords + g_northFaceVertexes[1].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[2] = Vertex3D(blockCoords + g_northFaceVertexes[2].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[3] = Vertex3D(blockCoords + g_northFaceVertexes[3].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
		}
		else if (rayResults.m_impactNormal.y < 0.f)	//South
		{
			faceHighlights[0] = Vertex3D(blockCoords + g_southFaceVertexes[0].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[1] = Vertex3D(blockCoords + g_southFaceVertexes[1].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[2] = Vertex3D(blockCoords + g_southFaceVertexes[2].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[3] = Vertex3D(blockCoords + g_southFaceVertexes[3].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
		}
		else if (rayResults.m_impactNormal.z > 0.f)	//Top
		{
			faceHighlights[0] = Vertex3D(blockCoords + g_topFaceVertexes[0].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[1] = Vertex3D(blockCoords + g_topFaceVertexes[1].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[2] = Vertex3D(blockCoords + g_topFaceVertexes[2].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[3] = Vertex3D(blockCoords + g_topFaceVertexes[3].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
		}
		else if (rayResults.m_impactNormal.z < 0.f)	//Bottom
		{
			faceHighlights[0] = Vertex3D(blockCoords + g_bottomFaceVertexes[0].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[1] = Vertex3D(blockCoords + g_bottomFaceVertexes[1].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[2] = Vertex3D(blockCoords + g_bottomFaceVertexes[2].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
			faceHighlights[3] = Vertex3D(blockCoords + g_bottomFaceVertexes[3].position + extrusionVector, Vector2::ZERO, Rgba(255, 255, 255, 75));
		}

		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawLineLoop(faceHighlights, 4, 30.f);

		faceHighlights[0].color = Rgba(255, 255, 255, 15);
		faceHighlights[1].color = Rgba(255, 255, 255, 15);
		faceHighlights[2].color = Rgba(255, 255, 255, 15);
		faceHighlights[3].color = Rgba(255, 255, 255, 15);
		g_theRenderer->SetDepthTesting(false);
		g_theRenderer->DrawLineLoop(faceHighlights, 4, 30.f);
		g_theRenderer->SetDepthTesting(true);

	}
}

void Game::DestroyTargettedBlock(const RaycastResult& rayResults)
{
	if (rayResults.m_didImpact)
	{
		BlockInfo impactedBlock = rayResults.m_impactedBlock;
		g_theAudio->PlaySound(BlockDefinition::s_blockDefinitions[impactedBlock.GetBlock()->GetBlockType()]->GetRandomBreakSound(), 0.5f);

		impactedBlock.GetBlock()->ChangeType(BLOCK_TYPE_AIR);
		m_theWorld->DirtyBlockLighting(impactedBlock);
		impactedBlock.m_chunk->MakeDirty();
		if (impactedBlock.GetAboveBlock().GetBlock()->GetIsSky())
		{
			impactedBlock.GetBlock()->SetIsSky();
			BlockInfo currentBlock = impactedBlock.GetBelowBlock();
			while (!currentBlock.GetBlock()->GetIsOpaque())
			{
				currentBlock.GetBlock()->SetIsSky();
				m_theWorld->DirtyBlockLighting(currentBlock);
				currentBlock.MoveDown();
			}
		}
	}
}

void Game::PlaceBlock(BlockType typeOfBlock, const RaycastResult& rayResults)
{
	if (rayResults.m_didImpact)
	{
		BlockInfo newBlock = rayResults.m_blockBeforeImpact;
// 		BlockInfo newBlock = m_theWorld->GetBlockInfoFromWorldCoords(rayResults.m_impactPosition + rayResults.m_impactNormal);
		if(newBlock.m_chunk)
		{
			newBlock.GetBlock()->ChangeType(typeOfBlock);
			m_theWorld->DirtyBlockLighting(newBlock);
			newBlock.m_chunk->MakeDirty();

			g_theAudio->PlaySound(BlockDefinition::s_blockDefinitions[typeOfBlock]->GetRandomPlaceSound(), 0.5f);

			if (newBlock.GetBlock()->GetIsSky() && newBlock.GetBlock()->GetIsOpaque())
			{
				newBlock.GetBlock()->ClearIsSky();
				BlockInfo currentBlock = newBlock.GetBelowBlock();
				while (!currentBlock.GetBlock()->GetIsOpaque())
				{
					currentBlock.GetBlock()->ClearIsSky();
					m_theWorld->DirtyBlockLighting(currentBlock);
					currentBlock.MoveDown();
				}
			}
		}
	}
}

void Game::AttachHook(const Vector3& startPosition, const Vector3& direction, float maxDistance)
{
	RaycastResult hookResults = Raycast(startPosition, direction, maxDistance);

	if (hookResults.m_didImpact == true)
	{
		m_thePlayer.m_isAttached = true;
		m_thePlayer.m_attachedPoints.push_back(hookResults.m_pointBeforeImpact);
		m_thePlayer.m_attachedDist = maxDistance * hookResults.m_impactFraction;
		m_thePlayer.m_attachedBlock = hookResults.m_impactedBlock;
	}
}

void Game::CorrectForHook()
{
	if (m_thePlayer.m_attachedBlock.GetBlock()->GetBlockType() == BLOCK_TYPE_AIR)
	{
		DetachHook();
		return;
	}

	Vector3 displacementToAttachPoint = *(m_thePlayer.m_attachedPoints.end()-1) - m_thePlayer.GetEyePosition();
	float distanceOfCurrentDisplacement = displacementToAttachPoint.CalcLength();
	RaycastResult hookResults = Raycast(m_thePlayer.GetEyePosition(), displacementToAttachPoint.GetNormalized(), distanceOfCurrentDisplacement);

	if(m_thePlayer.m_attachedPoints.size() > 1)
	{
		Vector3 displacementToDetachPoint = *(m_thePlayer.m_attachedPoints.end() - 2) - m_thePlayer.GetEyePosition();
		float distanceOfCurrentDisplacementToDetachPoint = displacementToDetachPoint.CalcLength();
		RaycastResult detachResults = Raycast(m_thePlayer.GetEyePosition(), displacementToDetachPoint.GetNormalized(), distanceOfCurrentDisplacementToDetachPoint);
		if (detachResults.m_didImpact == false)
		{
			RemoveHookPoint();
		}
	}

	if (hookResults.m_didImpact == true)
	{
		AddHookPoint(hookResults.m_pointBeforeImpact);
	}
	else
	{
		if (distanceOfCurrentDisplacement > HOOK_MAX_DISTANCE)
		{
			DetachHook();
		}
		else if (distanceOfCurrentDisplacement > m_thePlayer.m_attachedDist)
		{
			float distanceToCorrect = distanceOfCurrentDisplacement - m_thePlayer.m_attachedDist;
			Vector3 correctionVector = displacementToAttachPoint.GetNormalized();
			correctionVector.SetLength(distanceToCorrect);
			m_thePlayer.SetEyePosition(m_thePlayer.GetEyePosition() + correctionVector);
			m_thePlayer.m_velocity.CancelAlongNormal(correctionVector.GetNormalized());
		}
	}
}

void Game::DeveloperInput()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		g_theApp->SetIsQuitting(true);
	}

	if (g_theApp->IsQuitting())
	{
		m_theWorld->Quit();
		SavePlayerState();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		if (m_thePlayer.m_mode == PLAYER_WALKING)
		{
			m_thePlayer.m_mode = PLAYER_FLYING;
		}
		else if (m_thePlayer.m_mode == PLAYER_FLYING)
		{
			m_thePlayer.m_mode = PLAYER_WALKING;
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		switch (m_theCamera.m_mode)
		{
			case CAMERA_NO_CLIP:
			{
				m_theCamera.m_mode = CAMERA_FIRST_PERSON;
				break;

			}
			case CAMERA_FIXED_ANGLE:
			{
				m_theCamera.m_mode = CAMERA_NO_CLIP;
				break;

			}
			case CAMERA_FROM_BEHIND:
			{
				m_theCamera.m_mode = CAMERA_FIXED_ANGLE;
				break;

			}
			case CAMERA_FIRST_PERSON:
			{
				m_theCamera.m_mode = CAMERA_FROM_BEHIND;
				break;
				
			}
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		g_drawDebug = !g_drawDebug;
	}

}

void Game::SavePlayerState()
{
	std::string filePath = g_PLAYER_SAVE_FILE_PATH;
	std::string playerInfo = "";

	Vector3 playerPosition = m_thePlayer.GetCenterPosition();

	playerInfo += std::to_string(playerPosition.x) + " " + std::to_string(playerPosition.y) + " " + std::to_string(playerPosition.z) + " ";
	playerInfo += std::to_string(m_thePlayer.m_yaw) + " " + std::to_string(m_thePlayer.m_pitch) + " " + std::to_string(m_theCamera.m_roll);
	std::vector<unsigned char> outputBuffer;
	outputBuffer.reserve(playerInfo.size());
	for (std::string::iterator infoIter = playerInfo.begin(); infoIter != playerInfo.end(); infoIter++)
	{
		outputBuffer.push_back(*infoIter);
	}

	WriteBufferToFile(outputBuffer, filePath);
}

void Game::LoadPlayerState(const std::vector<unsigned char>& inputBuffer)
{
	std::string playerInfo = "";
	for (std::vector<unsigned char>::const_iterator infoIter = inputBuffer.begin(); infoIter != inputBuffer.end(); infoIter++)
	{
		playerInfo.push_back(*infoIter);
	}

	size_t currentOffset = 0;

	size_t xPosOffset = playerInfo.find(" ", 0);
	float xPosition = strtof(playerInfo.substr(0, xPosOffset).c_str(), nullptr);
	currentOffset = xPosOffset + 1;

	size_t yPosOffset = playerInfo.find(" ", currentOffset);
	float yPosition = strtof(playerInfo.substr(currentOffset, currentOffset + yPosOffset).c_str(), nullptr);

	currentOffset = yPosOffset + 1;

	size_t zPosOffset = playerInfo.find(" ", currentOffset);
	float zPosition = strtof(playerInfo.substr(currentOffset, currentOffset + zPosOffset).c_str(), nullptr);

	m_thePlayer.SetCenterPosition(Vector3(xPosition, yPosition, zPosition));

	currentOffset = zPosOffset + 1;

	size_t yawOffset = playerInfo.find(" ", currentOffset);
	m_thePlayer.m_yaw = strtof(playerInfo.substr(currentOffset, currentOffset + yawOffset).c_str(), nullptr);

	currentOffset = yawOffset + 1;

	size_t pitchOffset = playerInfo.find(" ", currentOffset);
	m_thePlayer.m_pitch = strtof(playerInfo.substr(currentOffset, currentOffset + pitchOffset).c_str(), nullptr);

	currentOffset = pitchOffset + 1;

	size_t rollOffset = playerInfo.find(" ", currentOffset);
	m_theCamera.m_roll = strtof(playerInfo.substr(currentOffset, currentOffset + rollOffset).c_str(), nullptr);
}

void Game::ApplyGlobalForces(float deltaSeconds)
{
	switch (m_thePlayer.m_mode)
	{
	case PLAYER_WALKING:
		m_thePlayer.m_velocity.z -= GRAVITY * deltaSeconds;
		break;

	case PLAYER_FLYING:
		if (m_thePlayer.m_velocity != Vector3::ZERO)
		{
			m_thePlayer.m_velocity -= m_thePlayer.m_velocity * FLYING_AIR_RESISTANCE * deltaSeconds;
		}
		break;
	}
}

void Game::DrawTargettingLine(const Vector3& startPosition, const Vector3& endPosition) const
{
	g_theRenderer->SetDepthTesting(false);
	Vertex3D lineVertexes[] = 
	{
		Vertex3D(startPosition, Vector2::ZERO, Rgba(0, 255, 255, 75)),
		Vertex3D(endPosition, Vector2::ZERO, Rgba(0, 255, 255, 75))
	};

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawLines(lineVertexes, 2, 5.f);
	g_theRenderer->SetDepthTesting(true);
}

bool Game::CheckIfPlayerIsOnGround()
{
	Vector3 centerBottomPoint = m_thePlayer.GetBottomPosition();
	centerBottomPoint.z -= 0.001f;

	BlockInfo centerBlock = m_theWorld->GetBlockInfoFromWorldCoords(centerBottomPoint);
	if (centerBlock.m_chunk && centerBlock.GetBlock()->GetIsSolid())
	{
		return true;
	}

	Vector3 northBottomPoint = centerBottomPoint;
	northBottomPoint.y += m_thePlayer.GetRadius();
	BlockInfo northBlock = m_theWorld->GetBlockInfoFromWorldCoords(northBottomPoint);
	if (northBlock.m_chunk && northBlock.GetBlock()->GetIsSolid())
	{
		return true;
	}

	Vector3 southBottomPoint = centerBottomPoint;
	southBottomPoint.y -= m_thePlayer.GetRadius();
	BlockInfo southBlock = m_theWorld->GetBlockInfoFromWorldCoords(southBottomPoint);
	if (southBlock.m_chunk && southBlock.GetBlock()->GetIsSolid())
	{
		return true;
	}

	Vector3 eastBottomPoint = centerBottomPoint;
	eastBottomPoint.x += m_thePlayer.GetRadius();
	BlockInfo eastBlock = m_theWorld->GetBlockInfoFromWorldCoords(eastBottomPoint);
	if (eastBlock.m_chunk && eastBlock.GetBlock()->GetIsSolid())
	{
		return true;
	}

	Vector3 westBottomPoint = centerBottomPoint;
	westBottomPoint.x -= m_thePlayer.GetRadius();
	BlockInfo westBlock = m_theWorld->GetBlockInfoFromWorldCoords(westBottomPoint);
	if (westBlock.m_chunk && westBlock.GetBlock()->GetIsSolid())
	{
		return true;
	}
	
	BlockInfo bottomCenterBlock = m_theWorld->GetBlockInfoFromWorldCoords(centerBottomPoint);

	//Check against bottomNorth
	BlockInfo bottomNorthEastBlock = bottomCenterBlock.GetNorthBlock().GetEastBlock();
	if (bottomNorthEastBlock.m_chunk && bottomNorthEastBlock.GetBlock()->GetIsSolid())
	{
		Vector3 bottomNorthEastBlockCorner = bottomNorthEastBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomNorthEastBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomNorthEastBlock.m_blockIndex));
		Vector2 bottomNorthEastBlockCornerXY(bottomNorthEastBlockCorner.x, bottomNorthEastBlockCorner.y);

		Vector2 displacement = bottomNorthEastBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			return true;
		}
	}

	BlockInfo bottomNorthWestBlock = bottomCenterBlock.GetNorthBlock().GetWestBlock();
	if (bottomNorthWestBlock.m_chunk && bottomNorthWestBlock.GetBlock()->GetIsSolid())
	{
		Vector3 bottomNorthWestBlockCorner = bottomNorthWestBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomNorthWestBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomNorthWestBlock.m_blockIndex));
		Vector2 bottomNorthWestBlockCornerXY(bottomNorthWestBlockCorner.x + 1.f, bottomNorthWestBlockCorner.y);

		Vector2 displacement = bottomNorthWestBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			return true;
		}
	}

	//Check against bottomNorth
	BlockInfo bottomSouthEastBlock = bottomCenterBlock.GetSouthBlock().GetEastBlock();
	if (bottomSouthEastBlock.m_chunk && bottomSouthEastBlock.GetBlock()->GetIsSolid())
	{
		Vector3 bottomSouthEastBlockCorner = bottomSouthEastBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomSouthEastBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomSouthEastBlock.m_blockIndex));
		Vector2 bottomSouthEastBlockCornerXY(bottomSouthEastBlockCorner.x, bottomSouthEastBlockCorner.y + 1.f);

		Vector2 displacement = bottomSouthEastBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			return true;
		}
	}


	//Check against bottomNorth
	BlockInfo bottomSouthWestBlock = bottomCenterBlock.GetSouthBlock().GetWestBlock();
	if (bottomSouthWestBlock.m_chunk && bottomSouthWestBlock.GetBlock()->GetIsSolid())
	{
		Vector3 bottomSouthWestBlockCorner = bottomSouthWestBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomSouthWestBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomSouthWestBlock.m_blockIndex));
		Vector2 bottomSouthWestBlockCornerXY(bottomSouthWestBlockCorner.x + 1.f, bottomSouthWestBlockCorner.y + 1.f);

		Vector2 displacement = bottomSouthWestBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			return true;
		}
	}

	m_thePlayer.m_footstepSoundDebt = m_thePlayer.m_footstepSoundDebtThreshold;
	return false;
}

void Game::InitializeTrees()
{
	//Oak
	std::vector<TreeBlockDefinition> oakTreeBlocks = 
	{
		TreeBlockDefinition(IntVector3(0, 0, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 3), BLOCK_TYPE_WOOD_LOG),
		
		TreeBlockDefinition(IntVector3(0, 0, 3), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 2, 3), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(0, 0, 4), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 4), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(0, 0, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 5), BLOCK_TYPE_LEAVES)
	};

	TreeDefinition::s_treeDefinitions[TREE_TYPE_OAK] = new TreeDefinition(oakTreeBlocks);

	//Willow
	std::vector<TreeBlockDefinition> willowTreeBlocks =
	{
		TreeBlockDefinition(IntVector3(0, 0, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 3), BLOCK_TYPE_WOOD_LOG),

		TreeBlockDefinition(IntVector3(0, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 3), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(0, 3, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 3, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 3, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 3, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(2, 3, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 3, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 3, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 3, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(3, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(3, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(3, 2, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(3, 2, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(3, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(3, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(3, 0, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(3, 0, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(3, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(3, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(3, -2, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(3, -2, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(2, -3, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -3, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -3, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -3, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(0, -3, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -3, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -3, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -3, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(-2, -3, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -3, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -3, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -3, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(-3, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-3, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-3, -2, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-3, -2, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(-3, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-3, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-3, 0, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-3, 0, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(-3, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-3, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-3, 2, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-3, 2, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(-2, 3, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 3, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 3, 2), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 3, 1), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(0, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 2, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 4), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(0, 0, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 2, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 2, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 2, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 0, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -2, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -2, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -2, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -2, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -2, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 0, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 2, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 2, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 5), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 5), BLOCK_TYPE_LEAVES)
	};

	TreeDefinition::s_treeDefinitions[TREE_TYPE_WILLOW] = new TreeDefinition(willowTreeBlocks);

	//Pine
	std::vector<TreeBlockDefinition> pineTreeBlocks =
	{
		TreeBlockDefinition(IntVector3(0, 0, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 3), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 4), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 5), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 6), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 7), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 8), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 0, 9), BLOCK_TYPE_WOOD_LOG),

		TreeBlockDefinition(IntVector3(0, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 0, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 1, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 2, 3), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 2, 3), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(0, 1, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, 2, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 2, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 2, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 1, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, 0, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -1, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(2, -2, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -2, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -2, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -2, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -2, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, -1, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 0, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 1, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-2, 2, 6), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 2, 6), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(0, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 4), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 4), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(0, 1, 7), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 7), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 7), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 7), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 7), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 7), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 7), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 7), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 7), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(0, 1, 9), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 1, 9), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, 0, 9), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 9), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(0, -1, 9), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, -1, 9), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 0, 9), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(-1, 1, 9), BLOCK_TYPE_LEAVES),
		TreeBlockDefinition(IntVector3(1, -1, 9), BLOCK_TYPE_LEAVES),

		TreeBlockDefinition(IntVector3(0, 0, 10), BLOCK_TYPE_GLOWSTONE)
	};

	TreeDefinition::s_treeDefinitions[TREE_TYPE_PINE] = new TreeDefinition(pineTreeBlocks);

	//Wimba
	std::vector<TreeBlockDefinition> wimbaTreeBlocks =
	{
		TreeBlockDefinition(IntVector3(0, 0, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 0, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, -1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, -1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, -1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 0, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 2, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 2, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 2, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 0, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, -1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, -2, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, -2, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, -2, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, -2, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, -2, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, -1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, 0, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, 1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, 2, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 2, 0), BLOCK_TYPE_WOOD_LOG),

		TreeBlockDefinition(IntVector3(3, 2, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, 1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, 0, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, -1, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, -2, 0), BLOCK_TYPE_WOOD_LOG),

		TreeBlockDefinition(IntVector3(-2, 3, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 3, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 3, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 3, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 3, 0), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, 3, 0), BLOCK_TYPE_WOOD_LOG),

		TreeBlockDefinition(IntVector3(0, 0, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 1, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 1, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 0, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, -1, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, -1, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, -1, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 0, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 1, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 2, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 2, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 2, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 2, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 1, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 0, 1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, -1, 1), BLOCK_TYPE_WOOD_LOG),

		TreeBlockDefinition(IntVector3(0, 0, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 0, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, -1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, -1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, -1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 0, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 0, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, -1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, -2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, -2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, -2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, -2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, -2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, -1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, 0, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, 1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, 2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, 2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, 1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, 0, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, -1, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, -2, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, 3, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 3, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 3, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 3, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 3, -1), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, 3, -1), BLOCK_TYPE_WOOD_LOG),

		TreeBlockDefinition(IntVector3(0, 0, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 0, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, -1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, -1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, -1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 0, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 0, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, -1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, -2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, -2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, -2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, -2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, -2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, -1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, 0, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, 1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, 2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, 2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, 1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, 0, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, -1, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, -2, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-2, 3, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(-1, 3, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(0, 3, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(1, 3, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(2, 3, -2), BLOCK_TYPE_WOOD_LOG),
		TreeBlockDefinition(IntVector3(3, 3, -2), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, -1, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, -1, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(-1, -1, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(-1, 0, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(-1, 1, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(-1, 2, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 2, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 2, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(2, 2, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(2, 1, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(2, 0, 2), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(2, -1, 2), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 3), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 3), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 3), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 3), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 4), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 4), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 4), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 4), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 5), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 5), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 5), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 5), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 6), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 6), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 6), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 6), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 7), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 7), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 7), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 7), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 8), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 8), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 8), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 8), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 9), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 9), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 9), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 9), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 10), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 10), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 10), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 10), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 11), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 11), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 11), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 11), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 12), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 12), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 12), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 12), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, -1, 12), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, -1, 12), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(-1, 0, 12), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(-1, 1, 12), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 2, 12), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 2, 12), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(2, 1, 12), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(2, 0, 12), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 13), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 0, 14), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 1, 14), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 1, 14), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 0, 14), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(-1, 0, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(-1, 1, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(-2, 0, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(-2, 1, 13), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(2, 0, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(2, 1, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(3, 0, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(3, 1, 13), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, 2, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 2, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, 3, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 3, 13), BLOCK_TYPE_WOOD_LOG),

			TreeBlockDefinition(IntVector3(0, -1, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, -1, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(0, -2, 13), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, -2, 13), BLOCK_TYPE_WOOD_LOG),


			TreeBlockDefinition(IntVector3(-2, 0, 14), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(-2, 1, 14), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(-2, 2, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(-2, 3, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(-2, -1, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(-2, -2, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(-3, 0, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(-3, 1, 14), BLOCK_TYPE_LEAVES),

			TreeBlockDefinition(IntVector3(3, 0, 14), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(3, 1, 14), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(3, 2, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(3, 3, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(3, -1, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(3, -2, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(4, 0, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(4, 1, 14), BLOCK_TYPE_LEAVES),

			TreeBlockDefinition(IntVector3(0, 3, 14), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, 3, 14), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(2, 3, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(-1, 3, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(0, 4, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(1, 4, 14), BLOCK_TYPE_LEAVES),

			TreeBlockDefinition(IntVector3(0, -2, 14), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(1, -2, 14), BLOCK_TYPE_WOOD_LOG),
			TreeBlockDefinition(IntVector3(2, -2, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(-1, -2, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(0, -3, 14), BLOCK_TYPE_LEAVES),
			TreeBlockDefinition(IntVector3(1, -3, 14), BLOCK_TYPE_LEAVES),

				TreeBlockDefinition(IntVector3(0, 0, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(0, 1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(1, 1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(1, 0, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(1, -1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(0, -1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-1, -1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-1, 0, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-1, 1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(0, 2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(1, 2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(2, 2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(2, 1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(2, 0, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(2, -1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(2, -2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(1, -2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(0, -2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-1, -2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-2, -2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-2, -1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-2, 0, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-2, 1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-2, 2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-1, 2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(3, 2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(3, 1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(3, 0, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(3, -1, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(3, -2, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-2, 3, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-1, 3, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(0, 3, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(1, 3, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(2, 3, 15), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(3, 3, 15), BLOCK_TYPE_LEAVES),

				TreeBlockDefinition(IntVector3(0, 0, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(0, 1, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(1, 1, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(1, 0, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(1, -1, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(0, -1, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-1, -1, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-1, 0, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-1, 1, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(-1, 2, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(0, 2, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(1, 2, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(2, 2, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(2, 1, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(2, 0, 16), BLOCK_TYPE_LEAVES),
				TreeBlockDefinition(IntVector3(2, -1, 16), BLOCK_TYPE_LEAVES)
				
	};

	TreeDefinition::s_treeDefinitions[TREE_TYPE_WIMBA] = new TreeDefinition(wimbaTreeBlocks);
}

void Game::DrawHookLine() const
{
	std::vector<Vertex3D> hookVerts;

	for (size_t hookIndex = 0; hookIndex < m_thePlayer.m_attachedPoints.size(); hookIndex++)
	{
		hookVerts.push_back(Vertex3D(Vector3(m_thePlayer.m_attachedPoints[hookIndex]), Vector2::ZERO, Rgba(139, 69, 19, 255)));
	}
	hookVerts.push_back(Vertex3D(m_thePlayer.GetCenterPosition(), Vector2::ZERO, Rgba(139, 69, 19, 255)));

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawLineStrip(hookVerts.data(), hookVerts.size(), 10.f);
}

void Game::DetachHook()
{
	m_thePlayer.m_isAttached = false;
	m_thePlayer.m_attachedDist = 0.f;
	m_thePlayer.m_attachedPoints.clear();
	m_thePlayer.m_attachedBlock = BlockInfo();
}

void Game::AddHookPoint(const Vector3& newPoint)
{
	Vector3 displacementToNewPoint = *(m_thePlayer.m_attachedPoints.end() - 1) - newPoint;
	m_thePlayer.m_attachedDist -= displacementToNewPoint.CalcLength();
	m_thePlayer.m_attachedPoints.push_back(newPoint);
}

void Game::RemoveHookPoint()
{
	Vector3 displacementToPointToRemove = *(m_thePlayer.m_attachedPoints.end() - 1) - *(m_thePlayer.m_attachedPoints.end() - 2);
	float lengthToPointToRemove = displacementToPointToRemove.CalcLength();
	m_thePlayer.m_attachedDist += lengthToPointToRemove;

	m_thePlayer.m_attachedPoints.pop_back();
}

void Game::ApplyPhysicsToPlayer()
{
	//If center is in solid, move back on velocity until it isn't in solid
	BlockInfo centerBlock = m_theWorld->GetBlockInfoFromWorldCoords(m_thePlayer.GetCenterPosition());
	while (centerBlock.m_chunk && centerBlock.GetBlock()->GetIsSolid())
	{
		Vector3 newPosition = m_thePlayer.GetCenterPosition();
		newPosition -= m_thePlayer.m_velocity * 0.001f;
		m_thePlayer.SetCenterPosition(newPosition);
		centerBlock = m_theWorld->GetBlockInfoFromWorldCoords(newPosition);
	}

	//Cylinder Extrema
	Vector3 bottomPoint = m_thePlayer.GetBottomPosition();
	BlockInfo bottomBlock = m_theWorld->GetBlockInfoFromWorldCoords(bottomPoint);
	if (bottomBlock.m_chunk && bottomBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = bottomBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomBlock.m_blockIndex));
		bottomPoint.z = blockCoords.z + 1.f;
		Vector3 newCenterPoint = bottomPoint;
		newCenterPoint.z += PLAYER_HEIGHT * 0.5f;
		m_thePlayer.SetCenterPosition(newCenterPoint);
		m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
	}

	Vector3 topPoint = m_thePlayer.GetTopPosition();
	BlockInfo topBlock = m_theWorld->GetBlockInfoFromWorldCoords(topPoint);
	if (topBlock.m_chunk && topBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = topBlock.m_chunk->GetChunkWorldMins() + Vector3(topBlock.m_chunk->GetBlockCoordsForBlockIndex(topBlock.m_blockIndex));
		topPoint.z = blockCoords.z;
		Vector3 newCenterPoint = topPoint;
		newCenterPoint.z -= ((PLAYER_HEIGHT * 0.5f) + 0.0001f);
		m_thePlayer.SetCenterPosition(newCenterPoint);
		m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
	}

	Vector3 bottomNorthPoint = m_thePlayer.GetNorthPosition() - Vector3(0.f, 0.f, PLAYER_HEIGHT * 0.5f);
	BlockInfo bottomNorthBlock = m_theWorld->GetBlockInfoFromWorldCoords(bottomNorthPoint);
	if (bottomNorthBlock.m_chunk && bottomNorthBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = bottomNorthBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomNorthBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomNorthBlock.m_blockIndex));
		if ((blockCoords.z + 1.f) - bottomNorthPoint.z < bottomNorthPoint.y - blockCoords.y)
		{
			bottomNorthPoint.z = blockCoords.z + 1.f;
			Vector3 newCenterPoint = bottomNorthPoint;
			newCenterPoint.y -= m_thePlayer.GetRadius();
			newCenterPoint.z += PLAYER_HEIGHT * 0.5f;
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
		}
		else
		{
			bottomNorthPoint.y = blockCoords.y;
			Vector3 newCenterPoint = bottomNorthPoint;
			newCenterPoint.y -= m_thePlayer.GetRadius();
			newCenterPoint.z += PLAYER_HEIGHT * 0.5f;
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, 0.f, m_thePlayer.m_velocity.z);
		}
		
	}

	Vector3 topNorthPoint = m_thePlayer.GetNorthPosition() + Vector3(0.f, 0.f, PLAYER_HEIGHT * 0.5f);
	BlockInfo topNorthBlock = m_theWorld->GetBlockInfoFromWorldCoords(topNorthPoint);
	if (topNorthBlock.m_chunk && topNorthBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = topNorthBlock.m_chunk->GetChunkWorldMins() + Vector3(topNorthBlock.m_chunk->GetBlockCoordsForBlockIndex(topNorthBlock.m_blockIndex));
		if (topNorthPoint.z - (blockCoords.z) < topNorthPoint.y - blockCoords.y)
		{
			topNorthPoint.z = blockCoords.z;
			Vector3 newCenterPoint = topNorthPoint;
			newCenterPoint.y -= m_thePlayer.GetRadius();
			newCenterPoint.z -= ((PLAYER_HEIGHT * 0.5f) + 0.0001f);
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
		}
		else
		{
			topNorthPoint.y = blockCoords.y;
			Vector3 newCenterPoint = topNorthPoint;
			newCenterPoint.y -= m_thePlayer.GetRadius();
			m_thePlayer.SetCenterPosition(newCenterPoint - Vector3(0.f, 0.f, PLAYER_HEIGHT * 0.5f));
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, 0.f, m_thePlayer.m_velocity.z);
		}
	}

	Vector3 northPoint = m_thePlayer.GetNorthPosition();
	BlockInfo northBlock = m_theWorld->GetBlockInfoFromWorldCoords(northPoint);
	if (northBlock.m_chunk && northBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = northBlock.m_chunk->GetChunkWorldMins() + Vector3(northBlock.m_chunk->GetBlockCoordsForBlockIndex(northBlock.m_blockIndex));
		northPoint.y = blockCoords.y;
		Vector3 newCenterPoint = northPoint;
		newCenterPoint.y -= m_thePlayer.GetRadius();
		m_thePlayer.SetCenterPosition(newCenterPoint);
		m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, 0.f, m_thePlayer.m_velocity.z);
	}

	Vector3 bottomSouthPoint = m_thePlayer.GetSouthPosition() - Vector3(0.f, 0.f, PLAYER_HEIGHT * 0.5f);
	BlockInfo bottomSouthBlock = m_theWorld->GetBlockInfoFromWorldCoords(bottomSouthPoint);
	if (bottomSouthBlock.m_chunk && bottomSouthBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = bottomSouthBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomSouthBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomSouthBlock.m_blockIndex));
		if ((blockCoords.z + 1.f) - bottomSouthPoint.z < (blockCoords.y + 1.f) - bottomSouthPoint.y)
		{
			bottomSouthPoint.z = blockCoords.z + 1.f;
			Vector3 newCenterPoint = bottomSouthPoint;
			newCenterPoint.y += m_thePlayer.GetRadius();
			newCenterPoint.z += PLAYER_HEIGHT * 0.5f;
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
		}
		else
		{
			bottomSouthPoint.y = blockCoords.y + 1.f;
			Vector3 newCenterPoint = bottomSouthPoint;
			newCenterPoint.y += m_thePlayer.GetRadius();
			newCenterPoint.z += PLAYER_HEIGHT * 0.5f;
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, 0.f, m_thePlayer.m_velocity.z);
		}

	}

	Vector3 topSouthPoint = m_thePlayer.GetSouthPosition() + Vector3(0.f, 0.f, PLAYER_HEIGHT * 0.5f);
	BlockInfo topSouthBlock = m_theWorld->GetBlockInfoFromWorldCoords(topSouthPoint);
	if (topSouthBlock.m_chunk && topSouthBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = topSouthBlock.m_chunk->GetChunkWorldMins() + Vector3(topSouthBlock.m_chunk->GetBlockCoordsForBlockIndex(topSouthBlock.m_blockIndex));
		if (topSouthPoint.z - (blockCoords.z) < (blockCoords.y + 1.f) - topSouthPoint.y)
		{
			topSouthPoint.z = blockCoords.z;
			Vector3 newCenterPoint = topSouthPoint;
			newCenterPoint.y += m_thePlayer.GetRadius();
			newCenterPoint.z -= ((PLAYER_HEIGHT * 0.5f) + 0.0001f);
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
		}
		else
		{
			topSouthPoint.y = blockCoords.y + 1.f;
			Vector3 newCenterPoint = topSouthPoint;
			newCenterPoint.y += m_thePlayer.GetRadius();
			m_thePlayer.SetCenterPosition(newCenterPoint - Vector3(0.f, 0.f, PLAYER_HEIGHT * 0.5f));
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, 0.f, m_thePlayer.m_velocity.z);
		}
	}

	Vector3 southPoint = m_thePlayer.GetSouthPosition();
	BlockInfo southBlock = m_theWorld->GetBlockInfoFromWorldCoords(southPoint);
	if (southBlock.m_chunk && southBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = southBlock.m_chunk->GetChunkWorldMins() + Vector3(southBlock.m_chunk->GetBlockCoordsForBlockIndex(southBlock.m_blockIndex));
		southPoint.y = blockCoords.y + 1.f;
		Vector3 newCenterPoint = southPoint;
		newCenterPoint.y += m_thePlayer.GetRadius();
		m_thePlayer.SetCenterPosition(newCenterPoint);
		m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, 0.f, m_thePlayer.m_velocity.z);
	}
	

	Vector3 bottomEastPoint = m_thePlayer.GetEastPosition() - Vector3(0.f, 0.f, PLAYER_HEIGHT * 0.5f);
	BlockInfo bottomEastBlock = m_theWorld->GetBlockInfoFromWorldCoords(bottomEastPoint);
	if (bottomEastBlock.m_chunk && bottomEastBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = bottomEastBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomEastBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomEastBlock.m_blockIndex));
		if ((blockCoords.z + 1.f) - bottomEastPoint.z < bottomEastPoint.x - blockCoords.x)
		{
			bottomEastPoint.z = blockCoords.z + 1.f;
			Vector3 newCenterPoint = bottomEastPoint;
			newCenterPoint.x -= m_thePlayer.GetRadius();
			newCenterPoint.z += PLAYER_HEIGHT * 0.5f;
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
		}
		else
		{
			bottomEastPoint.x = blockCoords.x;
			Vector3 newCenterPoint = bottomEastPoint;
			newCenterPoint.x -= m_thePlayer.GetRadius();
			newCenterPoint.z += PLAYER_HEIGHT * 0.5f;
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(0.f, m_thePlayer.m_velocity.y, m_thePlayer.m_velocity.z);
		}
	}

	Vector3 topEastPoint = m_thePlayer.GetEastPosition() + Vector3(0.f, 0.f, PLAYER_HEIGHT * 0.5f);
	BlockInfo topEastBlock = m_theWorld->GetBlockInfoFromWorldCoords(topEastPoint);
	if (topEastBlock.m_chunk && topEastBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = topEastBlock.m_chunk->GetChunkWorldMins() + Vector3(topEastBlock.m_chunk->GetBlockCoordsForBlockIndex(topEastBlock.m_blockIndex));
		if (topEastPoint.z - (blockCoords.z) < topEastPoint.x - blockCoords.x)
		{
			topEastPoint.z = blockCoords.z;
			Vector3 newCenterPoint = topEastPoint;
			newCenterPoint.x -= m_thePlayer.GetRadius();
			newCenterPoint.z -= ((PLAYER_HEIGHT * 0.5f) + 0.0001f);
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
		}
		else
		{
			topEastPoint.x = blockCoords.x;
			Vector3 newCenterPoint = topEastPoint;
			newCenterPoint.x -= m_thePlayer.GetRadius();
			newCenterPoint.z -= ((PLAYER_HEIGHT * 0.5f));
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(0.f, m_thePlayer.m_velocity.y, m_thePlayer.m_velocity.z);
		}
	}

	Vector3 eastPoint = m_thePlayer.GetEastPosition();
	BlockInfo eastBlock = m_theWorld->GetBlockInfoFromWorldCoords(eastPoint);
	if (eastBlock.m_chunk && eastBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = eastBlock.m_chunk->GetChunkWorldMins() + Vector3(eastBlock.m_chunk->GetBlockCoordsForBlockIndex(eastBlock.m_blockIndex));
		eastPoint.x = blockCoords.x;
		Vector3 newCenterPoint = eastPoint;
		newCenterPoint.x -= m_thePlayer.GetRadius();
		m_thePlayer.SetCenterPosition(newCenterPoint);
		m_thePlayer.m_velocity = Vector3(0.f, m_thePlayer.m_velocity.y, m_thePlayer.m_velocity.z);
	}
	

	Vector3 bottomWestPoint = m_thePlayer.GetWestPosition() - Vector3(0.f, 0.f, PLAYER_HEIGHT * 0.5f);
	BlockInfo bottomWestBlock = m_theWorld->GetBlockInfoFromWorldCoords(bottomWestPoint);
	if (bottomWestBlock.m_chunk && bottomWestBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = bottomWestBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomWestBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomWestBlock.m_blockIndex));
		if ((blockCoords.z + 1.f) - bottomWestPoint.z < (blockCoords.x + 1.f) - bottomWestPoint.x)
		{
			bottomWestPoint.z = blockCoords.z + 1.f;
			Vector3 newCenterPoint = bottomWestPoint;
			newCenterPoint.x += m_thePlayer.GetRadius();
			newCenterPoint.z += PLAYER_HEIGHT * 0.5f;
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
		}
		else
		{
			bottomWestPoint.x = blockCoords.x + 1.f;
			Vector3 newCenterPoint = bottomWestPoint;
			newCenterPoint.x += m_thePlayer.GetRadius();
			newCenterPoint.z += PLAYER_HEIGHT * 0.5f;
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(0.f, m_thePlayer.m_velocity.y, m_thePlayer.m_velocity.z);
		}
	}

	Vector3 topWestPoint = m_thePlayer.GetWestPosition() + Vector3(0.f, 0.f, PLAYER_HEIGHT * 0.5f);
	BlockInfo topWestBlock = m_theWorld->GetBlockInfoFromWorldCoords(topWestPoint);
	if (topWestBlock.m_chunk && topWestBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = topWestBlock.m_chunk->GetChunkWorldMins() + Vector3(topWestBlock.m_chunk->GetBlockCoordsForBlockIndex(topWestBlock.m_blockIndex));
		if (topWestPoint.z - (blockCoords.z) < (blockCoords.x + 1.f) - topWestPoint.x)
		{
			topWestPoint.z = blockCoords.z;
			Vector3 newCenterPoint = topWestPoint;
			newCenterPoint.x += m_thePlayer.GetRadius();
			newCenterPoint.z -= ((PLAYER_HEIGHT * 0.5f) + 0.0001f);
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
		}
		else
		{
			topWestPoint.x = blockCoords.x + 1.f;
			Vector3 newCenterPoint = topWestPoint;
			newCenterPoint.x += m_thePlayer.GetRadius();
			newCenterPoint.z -= ((PLAYER_HEIGHT * 0.5f));
			m_thePlayer.SetCenterPosition(newCenterPoint);
			m_thePlayer.m_velocity = Vector3(0.f, m_thePlayer.m_velocity.y, m_thePlayer.m_velocity.z);
		}
	}


	Vector3 westPoint = m_thePlayer.GetWestPosition();
	BlockInfo westBlock = m_theWorld->GetBlockInfoFromWorldCoords(westPoint);
	if (westBlock.m_chunk && westBlock.GetBlock()->GetIsSolid())
	{
		Vector3 blockCoords = westBlock.m_chunk->GetChunkWorldMins() + Vector3(westBlock.m_chunk->GetBlockCoordsForBlockIndex(westBlock.m_blockIndex));
		westPoint.x = blockCoords.x + 1.f;
		Vector3 newCenterPoint = westPoint;
		newCenterPoint.x += m_thePlayer.GetRadius();
		m_thePlayer.SetCenterPosition(newCenterPoint);
		m_thePlayer.m_velocity = Vector3(0.f, m_thePlayer.m_velocity.y, m_thePlayer.m_velocity.z);
	}

	//Check neighbors for overlap with non-extrema horizontally
	BlockInfo bottomCenterBlock = m_theWorld->GetBlockInfoFromWorldCoords(m_thePlayer.GetBottomPosition());
	BlockInfo topCenterBlock = m_theWorld->GetBlockInfoFromWorldCoords(m_thePlayer.GetTopPosition());

	//Check against bottomNorthEast
	BlockInfo bottomNorthEastBlock = bottomCenterBlock.GetNorthBlock().GetEastBlock();
	if (bottomNorthEastBlock.m_chunk && bottomNorthEastBlock.GetBlock()->GetIsSolid())
	{
		Vector3 bottomNorthEastBlockCorner = bottomNorthEastBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomNorthEastBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomNorthEastBlock.m_blockIndex));
		Vector2 bottomNorthEastBlockCornerXY(bottomNorthEastBlockCorner.x, bottomNorthEastBlockCorner.y);

		Vector2 displacement = bottomNorthEastBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			float zDisplacement = (bottomNorthEastBlockCorner.z + 1.f) - m_thePlayer.GetBottomPosition().z;
			if (zDisplacement < displacement.CalcLength())
			{
				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint.z += zDisplacement;
				m_thePlayer.SetCenterPosition(newCenterPoint);
				m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
			}
			else
			{
				Vector2 correctionVector = displacement;
				correctionVector.SetLength(m_thePlayer.GetRadius() - displacement.CalcLength());

				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint -= Vector3(correctionVector.x, correctionVector.y, 0.f);

				m_thePlayer.SetCenterPosition(newCenterPoint);
				// 			m_thePlayer.m_velocity = CancelVelocityAlongNormal(displacement.GetNormalized());
			}
		}
	}

	//Check against bottomNorthWest
	BlockInfo bottomNorthWestBlock = bottomCenterBlock.GetNorthBlock().GetWestBlock();
	if (bottomNorthWestBlock.m_chunk && bottomNorthWestBlock.GetBlock()->GetIsSolid())
	{
		Vector3 bottomNorthWestBlockCorner = bottomNorthWestBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomNorthWestBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomNorthWestBlock.m_blockIndex));
		Vector2 bottomNorthWestBlockCornerXY(bottomNorthWestBlockCorner.x + 1.f, bottomNorthWestBlockCorner.y);

		Vector2 displacement = bottomNorthWestBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			float zDisplacement = (bottomNorthWestBlockCorner.z + 1.f) - m_thePlayer.GetBottomPosition().z;
			if (zDisplacement < displacement.CalcLength())
			{
				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint.z += zDisplacement;
				m_thePlayer.SetCenterPosition(newCenterPoint);
				m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
			}
			else
			{
				Vector2 correctionVector = displacement;
				correctionVector.SetLength(m_thePlayer.GetRadius() - displacement.CalcLength());

				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint -= Vector3(correctionVector.x, correctionVector.y, 0.f);

				m_thePlayer.SetCenterPosition(newCenterPoint);
				// 			m_thePlayer.m_velocity = CancelVelocityAlongNormal(displacement.GetNormalized());
			}
		}
	}

	//Check against bottomSouthEast
	BlockInfo bottomSouthEastBlock = bottomCenterBlock.GetSouthBlock().GetEastBlock();
	if (bottomSouthEastBlock.m_chunk && bottomSouthEastBlock.GetBlock()->GetIsSolid())
	{
		Vector3 bottomSouthEastBlockCorner = bottomSouthEastBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomSouthEastBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomSouthEastBlock.m_blockIndex));
		Vector2 bottomSouthEastBlockCornerXY(bottomSouthEastBlockCorner.x, bottomSouthEastBlockCorner.y + 1.f);

		Vector2 displacement = bottomSouthEastBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			float zDisplacement = (bottomSouthEastBlockCorner.z + 1.f) - m_thePlayer.GetBottomPosition().z;
			if (zDisplacement < displacement.CalcLength())
			{
				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint.z += zDisplacement;
				m_thePlayer.SetCenterPosition(newCenterPoint);
				m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
			}
			else
			{
				Vector2 correctionVector = displacement;
				correctionVector.SetLength(m_thePlayer.GetRadius() - displacement.CalcLength());

				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint -= Vector3(correctionVector.x, correctionVector.y, 0.f);

				m_thePlayer.SetCenterPosition(newCenterPoint);
				// 			m_thePlayer.m_velocity = CancelVelocityAlongNormal(displacement.GetNormalized());
			}
		}
	}


	//Check against bottomSouthWest
	BlockInfo bottomSouthWestBlock = bottomCenterBlock.GetSouthBlock().GetWestBlock();
	if (bottomSouthWestBlock.m_chunk && bottomSouthWestBlock.GetBlock()->GetIsSolid())
	{
		Vector3 bottomSouthWestBlockCorner = bottomSouthWestBlock.m_chunk->GetChunkWorldMins() + Vector3(bottomSouthWestBlock.m_chunk->GetBlockCoordsForBlockIndex(bottomSouthWestBlock.m_blockIndex));
		Vector2 bottomSouthWestBlockCornerXY(bottomSouthWestBlockCorner.x + 1.f, bottomSouthWestBlockCorner.y + 1.f);

		Vector2 displacement = bottomSouthWestBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			float zDisplacement = (bottomSouthWestBlockCorner.z + 1.f) - m_thePlayer.GetBottomPosition().z;
			if (zDisplacement < displacement.CalcLength())
			{
				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint.z += zDisplacement;
				m_thePlayer.SetCenterPosition(newCenterPoint);
				m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
			}
			else
			{
				Vector2 correctionVector = displacement;
				correctionVector.SetLength(m_thePlayer.GetRadius() - displacement.CalcLength());

				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint -= Vector3(correctionVector.x, correctionVector.y, 0.f);

				m_thePlayer.SetCenterPosition(newCenterPoint);
				// 			m_thePlayer.m_velocity = CancelVelocityAlongNormal(displacement.GetNormalized());
			}
		}
	}

	//TOP
	//Check against topNorthEast
	BlockInfo topNorthEastBlock = topCenterBlock.GetNorthBlock().GetEastBlock();
	if (topNorthEastBlock.m_chunk && topNorthEastBlock.GetBlock()->GetIsSolid())
	{
		Vector3 topNorthEastBlockCorner = topNorthEastBlock.m_chunk->GetChunkWorldMins() + Vector3(topNorthEastBlock.m_chunk->GetBlockCoordsForBlockIndex(topNorthEastBlock.m_blockIndex));
		Vector2 topNorthEastBlockCornerXY(topNorthEastBlockCorner.x, topNorthEastBlockCorner.y);

		Vector2 displacement = topNorthEastBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			float zDisplacement = m_thePlayer.GetTopPosition().z - (topNorthEastBlockCorner.z);
			if (zDisplacement < displacement.CalcLength())
			{
				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint.z -= zDisplacement;
				m_thePlayer.SetCenterPosition(newCenterPoint);
				m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
			}
			else
			{
				Vector2 correctionVector = displacement;
				correctionVector.SetLength(m_thePlayer.GetRadius() - displacement.CalcLength());

				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint -= Vector3(correctionVector.x, correctionVector.y, 0.f);

				m_thePlayer.SetCenterPosition(newCenterPoint);
				// 			m_thePlayer.m_velocity = CancelVelocityAlongNormal(displacement.GetNormalized());
			}
		}
	}

	//Check against topNorthWest
	BlockInfo topNorthWestBlock = topCenterBlock.GetNorthBlock().GetWestBlock();
	if (topNorthWestBlock.m_chunk && topNorthWestBlock.GetBlock()->GetIsSolid())
	{
		Vector3 topNorthWestBlockCorner = topNorthWestBlock.m_chunk->GetChunkWorldMins() + Vector3(topNorthWestBlock.m_chunk->GetBlockCoordsForBlockIndex(topNorthWestBlock.m_blockIndex));
		Vector2 topNorthWestBlockCornerXY(topNorthWestBlockCorner.x + 1.f, topNorthWestBlockCorner.y);

		Vector2 displacement = topNorthWestBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			float zDisplacement = m_thePlayer.GetTopPosition().z - (topNorthWestBlockCorner.z);
			if (zDisplacement < displacement.CalcLength())
			{
				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint.z -= zDisplacement;
				m_thePlayer.SetCenterPosition(newCenterPoint);
				m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
			}
			else
			{
				Vector2 correctionVector = displacement;
				correctionVector.SetLength(m_thePlayer.GetRadius() - displacement.CalcLength());

				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint -= Vector3(correctionVector.x, correctionVector.y, 0.f);

				m_thePlayer.SetCenterPosition(newCenterPoint);
				// 			m_thePlayer.m_velocity = CancelVelocityAlongNormal(displacement.GetNormalized());
			}
		}
	}

	//Check against topSouthEast
	BlockInfo topSouthEastBlock = topCenterBlock.GetSouthBlock().GetEastBlock();
	if (topSouthEastBlock.m_chunk && topSouthEastBlock.GetBlock()->GetIsSolid())
	{
		Vector3 topSouthEastBlockCorner = topSouthEastBlock.m_chunk->GetChunkWorldMins() + Vector3(topSouthEastBlock.m_chunk->GetBlockCoordsForBlockIndex(topSouthEastBlock.m_blockIndex));
		Vector2 topSouthEastBlockCornerXY(topSouthEastBlockCorner.x, topSouthEastBlockCorner.y + 1.f);

		Vector2 displacement = topSouthEastBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			float zDisplacement = m_thePlayer.GetTopPosition().z - (topSouthEastBlockCorner.z);
			if (zDisplacement < displacement.CalcLength())
			{
				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint.z -= zDisplacement;
				m_thePlayer.SetCenterPosition(newCenterPoint);
				m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
			}
			else
			{
				Vector2 correctionVector = displacement;
				correctionVector.SetLength(m_thePlayer.GetRadius() - displacement.CalcLength());

				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint -= Vector3(correctionVector.x, correctionVector.y, 0.f);

				m_thePlayer.SetCenterPosition(newCenterPoint);
				// 			m_thePlayer.m_velocity = CancelVelocityAlongNormal(displacement.GetNormalized());
			}
		}
	}


	//Check against topSouthWest
	BlockInfo topSouthWestBlock = topCenterBlock.GetSouthBlock().GetWestBlock();
	if (topSouthWestBlock.m_chunk && topSouthWestBlock.GetBlock()->GetIsSolid())
	{
		Vector3 topSouthWestBlockCorner = topSouthWestBlock.m_chunk->GetChunkWorldMins() + Vector3(topSouthWestBlock.m_chunk->GetBlockCoordsForBlockIndex(topSouthWestBlock.m_blockIndex));
		Vector2 topSouthWestBlockCornerXY(topSouthWestBlockCorner.x + 1.f, topSouthWestBlockCorner.y + 1.f);

		Vector2 displacement = topSouthWestBlockCornerXY - Vector2(m_thePlayer.GetCenterPosition().x, m_thePlayer.GetCenterPosition().y);
		if (displacement.CalcLengthSquared() < (m_thePlayer.GetRadius() * m_thePlayer.GetRadius()))
		{
			float zDisplacement = m_thePlayer.GetTopPosition().z - (topSouthWestBlockCorner.z);
			if (zDisplacement < displacement.CalcLength())
			{
				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint.z -= zDisplacement;
				m_thePlayer.SetCenterPosition(newCenterPoint);
				m_thePlayer.m_velocity = Vector3(m_thePlayer.m_velocity.x, m_thePlayer.m_velocity.y, 0.f);
			}
			else
			{
				Vector2 correctionVector = displacement;
				correctionVector.SetLength(m_thePlayer.GetRadius() - displacement.CalcLength());

				Vector3 newCenterPoint = m_thePlayer.GetCenterPosition();
				newCenterPoint -= Vector3(correctionVector.x, correctionVector.y, 0.f);

				m_thePlayer.SetCenterPosition(newCenterPoint);
				// 			m_thePlayer.m_velocity = CancelVelocityAlongNormal(displacement.GetNormalized());
			}
		}
	}

}

void Game::UpdateCamera(float deltaSeconds)
{
	switch (m_theCamera.m_mode)
	{
	case CAMERA_FIRST_PERSON:
	{
		m_theCamera.m_position = m_thePlayer.GetEyePosition();
		m_theCamera.m_yaw = m_thePlayer.m_yaw;
		m_theCamera.m_pitch = m_thePlayer.m_pitch;
		break;
	}
	case CAMERA_FROM_BEHIND:
	{
		Vector3 cameraStartPosition = m_thePlayer.GetEyePosition();
		m_theCamera.m_yaw = m_thePlayer.m_yaw;
		m_theCamera.m_pitch = m_thePlayer.m_pitch;

		RaycastResult thirdPersonPositionRaycast = Raycast(cameraStartPosition, Vector3::ZERO - m_theCamera.GetForwardXYZ(), CAMERA_THIRD_PERSON_DISTANCE);
		m_theCamera.m_position = thirdPersonPositionRaycast.m_pointBeforeImpact;
		break;
	}
	case CAMERA_FIXED_ANGLE:
	{
		Vector3 cameraStartPosition = m_thePlayer.GetEyePosition();
		m_theCamera.m_yaw = CAMERA_FIXED_ANGLE_YAW;
		m_theCamera.m_pitch = CAMERA_FIXED_ANGLE_PITCH;

		RaycastResult thirdPersonPositionRaycast = Raycast(cameraStartPosition, Vector3::ZERO - m_theCamera.GetForwardXYZ(), CAMERA_THIRD_PERSON_DISTANCE);
		m_theCamera.m_position = thirdPersonPositionRaycast.m_pointBeforeImpact;
		break;
	}
	case CAMERA_NO_CLIP:
	{
		if (g_theApp->HasFocus())
		{
			IntVector2 mouseScreenPos = g_theInput->GetCursorScreenPos();
			IntVector2 screenCenter = g_theInput->GetScreenCenter();

			IntVector2 mouseDeltaMove = mouseScreenPos - screenCenter;

			g_theInput->SetCursorScreenPos(screenCenter);

			float mouseMovedX = (float)mouseDeltaMove.x;
			float mouseMovedY = (float)mouseDeltaMove.y;
			float mouseSensitivityY = 0.03f;
			float mouseSensitivityX = 0.03f;
			m_theCamera.m_yaw -= mouseMovedX * mouseSensitivityX;
			m_theCamera.m_pitch += mouseMovedY * mouseSensitivityY;

			m_theCamera.m_pitch = ClampFloat(m_theCamera.m_pitch, -89.99f, 89.99f);
		}
		float playerSpeed = PLAYER_SPEED;

		if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
		{
			playerSpeed *= 8.f;
		}

		//Forward Back
		if (g_theInput->IsKeyDown('W'))
		{
			m_theCamera.m_position += m_theCamera.GetForwardXY() * playerSpeed * deltaSeconds;
		}
		else if (g_theInput->IsKeyDown('S'))
		{
			m_theCamera.m_position -= m_theCamera.GetForwardXY() * playerSpeed * deltaSeconds;
		}

		//Left Right
		if (g_theInput->IsKeyDown('A'))
		{
			m_theCamera.m_position += m_theCamera.GetLeftXY() * playerSpeed * deltaSeconds;
		}
		else if (g_theInput->IsKeyDown('D'))
		{
			m_theCamera.m_position -= m_theCamera.GetLeftXY() * playerSpeed * deltaSeconds;
		}

		//Up Down
		if (g_theInput->IsKeyDown(KEYCODE_SPACE))
		{
			m_theCamera.m_position += Vector3::Z_AXIS * playerSpeed * deltaSeconds;
		}
		else if (g_theInput->IsKeyDown(KEYCODE_CTRL))
		{
			m_theCamera.m_position -= Vector3::Z_AXIS * playerSpeed * deltaSeconds;
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
		{
			float playerEyePosition = m_theCamera.m_position.z - (PLAYER_EYE_HEIGHT - (PLAYER_HEIGHT * 0.5f));
			m_thePlayer.SetCenterPosition(Vector3(m_theCamera.m_position.x, m_theCamera.m_position.y, playerEyePosition));
		}

	}
	break;
	}
}

void Game::SetUpCamera() const
{
	//Setting up view
	g_theRenderer->ClearScreen(Rgba(30, 30, 30));
	g_theRenderer->ClearDepth();

	float fovVerticalDegrees = 60.f;
	float aspectRatio = 16.f / 9.f;
	float zNearDistance = 0.01f;
	float zFarDistance = 1000.f;
	g_theRenderer->SetPerspectiveProjection(fovVerticalDegrees, aspectRatio, zNearDistance, zFarDistance);

	//Setting Axes
	g_theRenderer->RotateCoordinates3D(-90.f, Vector3::X_AXIS); //+Z up
	g_theRenderer->RotateCoordinates3D(90.f, Vector3::Z_AXIS); //+X forward

	ApplyCameraTransform(m_theCamera);

	g_theRenderer->SetBackFaceCulling(true);
	g_theRenderer->SetDepthTesting(true);
}

void Game::DrawWorld() const
{
	//Draw world
	g_theRenderer->BindTexture(g_blockSprites->GetTexture());

	m_theWorld->Render(m_theCamera.m_position, m_theCamera.GetForwardXYZ());
}

void Game::DrawWorldAxes() const
{
	g_theRenderer->DrawAxes(100.f, 3.f, 1.f);

	g_theRenderer->SetDepthTesting(false);
	g_theRenderer->DrawAxes(100.f, 1.f, 0.3f);
}
