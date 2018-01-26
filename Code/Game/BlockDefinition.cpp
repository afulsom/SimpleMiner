#include "Game/BlockDefinition.hpp"
#include "Game/GameCommon.hpp"

BlockDefinition* BlockDefinition::s_blockDefinitions[];

BlockDefinition::BlockDefinition(int topSpriteCoords, int bottomSpriteCoords, int northSpriteCoords, int southSpriteCoords, int eastSpriteCoords, int westSpriteCoords, bool isOpaque, bool isSolid, unsigned char selfIllumination) : m_topSpriteIndex(topSpriteCoords)
, m_bottomSpriteIndex(bottomSpriteCoords)
, m_northSpriteIndex(northSpriteCoords)
, m_southSpriteIndex(southSpriteCoords)
, m_eastSpriteIndex(eastSpriteCoords)
, m_westSpriteIndex(westSpriteCoords)
, m_isOpaque(isOpaque)
, m_isSolid(isSolid)
, m_selfIlluminationValue(selfIllumination)
, m_breakSounds()
, m_placeSounds()
, m_footstepSounds()
{
	unsigned char opaqueBit;
	if (isOpaque)
		opaqueBit = IS_OPAQUE_MASK;
	else
		opaqueBit = 0b00000000;

	unsigned char solidBit;
	if (isSolid)
		solidBit = IS_SOLID_MASK;
	else
		solidBit = 0b00000000;

	m_opaqueAndSolidBits = opaqueBit | solidBit;
}

SoundID BlockDefinition::GetRandomPlaceSound() const
{
	int index = GetRandomIntLessThan(m_placeSounds.size());
	return m_placeSounds[index];
}

SoundID BlockDefinition::GetRandomBreakSound() const
{
	int index = GetRandomIntLessThan(m_breakSounds.size());
	return m_breakSounds[index];
}

SoundID BlockDefinition::GetRandomFootstepSound() const
{
	int index = GetRandomIntLessThan(m_footstepSounds.size());
	return m_footstepSounds[index];
}

