#include "Game/Block.hpp"


Block::Block(BlockType type)
	: m_type(type)
	, m_lightingAndFlags(0x00)
{

}

Block::Block()
	: m_type(BLOCK_TYPE_AIR)
	, m_lightingAndFlags(0x00)
{

}

