#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Cylinder3D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/BlockInfo.hpp"

enum MovementMode
{
	PLAYER_WALKING,
	PLAYER_FLYING,
	NUM_PLAYER_MODES
};

class Player
{
public:
	Vector3 m_velocity;
	float m_yaw;
	float m_pitch;

	MovementMode m_mode;
	bool m_isGrounded;

	float m_footstepSoundDebt;
	float m_footstepSoundDebtThreshold;

	bool m_isAttached;
	std::vector<Vector3> m_attachedPoints;
	float m_attachedDist;
	BlockInfo m_attachedBlock;

	Player();
	Player(const Vector3& position, float yaw, float pitch);

	Vector3 GetCenterPosition() const;
	void SetCenterPosition(const Vector3& newPosition);
	Vector3 GetEyePosition() const;
	void SetEyePosition(const Vector3& newPosition);

	float GetRadius() const;

	Vector3 GetBottomPosition() const;
	Vector3 GetTopPosition() const;
	Vector3 GetNorthPosition() const;
	Vector3 GetSouthPosition() const;
	Vector3 GetEastPosition() const;
	Vector3 GetWestPosition() const;

	Vector3 GetForwardXYZ() const;
	Vector3 GetForwardXY() const;
	Vector3 GetLeftXY() const;

	float GetRandomFootstepSoundThreshold() const;

	void Update(float deltaSeconds);
	void Render() const;
private:
	Cylinder3D m_physicsCylinder;

	void GeneratePlayerVertexes(std::vector<Vertex3D>& playerVerts) const;
};




inline Vector3 Player::GetCenterPosition() const
{
	return m_physicsCylinder.m_startPosition + Vector3(0.f, 0.f, (PLAYER_HEIGHT * 0.5f));
}

inline void Player::SetCenterPosition(const Vector3& newPosition)
{
	m_physicsCylinder.m_startPosition = newPosition - Vector3(0.f, 0.f, (PLAYER_HEIGHT * 0.5f));
	m_physicsCylinder.m_endPosition = m_physicsCylinder.m_startPosition + Vector3(0.f, 0.f, PLAYER_HEIGHT);
}

inline void Player::SetEyePosition(const Vector3& newPosition)
{
	m_physicsCylinder.m_startPosition = newPosition - Vector3(0.f, 0.f, PLAYER_EYE_HEIGHT);
	m_physicsCylinder.m_endPosition = m_physicsCylinder.m_startPosition + Vector3(0.f, 0.f, PLAYER_HEIGHT);
}

inline Vector3 Player::GetEyePosition() const
{
	return m_physicsCylinder.m_startPosition + Vector3(0.f, 0.f, PLAYER_EYE_HEIGHT);
}

inline float Player::GetRadius() const
{
	return m_physicsCylinder.m_radius;
}


inline Vector3 Player::GetBottomPosition() const
{
	return m_physicsCylinder.m_startPosition;
}

inline Vector3 Player::GetTopPosition() const
{
	return m_physicsCylinder.m_endPosition;
}

inline Vector3 Player::GetNorthPosition() const
{
	return GetCenterPosition() + Vector3(0.f, m_physicsCylinder.m_radius, 0.f);
}

inline Vector3 Player::GetSouthPosition() const
{
	return GetCenterPosition() - Vector3(0.f, m_physicsCylinder.m_radius, 0.f);
}
inline Vector3 Player::GetEastPosition() const
{
	return GetCenterPosition() + Vector3(m_physicsCylinder.m_radius, 0.f, 0.f);
}

inline Vector3 Player::GetWestPosition() const
{
	return GetCenterPosition() - Vector3(m_physicsCylinder.m_radius, 0.f, 0.f);
}
