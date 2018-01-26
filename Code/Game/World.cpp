#include "Game/World.hpp"
#include <math.h>
#include "Engine/Core/ProfileLogScope.hpp"


World::World()
	: m_numCurrentChunks(0)
{
	
}

void World::Update(float deltaSeconds, const Vector3& playerPosition)
{
	ManageChunks(playerPosition);
	UpdateChunks(deltaSeconds);
	UpdateLighting();
	UpdateVertexArrays();
}

void World::Render(const Vector3& cameraPosition, const Vector3& cameraForward) const
{
	std::map<ChunkCoords, Chunk*>::const_iterator chunkIter = m_chunks.begin();
	for (chunkIter; chunkIter != m_chunks.end(); chunkIter++)
	{
		if(CanPlayerSeeChunk(cameraPosition, cameraForward, chunkIter->second))
		{
			chunkIter->second->Render();
		}
	}
}

void World::AddChunk(const ChunkCoords& chunkCoords, Chunk* newChunk)
{
	m_chunks[chunkCoords] = newChunk;
}

Chunk* World::GetChunk(const ChunkCoords& chunkCoords)
{
	std::map<ChunkCoords, Chunk*>::iterator found = m_chunks.find(chunkCoords);
	if (found != m_chunks.end())
		return m_chunks.find(chunkCoords)->second;
	else
		return nullptr;
}

int World::GetNumCurrentChunks() const
{
	return m_numCurrentChunks;
}

void World::ActivateChunk(const ChunkCoords& chunkCoords)
{
	Chunk* newChunk = new Chunk(chunkCoords);

	Chunk* northNeighbor = GetChunk(ChunkCoords(chunkCoords.x, chunkCoords.y + 1));
	Chunk* southNeighbor = GetChunk(ChunkCoords(chunkCoords.x, chunkCoords.y - 1));
	Chunk* eastNeighbor = GetChunk(ChunkCoords(chunkCoords.x + 1, chunkCoords.y));
	Chunk* westNeighbor = GetChunk(ChunkCoords(chunkCoords.x - 1, chunkCoords.y));

	if (northNeighbor)
		northNeighbor->SetSouthNeighbor(newChunk);
	if (southNeighbor)
		southNeighbor->SetNorthNeighbor(newChunk);
	if (eastNeighbor)
		eastNeighbor->SetWestNeighbor(newChunk);
	if (westNeighbor)
		westNeighbor->SetEastNeighbor(newChunk);


	newChunk->SetNorthNeighbor(northNeighbor);
	newChunk->SetSouthNeighbor(southNeighbor);
	newChunk->SetEastNeighbor(eastNeighbor);
	newChunk->SetWestNeighbor(westNeighbor);

	m_chunks[chunkCoords] = newChunk;
	newChunk->GenerateChunk();
	newChunk->InitializeLighting();

	++m_numCurrentChunks;
}

void World::DeactivateChunk(Chunk* chunk)
{
	ChunkCoords chunkToDeleteCoords = chunk->GetChunkCoords();

	Chunk* northNeighbor = chunk->GetNorthNeighbor();
	Chunk* southNeighbor = chunk->GetSouthNeighbor();
	Chunk* eastNeighbor = chunk->GetEastNeighbor();
	Chunk* westNeighbor = chunk->GetWestNeighbor();

	if (northNeighbor)
		northNeighbor->SetSouthNeighbor(nullptr);
	if (southNeighbor)
		southNeighbor->SetNorthNeighbor(nullptr);
	if (eastNeighbor)
		eastNeighbor->SetWestNeighbor(nullptr);
	if (westNeighbor)
		westNeighbor->SetEastNeighbor(nullptr);

	chunk->SaveToFile();
	m_chunks.erase(chunkToDeleteCoords);
	delete chunk;

	--m_numCurrentChunks;
}

Chunk* World::FindFurthestChunk(const Vector3& position)
{
	Chunk* furthestChunk = new Chunk();
	float distanceSquaredToFurthestChunk = 0;
	for (std::map<ChunkCoords, Chunk*>::iterator chunkIter = m_chunks.begin(); chunkIter != m_chunks.end(); ++chunkIter)
	{
		float distanceSquaredToChunk = CalcDistanceSquared(position, chunkIter->second->GetChunkCenter());
		if (distanceSquaredToChunk > distanceSquaredToFurthestChunk)
		{
			distanceSquaredToFurthestChunk = distanceSquaredToChunk;
			furthestChunk = chunkIter->second;
		}
	}

	return furthestChunk;
}

