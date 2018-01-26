#include "Game/Camera3D.hpp"
#include "Engine/Math/MathUtils.hpp"

Camera3D::Camera3D()
	: m_position(Vector3::ZERO)
	, m_yaw(0.f)
	, m_pitch(0.f)
	, m_roll(0.f)
	, m_mode(CAMERA_FIRST_PERSON)
{

}

Vector3 Camera3D::GetForwardXYZ() const
{
	return Vector3(CosDegrees(m_yaw) * CosDegrees(-m_pitch), SinDegrees(m_yaw) * CosDegrees(-m_pitch), SinDegrees(-m_pitch));
}

Vector3 Camera3D::GetForwardXY() const
{
	return Vector3(CosDegrees(m_yaw), SinDegrees(m_yaw), 0.f);
}

Vector3 Camera3D::GetLeftXY() const
{
	Vector3 forward = GetForwardXY();
	return Vector3(-forward.y, forward.x, 0.f);
}
