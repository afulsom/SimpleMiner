#pragma once
#include "Game/Chunk.hpp"
#include "Game/BlockInfo.hpp"
#include <map>
#include <deque>


typedef IntVector2 ChunkCoords;


class World
{
public:
	World();

	void Update(float deltaSeconds, const Vector3& playerPosition);
	void Render(const Vector3& cameraPosition, const Vector3& cameraForward) const;

	void AddChunk(const ChunkCoords& chunkCoords, Chunk* newChunk);
	Chunk* GetChunk(const ChunkCoords& chunkCoords);

	int GetNumCurrentChunks() const;

	void ActivateChunk(const ChunkCoords& chunkCoords);
	void DeactivateChunk(Chunk* chunk);

	Chunk* FindFurthestChunk(const Vector3& position);
	ChunkCoords FindNearestMissingChunk(const Vector3& position);

	bool IsMissingChunkNear(const Vector3& position);
	Block* GetBlockFromWorldCoords(const Vector3& worldPosition);
	BlockInfo GetBlockInfoFromWorldCoords(const Vector3& worldPosition);
	ChunkCoords GetChunkCoordsFromWorldCoords(const Vector3& worldPosition);
	IntVector3 GetBlockCoordsFromWorldCoords(const Vector3& worldPosition);

	void DirtyBlockLighting(BlockInfo& blockInfo);
	void UpdateBlockLighting(BlockInfo& blockInfo);

	void Quit();

private:
	std::map<ChunkCoords, Chunk*> m_chunks;
	int m_numCurrentChunks;

	std::deque<BlockInfo> m_dirtyLightingQueue;

	void ManageChunks(const Vector3& playerPosition);
	void UpdateChunks(float deltaSeconds);
	void UpdateLighting();
	void UpdateVertexArrays();
	bool CanPlayerSeeChunk(const Vector3& cameraPosition, const Vector3& cameraForward, Chunk* chunkToSee) const;
};

inline ChunkCoords World::GetChunkCoordsFromWorldCoords(const Vector3& worldPosition)
{
	return ChunkCoords((int)floor(worldPosition.x) >> CHUNK_X_BITS, (int)floor(worldPosition.y) >> CHUNK_Y_BITS);
}

inline void World::DirtyBlockLighting(BlockInfo& blockInfo)
{
	if (blockInfo.GetBlock()->GetIsLightingDirty())
	{
		return;
	}

	m_dirtyLightingQueue.push_back(blockInfo);
	blockInfo.GetBlock()->SetIsLightingDirty();
}