ChunkCoords World::FindNearestMissingChunk(const Vector3& position)
{
	IntVector2 XYChunkPosition = IntVector2((int)(position.x / CHUNK_X), (int)(position.y / CHUNK_Y));
	Vector2 XYWorldPosition = Vector2(position.x, position.y);

	int numChunksXHalfExtent = (int)(ceil(VISIBILITY_RANGE / (float)CHUNK_X));
	int numChunksYHalfExtent = (int)(ceil(VISIBILITY_RANGE / (float)CHUNK_Y));

	ChunkCoords nearestMissingChunkCoords(0, 0);
	float visibilityRangeSquared = VISIBILITY_RANGE * VISIBILITY_RANGE;
	float distanceSquaredToNearestMissingChunk = visibilityRangeSquared + 1.f;

	for (int chunkYIndex = (XYChunkPosition.y - numChunksYHalfExtent); chunkYIndex < (XYChunkPosition.y + numChunksYHalfExtent); chunkYIndex++)
	{
		for (int chunkXIndex = (XYChunkPosition.x - numChunksXHalfExtent); chunkXIndex < (XYChunkPosition.x + numChunksXHalfExtent); chunkXIndex++)
		{
			if (!GetChunk(ChunkCoords(chunkXIndex, chunkYIndex)))
			{
				float distanceSquaredToChunk = CalcDistanceSquared(XYWorldPosition, Vector2((float)((chunkXIndex << CHUNK_X_BITS) + (CHUNK_X>>1)), (float)((chunkYIndex << CHUNK_Y_BITS) + (CHUNK_Y>>1))));
				if (distanceSquaredToChunk < distanceSquaredToNearestMissingChunk)
				{
					distanceSquaredToNearestMissingChunk = distanceSquaredToChunk;
					nearestMissingChunkCoords = ChunkCoords(chunkXIndex, chunkYIndex);
				}
			}
		}
	}
	return nearestMissingChunkCoords;
}

bool World::IsMissingChunkNear(const Vector3& position)
{
	IntVector2 XYChunkPosition = IntVector2((int)(position.x / CHUNK_X), (int)(position.y / CHUNK_Y));
	Vector2 XYWorldPosition = Vector2(position.x, position.y);

	int numChunksXHalfExtent = (int)(ceil(VISIBILITY_RANGE / (float)CHUNK_X));
	int numChunksYHalfExtent = (int)(ceil(VISIBILITY_RANGE / (float)CHUNK_Y));

	float visibilityRangeSquared = VISIBILITY_RANGE * VISIBILITY_RANGE;

	for (int chunkYIndex = (XYChunkPosition.y - numChunksYHalfExtent); chunkYIndex < (XYChunkPosition.y + numChunksYHalfExtent); chunkYIndex++)
	{
		for (int chunkXIndex = (XYChunkPosition.x - numChunksXHalfExtent); chunkXIndex < (XYChunkPosition.x + numChunksXHalfExtent); chunkXIndex++)
		{
			float distanceSquaredToChunk = CalcDistanceSquared(XYWorldPosition, Vector2((float)((chunkXIndex << CHUNK_X_BITS) + (CHUNK_X >> 1)), (float)((chunkYIndex << CHUNK_Y_BITS) + (CHUNK_Y >> 1))));
			if (!GetChunk(ChunkCoords(chunkXIndex, chunkYIndex)) && distanceSquaredToChunk <= visibilityRangeSquared)
			{
				return true;
			}
		}
	}

	return false;
}

Block* World::GetBlockFromWorldCoords(const Vector3& worldPosition)
{
	Chunk* currentChunk = GetChunk(GetChunkCoordsFromWorldCoords(worldPosition));
	if (!currentChunk)
		return nullptr;
	Block* currentBlock = currentChunk->GetBlockFromBlockCoords(GetBlockCoordsFromWorldCoords(worldPosition));
	return currentBlock;
}

BlockInfo World::GetBlockInfoFromWorldCoords(const Vector3& worldPosition)
{
	Chunk* currentChunk = GetChunk(GetChunkCoordsFromWorldCoords(worldPosition));
	int blockIndex;

	if (!currentChunk)
	{
		blockIndex = 0;
	}
	else
	{
		blockIndex = currentChunk->GetBlockIndexForBlockCoords(GetBlockCoordsFromWorldCoords(worldPosition));
	}

	if (blockIndex < 0 || blockIndex >= BLOCKS_PER_CHUNK)
	{
		BlockInfo outputInfo(nullptr, 0);
		return outputInfo;
	}

	BlockInfo outputInfo(currentChunk, blockIndex);
	return outputInfo;
}

