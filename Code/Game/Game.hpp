#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Game/Camera3D.hpp"
#include "Game/Block.hpp"
#include "Game/Chunk.hpp"
#include "Game/World.hpp"
#include "Game/Player.hpp"

struct RaycastResult
{
	bool m_didImpact;
	float m_impactFraction;
	Vector3 m_pointBeforeImpact;
	Vector3 m_impactPosition;
	BlockInfo m_impactedBlock;
	BlockInfo m_blockBeforeImpact;
	Vector3 m_impactNormal;
};

class Game
{
private:
	Camera3D m_theCamera;
	Player m_thePlayer;

	int m_selectedBlockIndex;
	std::vector<BlockType> m_inventoryBlocks;

	SoundID m_selectionSoundID;

	bool isGamePaused;
	bool isScreenShaking;
	float screenShakeLifetime;
	float screenShakeMagnitude;

	void ScreenShake() const;
	void SetScreenShake(float shakeLifetime, float shakeMagnitude);
	void SetScreenShakeMagnitude(float shakeMagnitude);
	void SetScreenShakeLifetime(float shakeLifetime);
	void DecrementShakeTime(float deltaSeconds);

	Vector2 GetRandomOffscreenPosition(float entityRadius) const;

	void UpdatePlayer(float deltaSeconds);
	void UpdatePlayerFromController(float deltaSeconds);
	void UpdatePlayerMovementFromKeyboard(float deltaSeconds);
	void UpdatePlayerOrientationFromKeyboard(float deltaSeconds);
	void UpdatePlayerViewFromMouse();
	void UpdatePlayerActionsFromMouse();
	void ApplyPhysicsToPlayer();

	void UpdateCamera(float deltaSeconds);
	void SetUpCamera() const;
	void ApplyCameraTransform(const Camera3D& m_theCamera) const;
	
	void DrawWorld() const;
	void DrawWorldAxes() const;
	void DrawDebugText() const;
	void DrawCrosshair() const;
	void DrawBlockSelectionHUD() const;

	RaycastResult Raycast(const Vector3& startPosition, const Vector3& direction, float maxDistance) const;

	void HighlightTargettedBlock(const RaycastResult& rayResult) const;
	void DestroyTargettedBlock(const RaycastResult& rayResult);
	void PlaceBlock(BlockType typeOfBlock, const RaycastResult& rayResult);

	void AttachHook(const Vector3& startPosition, const Vector3& direction, float maxDistance);
	void DetachHook();
	void AddHookPoint(const Vector3& newPoint);
	void RemoveHookPoint();
	void CorrectForHook();
	void DrawHookLine() const;

	void DeveloperInput();
	void SavePlayerState();
	void LoadPlayerState(const std::vector<unsigned char>& inputBuffer);
	void ApplyGlobalForces(float deltaSeconds);
	void DrawTargettingLine(const Vector3& startPosition, const Vector3& endPosition) const;
	bool CheckIfPlayerIsOnGround();
	void InitializeTrees();
public:
	World* m_theWorld;

	Game();
	~Game();

	void Update(float deltaSeconds);
	void Render() const;
private:
	void LoadSounds();
	void PlayFootstepSound();
	void InitializeBlockTypes();
};