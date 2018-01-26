#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Disc2D.hpp"
#include "Engine/Core/Rgba.hpp"


class Entity
{
	friend class Game;

public:
	Entity();
	virtual ~Entity();
	explicit Entity(const Vector2& pos);
	explicit Entity(const Vector2& pos, const Vector2& vel, float orientationDegrees, float angularVelocity, float physicalRadius, float cosmeticRadius, const Rgba& color = Rgba::WHITE);
	
	virtual void Update(float deltaSeconds);
	virtual void Render() const;

	virtual Vector2 GetForward();


protected:
	Vector2 m_position;
	Vector2 m_velocity;
	float m_orientationDegrees;
	float m_angularVelocity;
	float m_physicalRadius;
	float m_cosmeticRadius;
	Rgba m_color;
	
	void WrapAround();
	
};