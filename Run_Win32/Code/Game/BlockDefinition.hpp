#pragma once
#include <vector>
#include "Game/GameCommon.hpp"



enum BlockType : unsigned char 
{
	BLOCK_TYPE_AIR,
	BLOCK_TYPE_DIRT,
	BLOCK_TYPE_GRASS,
	BLOCK_TYPE_SAND,
	BLOCK_TYPE_STONE,
	BLOCK_TYPE_GRAVEL,
	BLOCK_TYPE_COBBLESTONE,
	BLOCK_TYPE_GLOWSTONE,
	BLOCK_TYPE_WOOD_PLANKS,
	BLOCK_TYPE_WOOD_LOG,
	BLOCK_TYPE_LEAVES,
	BLOCK_TYPE_WATER,
	BLOCK_TYPE_SNOW,
	NUM_BLOCK_TYPES
};


class BlockDefinition
{
public:

	static BlockDefinition* s_blockDefinitions[NUM_BLOCK_TYPES];

	int m_topSpriteIndex;
	int m_bottomSpriteIndex;
	int m_northSpriteIndex;
	int m_southSpriteIndex;
	int m_eastSpriteIndex;
	int m_westSpriteIndex;

	bool m_isOpaque;
	bool m_isSolid;
	unsigned char m_opaqueAndSolidBits;
	unsigned char m_selfIlluminationValue;

	std::vector<SoundID> m_breakSounds;
	std::vector<SoundID> m_placeSounds;
	std::vector<SoundID> m_footstepSounds;

	BlockDefinition(int topSpriteCoords, int bottomSpriteCoords, int northSpriteCoords, int southSpriteCoords, int eastSpriteCoords, int westSpriteCoords, bool isOpaque, bool isSolid, unsigned char selfIllumination);
	SoundID GetRandomPlaceSound() const;
	SoundID GetRandomBreakSound() const;
	SoundID GetRandomFootstepSound() const;
};