#pragma  once
#include "Game/BlockDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

class Block
{
private:
	BlockType m_type;
	unsigned char m_lightingAndFlags;

public:
	Block();
	Block(BlockType type);

	BlockType GetBlockType() const;
	void ChangeType(BlockType newType);

	unsigned int GetLightValue() const;
	void SetLightValue(unsigned int newLightValue);

	bool GetIsLightingDirty() const;
	void SetIsLightingDirty();
	void ClearIsLightingDirty();

	bool GetIsSky() const;
	void SetIsSky();
	void ClearIsSky();

	bool GetIsOpaque() const;

	bool GetIsSolid() const;
};

inline BlockType Block::GetBlockType() const
{
	return m_type;
}

inline void Block::ChangeType(BlockType newType)
{
	m_lightingAndFlags &= ~IS_OPAQUE_AND_SOLID_MASK;
	const BlockDefinition& blockDef = *BlockDefinition::s_blockDefinitions[newType];
	m_lightingAndFlags |= blockDef.m_opaqueAndSolidBits;
	m_type = newType;
}

inline void Block::SetLightValue(unsigned int newLightValue)
{
	ASSERT_OR_DIE(newLightValue < 16, "Invalid light value.");
	m_lightingAndFlags &= ~LIGHT_MASK;
	m_lightingAndFlags |= newLightValue;
}

inline unsigned int Block::GetLightValue() const
{
	return m_lightingAndFlags & LIGHT_MASK;
}


inline bool Block::GetIsLightingDirty() const
{
	return (m_lightingAndFlags & LIGHT_DIRTY_MASK) == LIGHT_DIRTY_MASK;
}

inline void Block::SetIsLightingDirty()
{
	m_lightingAndFlags |= LIGHT_DIRTY_MASK;
}

inline void Block::ClearIsLightingDirty()
{
	m_lightingAndFlags &= ~LIGHT_DIRTY_MASK;
}

inline bool Block::GetIsSky() const
{
	return (m_lightingAndFlags & IS_SKY_MASK) == IS_SKY_MASK;
}

inline void Block::SetIsSky()
{
	m_lightingAndFlags |= IS_SKY_MASK;
}

inline void Block::ClearIsSky()
{
	m_lightingAndFlags &= ~IS_SKY_MASK;
}

inline bool Block::GetIsOpaque() const
{
	return (m_lightingAndFlags & IS_OPAQUE_MASK) == IS_OPAQUE_MASK;
}

inline bool Block::GetIsSolid() const
{
	return (m_lightingAndFlags & IS_SOLID_MASK) == IS_SOLID_MASK;
}
