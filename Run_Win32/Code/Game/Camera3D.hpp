#pragma once
#include "Engine/Math/Vector3.hpp"

enum CameraMode
{
	CAMERA_FIRST_PERSON,
	CAMERA_FROM_BEHIND,
	CAMERA_FIXED_ANGLE,
	CAMERA_NO_CLIP,
	NUM_CAMERA_MODES
};

class Camera3D
{
public:
	Vector3 m_position;
	float m_yaw;
	float m_pitch;
	float m_roll;
	
	CameraMode m_mode;

	Camera3D();

 	Vector3 GetForwardXYZ() const;
	Vector3 GetForwardXY() const;
// 	Vector3 GetLeftXYZ() const;
	Vector3 GetLeftXY() const;
// 	Vector3 GetUpXYZ() const;

};