IntVector3 World::GetBlockCoordsFromWorldCoords(const Vector3& worldPosition)
{
	int blockCoordX = (int)floor(worldPosition.x) % CHUNK_X;
	if (blockCoordX < 0)
		blockCoordX += CHUNK_X;
	int blockCoordY = (int)floor(worldPosition.y) % CHUNK_Y;
	if(blockCoordY < 0)
		blockCoordY += CHUNK_Y;

	return IntVector3(blockCoordX, blockCoordY, (int)floor(worldPosition.z));
}

void World::UpdateBlockLighting(BlockInfo& blockInfo)
{
	Block* block = blockInfo.GetBlock();

	//calculate ideal
	unsigned int idealLightValue = BlockDefinition::s_blockDefinitions[block->GetBlockType()]->m_selfIlluminationValue;

	if (block->GetIsSky() && SKY_LIGHT_VALUE > idealLightValue)
	{
		idealLightValue = SKY_LIGHT_VALUE;
	}

	BlockInfo topNeighbor = blockInfo.GetAboveBlock();
	BlockInfo bottomNeighbor = blockInfo.GetBelowBlock();
	BlockInfo northNeighbor = blockInfo.GetNorthBlock();
	BlockInfo southNeighbor = blockInfo.GetSouthBlock();
	BlockInfo eastNeighbor = blockInfo.GetEastBlock();
	BlockInfo westNeighbor = blockInfo.GetWestBlock();

	//check neighbors, if not opaque
	if(!blockInfo.GetBlock()->GetIsOpaque())
	{
		if (topNeighbor.m_chunk)
		{
			unsigned int topLightValue = topNeighbor.GetBlock()->GetLightValue();
			if (topLightValue > 0)
				--topLightValue;

			if (topLightValue > idealLightValue)
				idealLightValue = topLightValue;
		}

		if (bottomNeighbor.m_chunk)
		{
			unsigned int bottomLightValue = bottomNeighbor.GetBlock()->GetLightValue();
			if (bottomLightValue > 0)
				--bottomLightValue;

			if (bottomLightValue > idealLightValue)
				idealLightValue = bottomLightValue;
		}

		if (northNeighbor.m_chunk)
		{
			unsigned int northLightValue = northNeighbor.GetBlock()->GetLightValue();
			if (northLightValue > 0)
				--northLightValue;

			if (northLightValue > idealLightValue)
				idealLightValue = northLightValue;
		}

		if (southNeighbor.m_chunk)
		{
			unsigned int southLightValue = southNeighbor.GetBlock()->GetLightValue();
			if (southLightValue > 0)
				--southLightValue;

			if (southLightValue > idealLightValue)
				idealLightValue = southLightValue;
		}

		if (eastNeighbor.m_chunk)
		{
			unsigned int eastLightValue = eastNeighbor.GetBlock()->GetLightValue();
			if (eastLightValue > 0)
				--eastLightValue;

			if (eastLightValue > idealLightValue)
				idealLightValue = eastLightValue;
		}

		if (westNeighbor.m_chunk)
		{
			unsigned int westLightValue = westNeighbor.GetBlock()->GetLightValue();
			if (westLightValue > 0)
				--westLightValue;

			if (westLightValue > idealLightValue)
				idealLightValue = westLightValue;
		}
	}

	//if already ideal, return
	if (idealLightValue == block->GetLightValue())
		return;


	//set to ideal
	block->SetLightValue(idealLightValue);
	blockInfo.m_chunk->m_isVBODirty = true;

	if ((blockInfo.m_blockIndex & X_MASK_BITS) == X_MASK_BITS && blockInfo.m_chunk->GetEastNeighbor())
	{
		blockInfo.m_chunk->GetEastNeighbor()->MakeDirty();
	}
	else if ((blockInfo.m_blockIndex & X_MASK_BITS) == 0 && blockInfo.m_chunk->GetWestNeighbor())
	{
		blockInfo.m_chunk->GetWestNeighbor()->MakeDirty();
	}

	if ((blockInfo.m_blockIndex & Y_MASK_BITS) == Y_MASK_BITS && blockInfo.m_chunk->GetNorthNeighbor())
	{
		blockInfo.m_chunk->GetNorthNeighbor()->MakeDirty();
	}
	else if ((blockInfo.m_blockIndex & Y_MASK_BITS) == 0 && blockInfo.m_chunk->GetSouthNeighbor())
	{
		blockInfo.m_chunk->GetSouthNeighbor()->MakeDirty();
	}

	//dirty neighbors
	if (topNeighbor.m_chunk && !topNeighbor.GetBlock()->GetIsOpaque())
		DirtyBlockLighting(topNeighbor);

	if (bottomNeighbor.m_chunk && !bottomNeighbor.GetBlock()->GetIsOpaque())
		DirtyBlockLighting(bottomNeighbor);

	if (northNeighbor.m_chunk && !northNeighbor.GetBlock()->GetIsOpaque())
		DirtyBlockLighting(northNeighbor);

	if (southNeighbor.m_chunk && !southNeighbor.GetBlock()->GetIsOpaque())
		DirtyBlockLighting(southNeighbor);

	if (eastNeighbor.m_chunk && !eastNeighbor.GetBlock()->GetIsOpaque())
		DirtyBlockLighting(eastNeighbor);

	if (westNeighbor.m_chunk && !westNeighbor.GetBlock()->GetIsOpaque())
		DirtyBlockLighting(westNeighbor);
}

