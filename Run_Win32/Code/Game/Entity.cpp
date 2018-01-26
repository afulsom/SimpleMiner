#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

Entity::Entity()
	: m_position(0.f, 0.f)
	, m_velocity(0.f, 0.f)
	, m_orientationDegrees(0.f)
	, m_angularVelocity(0.f)
	, m_physicalRadius(0.f)
	, m_cosmeticRadius(0.f)
{

}

Entity::Entity(const Vector2& pos)
	: m_position(pos)
	, m_velocity(0.f, 0.f)
	, m_orientationDegrees(0.f)
	, m_angularVelocity(0.f)
	, m_physicalRadius(0.f)
	, m_cosmeticRadius(0.f)
{

}

Entity::Entity(const Vector2& pos, const Vector2& vel, float orientationDegrees, float angularVelocity, float physicalRadius, float cosmeticRadius, const Rgba& color)
	: m_position(pos)
	, m_velocity(vel)
	, m_orientationDegrees(orientationDegrees)
	, m_angularVelocity(angularVelocity)
	, m_physicalRadius(physicalRadius)
	, m_cosmeticRadius(cosmeticRadius)
	, m_color(color)
{

}

Entity::~Entity()
{

}

void Entity::Update(float deltaSeconds)
{
	WrapAround();

	m_position += (m_velocity * deltaSeconds);
	m_orientationDegrees += (m_angularVelocity * deltaSeconds);
}

void Entity::Render() const
{

}

Vector2 Entity::GetForward()
{
	Vector2 forward;
	forward.SetUnitLengthAndHeadingDegrees(m_orientationDegrees);
	return forward;
}

void Entity::WrapAround()
{
	if (m_position.x < -m_cosmeticRadius) //Left side
	{
		m_position.x = g_GAME_WIDTH + m_cosmeticRadius;
	}

	else if (m_position.x > g_GAME_WIDTH + m_cosmeticRadius) //Right side
	{
		m_position.x = -m_cosmeticRadius;
	}

	else if (m_position.y < -m_cosmeticRadius) //Bottom
	{
		m_position.y = g_GAME_HEIGHT + m_cosmeticRadius;
	}

	else if (m_position.y > g_GAME_HEIGHT + m_cosmeticRadius) //Top
	{
		m_position.y = -m_cosmeticRadius;
	}
}
