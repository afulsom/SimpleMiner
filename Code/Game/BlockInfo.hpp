#pragma once
#include "Game/Chunk.hpp"


class BlockInfo
{
public:
	Chunk* m_chunk;
	int m_blockIndex;

	BlockInfo();
	BlockInfo(Chunk* chunk, int blockIndex);

	Block* GetBlock() const;

	void ChangeType(BlockType newType);

	void MoveEast();
	BlockInfo GetEastBlock() const;
	void MoveWest();
	BlockInfo GetWestBlock() const;
	void MoveNorth();
	BlockInfo GetNorthBlock() const;
	void MoveSouth();
	BlockInfo GetSouthBlock() const;
	void MoveUp();
	BlockInfo GetAboveBlock() const;
	void MoveDown();
	BlockInfo GetBelowBlock() const;
};

inline BlockInfo::BlockInfo()
	: m_chunk(nullptr)
	, m_blockIndex(0)
{

}

inline BlockInfo::BlockInfo(Chunk* chunk, int blockIndex)
	: m_chunk(chunk)
	, m_blockIndex(blockIndex)
{

}

inline Block* BlockInfo::GetBlock() const
{
	if (!m_chunk)
		return nullptr;
	return m_chunk->GetBlockFromBlockIndex(m_blockIndex);
}

inline void BlockInfo::MoveEast()
{
	if (m_chunk == nullptr)
	{
		return;
	}

	if ((m_blockIndex & X_MASK_BITS) == X_MASK_BITS)		//if it is on the eastern edge of the chunk, move to next chunk
	{
		m_chunk = m_chunk->GetEastNeighbor();
		m_blockIndex &= ~X_MASK_BITS;
	}
	else
	{
		++m_blockIndex;
	}
}

inline BlockInfo BlockInfo::GetEastBlock() const
{
	BlockInfo output(m_chunk, m_blockIndex);
	output.MoveEast();
	return output;
}


inline void BlockInfo::MoveWest()
{
	if (m_chunk == nullptr)
	{
		return;
	}

	if ((m_blockIndex & X_MASK_BITS) == 0)		//if it is on the western edge of the chunk, move to next chunk
	{
		m_chunk = m_chunk->GetWestNeighbor();
		m_blockIndex |= X_MASK_BITS;
	}
	else
	{
		--m_blockIndex;
	}
}

inline BlockInfo BlockInfo::GetWestBlock() const
{
	BlockInfo output(m_chunk, m_blockIndex);
	output.MoveWest();
	return output;
}

inline void BlockInfo::MoveNorth()
{
	if (m_chunk == nullptr)
	{
		return;
	}

	if ((m_blockIndex & Y_MASK_BITS) == Y_MASK_BITS)		//if it is on the northern edge of the chunk, move to next chunk
	{
		m_chunk = m_chunk->GetNorthNeighbor();
		m_blockIndex &= ~Y_MASK_BITS;
	}
	else
	{
		m_blockIndex += CHUNK_X;
	}
}

inline BlockInfo BlockInfo::GetNorthBlock() const
{
	BlockInfo output(m_chunk, m_blockIndex);
	output.MoveNorth();
	return output;
}

inline void BlockInfo::MoveSouth()
{
	if (m_chunk == nullptr)
	{
		return;
	}

	if ((m_blockIndex & Y_MASK_BITS) == 0)		//if it is on the southern edge of the chunk, move to next chunk
	{
		m_chunk = m_chunk->GetSouthNeighbor();
		m_blockIndex |= Y_MASK_BITS;
	}
	else
	{
		m_blockIndex -= CHUNK_X;
	}
}

inline BlockInfo BlockInfo::GetSouthBlock() const
{
	BlockInfo output(m_chunk, m_blockIndex);
	output.MoveSouth();
	return output;
}

inline void BlockInfo::MoveUp()
{
	if (m_chunk == nullptr)
	{
		return;
	}

	if ((m_blockIndex & Z_MASK_BITS) == Z_MASK_BITS)		//if it is on the top edge of the chunk, set it to a null chunk
	{
		m_chunk = nullptr;
		m_blockIndex &= ~Z_MASK_BITS;
	}
	else
	{
		m_blockIndex += BLOCKS_PER_LAYER;
	}
}

inline BlockInfo BlockInfo::GetAboveBlock() const
{
	BlockInfo output(m_chunk, m_blockIndex);
	output.MoveUp();
	return output;
}

inline void BlockInfo::MoveDown()
{
	if (m_chunk == nullptr)
	{
		return;
	}

	if ((m_blockIndex & Z_MASK_BITS) == 0)		//if it is on the bottom edge of the chunk, set it to a null chunk
	{
		m_chunk = nullptr;
		m_blockIndex |= Z_MASK_BITS;
	}
	else
	{
		m_blockIndex -= BLOCKS_PER_LAYER;
	}
}

inline BlockInfo BlockInfo::GetBelowBlock() const
{
	BlockInfo output(m_chunk, m_blockIndex);
	output.MoveDown();
	return output;
}