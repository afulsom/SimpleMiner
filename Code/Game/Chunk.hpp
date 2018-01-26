#pragma once
#include "Game/Block.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Game/TreeDefinition.hpp"
#include <vector>



class Chunk
{
private:
	IntVector2 m_chunkCoords;

	Vector3 m_chunkWorldMins;
	Vector3 m_chunkWorldMaxs;
	Vector3 m_chunkCenter;

	Vector3 m_topNorthEastCorner;
	Vector3 m_topNorthWestCorner;
	Vector3 m_topSouthEastCorner;
	Vector3 m_topSouthWestCorner;

	Vector3 m_bottomNorthEastCorner;
	Vector3 m_bottomNorthWestCorner;
	Vector3 m_bottomSouthEastCorner;
	Vector3 m_bottomSouthWestCorner;

	Block m_blocks[BLOCKS_PER_CHUNK];

	unsigned int m_vboID;
	unsigned int m_numVertexesInVBO;

	Chunk* m_northNeighbor;
	Chunk* m_southNeighbor;
	Chunk* m_eastNeighbor;
	Chunk* m_westNeighbor;

	const Vector3 CalcChunkMins() const;
	bool IsLocalMaxima(float* arrayValues, int indexInArray, int numValues, int xDimension) const;
	void PlaceTreeBlocks(const Vector3& worldStartPosition, TreeDefinition treeToPlace);
public:
	bool m_isVBODirty;

	Chunk();
	Chunk(const IntVector2& chunkCoords);
	~Chunk();

	void Update(float deltaSeconds);
	void Render() const;

	void RebuildVertexArray();

	void GenerateChunk();
	void PopulateFromFile(const std::vector<unsigned char>& fileBuffer);
	void PopulateFromNoise();

	void InitializeLighting();

	bool SaveToFile();
	void CompressToRLE(std::vector<unsigned char>& out_chunkBuffer);

	int GetBlockIndexForBlockCoords(IntVector3 blockCoords);
	IntVector3 GetBlockCoordsForBlockIndex(int blockIndex);

	const IntVector2& GetChunkCoords() const;
	const Vector3& GetChunkWorldMins() const;

	const Vector3& GetChunkCenter() const;
	Block* GetBlockFromBlockCoords(const IntVector3& blockCoords);
	Block* GetBlockFromBlockIndex(int blockIndex);
	void MakeDirty();

	void SetNorthNeighbor(Chunk* northNeighbor);
	void SetSouthNeighbor(Chunk* southNeighbor);
	void SetEastNeighbor(Chunk* eastNeighbor);
	void SetWestNeighbor(Chunk* westNeighbor);

	Chunk* GetNorthNeighbor();
	Chunk* GetSouthNeighbor();
	Chunk* GetEastNeighbor();
	Chunk* GetWestNeighbor();

	Vector3 GetBottomSouthWestCorner();
	Vector3 GetBottomSouthEastCorner();
	Vector3 GetBottomNorthWestCorner();
	Vector3 GetBottomNorthEastCorner();

	Vector3 GetTopSouthWestCorner();
	Vector3 GetTopSouthEastCorner();
	Vector3 GetTopNorthWestCorner();
	Vector3 GetTopNorthEastCorner();

};


inline int Chunk::GetBlockIndexForBlockCoords(IntVector3 blockCoords)
{
	return blockCoords.x + (blockCoords.y * CHUNK_X) + (blockCoords.z * BLOCKS_PER_LAYER);
}

inline IntVector3 Chunk::GetBlockCoordsForBlockIndex(int blockIndex)
{
	int blockX = blockIndex & X_MASK_BITS;
	int blockY = (blockIndex & Y_MASK_BITS) >> CHUNK_X_BITS;
	int blockZ = (blockIndex & Z_MASK_BITS) >> CHUNK_XY_BITS;

	return IntVector3(blockX, blockY, blockZ);
}

inline const Vector3& Chunk::GetChunkWorldMins() const
{
	return m_chunkWorldMins;
}

inline Block* Chunk::GetBlockFromBlockCoords(const IntVector3& blockCoords)
{
	if (blockCoords.x < 0 || blockCoords.x > CHUNK_X || blockCoords.y < 0 || blockCoords.y > CHUNK_Y || blockCoords.z < 0 || blockCoords.z >= CHUNK_Z)
		return nullptr;
	int blockIndex = GetBlockIndexForBlockCoords(blockCoords);
	return GetBlockFromBlockIndex(blockIndex);
}

inline Block* Chunk::GetBlockFromBlockIndex(int blockIndex)
{
	if (blockIndex >= BLOCKS_PER_CHUNK || blockIndex < 0)
		return nullptr;
	return &m_blocks[blockIndex];
}

inline void Chunk::MakeDirty()
{
	m_isVBODirty = true;
}


inline const IntVector2& Chunk::GetChunkCoords() const
{
	return m_chunkCoords;
}

inline const Vector3& Chunk::GetChunkCenter() const
{
	return m_chunkCenter;
}

inline void Chunk::SetNorthNeighbor(Chunk* northNeighbor)
{
	m_northNeighbor = northNeighbor;
}

inline void Chunk::SetSouthNeighbor(Chunk* southNeighbor)
{
	m_southNeighbor = southNeighbor;
}

inline void Chunk::SetEastNeighbor(Chunk* eastNeighbor)
{
	m_eastNeighbor = eastNeighbor;
}

inline void Chunk::SetWestNeighbor(Chunk* westNeighbor)
{
	m_westNeighbor = westNeighbor;
}

inline Chunk* Chunk::GetNorthNeighbor()
{
	return m_northNeighbor;
}

inline Chunk* Chunk::GetSouthNeighbor()
{
	return m_southNeighbor;
}

inline Chunk* Chunk::GetEastNeighbor()
{
	return m_eastNeighbor;
}

inline Chunk* Chunk::GetWestNeighbor()
{
	return m_westNeighbor;
}

inline const Vector3 Chunk::CalcChunkMins() const
{
	return Vector3((float)m_chunkCoords.x * CHUNK_X, (float)m_chunkCoords.y * CHUNK_Y, 0.f);
}

inline Vector3 Chunk::GetBottomSouthWestCorner()
{
	return m_bottomSouthWestCorner;
}

inline Vector3 Chunk::GetBottomSouthEastCorner()
{
	return m_bottomSouthEastCorner;
}

inline Vector3 Chunk::GetBottomNorthWestCorner()
{
	return m_bottomNorthWestCorner;
}

inline Vector3 Chunk::GetBottomNorthEastCorner()
{
	return m_bottomNorthEastCorner;
}

inline Vector3 Chunk::GetTopSouthWestCorner()
{
	return m_topSouthWestCorner;
}
inline Vector3 Chunk::GetTopSouthEastCorner()
{
	return m_topSouthEastCorner;
}

inline Vector3 Chunk::GetTopNorthWestCorner()
{
	return m_topNorthWestCorner;
}

inline Vector3 Chunk::GetTopNorthEastCorner()
{
	return m_topNorthEastCorner;
}
