#include "Game/BlockInfo.hpp"

void BlockInfo::ChangeType(BlockType newType)
{
	if (m_chunk == nullptr)
	{
		return;
	}

	Block* block = GetBlock();
	block->ChangeType(newType);
	m_chunk->m_isVBODirty = true;
}
