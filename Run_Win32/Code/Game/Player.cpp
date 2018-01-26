#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"

Player::Player()
	: m_physicsCylinder(Vector3::ZERO, Vector3(0.f, 0.f, PLAYER_HEIGHT), PLAYER_RADIUS)
	, m_velocity(Vector3::ZERO)
	, m_yaw(0.f)
	, m_pitch(0.f)
	, m_mode(PLAYER_WALKING)
	, m_isGrounded(false)
	, m_footstepSoundDebt(0.f)
	, m_isAttached(false)
	, m_attachedDist(0.f)
	, m_attachedPoints()
	, m_attachedBlock()
{
	m_footstepSoundDebtThreshold = GetRandomFootstepSoundThreshold();
}

Player::Player(const Vector3& position, float yaw, float pitch)
	: m_physicsCylinder(position, Vector3(position.x, position.y, position.z + PLAYER_HEIGHT), PLAYER_RADIUS)
	, m_velocity(Vector3::ZERO)
	, m_yaw(yaw)
	, m_pitch(pitch)
	, m_mode(PLAYER_WALKING)
	, m_isGrounded(false)
	, m_footstepSoundDebt(0.f)
	, m_isAttached(false)
	, m_attachedDist(0.f)
	, m_attachedPoints()
	, m_attachedBlock()
{
	m_footstepSoundDebtThreshold = GetRandomFootstepSoundThreshold();
}

void Player::Update(float deltaSeconds)
{
	SetCenterPosition(GetCenterPosition() + (m_velocity * deltaSeconds));
}

void Player::Render() const
{
	std::vector<Vertex3D> playerVerts;
	GeneratePlayerVertexes(playerVerts);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawLines(playerVerts.data(), playerVerts.size(), 2.f);
}

void Player::GeneratePlayerVertexes(std::vector<Vertex3D>& playerVerts) const
{
	int numSamplePoints = 12;
	float theta = 360.f / numSamplePoints;

	for(int pointIndex = 0; pointIndex <= numSamplePoints; pointIndex++)
	{
		float currentAngle = pointIndex * theta;
		float x = (m_physicsCylinder.m_radius*CosDegrees(currentAngle)) + m_physicsCylinder.m_startPosition.x;
		float y = (m_physicsCylinder.m_radius*SinDegrees(currentAngle)) + m_physicsCylinder.m_startPosition.y;

		playerVerts.push_back(Vertex3D(Vector3(x, y, m_physicsCylinder.m_startPosition.z), Vector2::ZERO, Rgba(255, 0, 255, 255)));
		playerVerts.push_back(Vertex3D(Vector3(x, y, m_physicsCylinder.m_endPosition.z), Vector2::ZERO, Rgba(255, 0, 255, 255)));
	}

}

float Player::GetRandomFootstepSoundThreshold() const
{
	float variance = PLAYER_FOOTSTEP_SOUND_THRESHOLD_VARIANCE * GetRandomFloatInRange(-1.f, 1.f);
	return PLAYER_FOOTSTEP_SOUND_THRESHOLD + variance;
}

Vector3 Player::GetForwardXYZ() const
{
	return Vector3(CosDegrees(m_yaw) * CosDegrees(-m_pitch), SinDegrees(m_yaw) * CosDegrees(-m_pitch), SinDegrees(-m_pitch));
}

Vector3 Player::GetForwardXY() const
{
	return Vector3(CosDegrees(m_yaw), SinDegrees(m_yaw), 0.f);
}

Vector3 Player::GetLeftXY() const
{
	Vector3 forward = GetForwardXY();
	return Vector3(-forward.y, forward.x, 0.f);
}