void World::ManageChunks(const Vector3& playerPosition)
{
	if (GetNumCurrentChunks() >= MAXIMUM_CHUNKS)
	{
		DeactivateChunk(FindFurthestChunk(playerPosition));
	}
	else if (IsMissingChunkNear(playerPosition))
	{
		ActivateChunk(FindNearestMissingChunk(playerPosition));
	}
	else if (GetNumCurrentChunks() > IDEAL_CHUNKS)
	{
		DeactivateChunk(FindFurthestChunk(playerPosition));
	}
}

void World::UpdateChunks(float deltaSeconds)
{
	std::map<ChunkCoords, Chunk*>::iterator chunkIter = m_chunks.begin();
	for (chunkIter; chunkIter != m_chunks.end(); chunkIter++)
	{
		chunkIter->second->Update(deltaSeconds);
	}
}

void World::UpdateLighting()
{
	while (!m_dirtyLightingQueue.empty())
	{
		BlockInfo& block = m_dirtyLightingQueue.front();
		m_dirtyLightingQueue.pop_front();
		block.GetBlock()->ClearIsLightingDirty();
		UpdateBlockLighting(block);
	}
}

void World::UpdateVertexArrays()
{
	std::map<ChunkCoords, Chunk*>::iterator chunkIter = m_chunks.begin();
	for (chunkIter; chunkIter != m_chunks.end(); chunkIter++)
	{
		if (chunkIter->second->m_isVBODirty)
		{
			chunkIter->second->RebuildVertexArray();
		}
	}
}

bool World::CanPlayerSeeChunk(const Vector3& cameraPosition, const Vector3& cameraForward, Chunk* chunkToSee) const
{
	Vector3 displacementToBottomSouthWest = chunkToSee->GetBottomSouthWestCorner() - cameraPosition;
	if (DotProduct(cameraForward, displacementToBottomSouthWest) > 0.f)
	{
		return true;
	}

	Vector3 displacementToBottomSouthEast = chunkToSee->GetBottomSouthEastCorner() - cameraPosition;
	if (DotProduct(cameraForward, displacementToBottomSouthEast) > 0.f)
	{
		return true;
	}

	Vector3 displacementToBottomNorthWest = chunkToSee->GetBottomNorthWestCorner() - cameraPosition;
	if (DotProduct(cameraForward, displacementToBottomNorthWest) > 0.f)
	{
		return true;
	}

	Vector3 displacementToBottomNorthEast = chunkToSee->GetBottomNorthEastCorner() - cameraPosition;
	if (DotProduct(cameraForward, displacementToBottomNorthEast) > 0.f)
	{
		return true;
	}

	Vector3 displacementToTopSouthWest = chunkToSee->GetTopSouthWestCorner() - cameraPosition;
	if (DotProduct(cameraForward, displacementToTopSouthWest) > 0.f)
	{
		return true;
	}

	Vector3 displacementToTopSouthEast = chunkToSee->GetTopSouthEastCorner() - cameraPosition;
	if (DotProduct(cameraForward, displacementToTopSouthEast) > 0.f)
	{
		return true;
	}

	Vector3 displacementToTopNorthWest = chunkToSee->GetTopNorthWestCorner() - cameraPosition;
	if (DotProduct(cameraForward, displacementToTopNorthWest) > 0.f)
	{
		return true;
	}

	Vector3 displacementToTopNorthEast = chunkToSee->GetTopNorthEastCorner() - cameraPosition;
	if (DotProduct(cameraForward, displacementToTopNorthEast) > 0.f)
	{
		return true;
	}

	return false;
}

void World::Quit()
{
	while (!m_chunks.empty())
	{
		DeactivateChunk(m_chunks.begin()->second);
	}
}
