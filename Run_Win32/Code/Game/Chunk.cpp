#include "Game/Chunk.hpp"
#include "Engine/Core/Noise.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/BlockInfo.hpp"
#include "Game/App.hpp"
#include "Engine/Core/ProfileLogScope.hpp"



Chunk::Chunk()
	: m_chunkCoords(IntVector2(0, 0))
	, m_isVBODirty(true)
	, m_numVertexesInVBO(0)
{
	g_theRenderer->CreateVBOs(1, &m_vboID);
}

Chunk::Chunk(const IntVector2& chunkCoords)
	: m_chunkCoords(chunkCoords)
	, m_chunkWorldMins()
	, m_chunkWorldMaxs()
	, m_chunkCenter()
	, m_isVBODirty(true)
	, m_numVertexesInVBO(0)
	, m_northNeighbor(nullptr)
	, m_southNeighbor(nullptr)
	, m_eastNeighbor(nullptr)
	, m_westNeighbor(nullptr)
{
	m_chunkWorldMins = CalcChunkMins();
	m_chunkWorldMaxs = m_chunkWorldMins + Vector3((float)CHUNK_X, (float)CHUNK_Y, 0.f);
	m_chunkCenter = (m_chunkWorldMaxs + m_chunkWorldMins) / 2;

	m_bottomSouthWestCorner = m_chunkWorldMins;
	m_bottomSouthEastCorner = m_bottomSouthWestCorner + Vector3((float)CHUNK_X, 0.f, 0.f);
	m_bottomNorthWestCorner = m_bottomSouthWestCorner + Vector3(0.f, (float)CHUNK_Y, 0.f);
	m_bottomNorthEastCorner = m_chunkWorldMaxs;

	m_topSouthWestCorner = m_bottomSouthWestCorner;
	m_topSouthEastCorner = m_bottomSouthEastCorner;
	m_topNorthWestCorner = m_bottomNorthWestCorner;
	m_topNorthEastCorner = m_bottomNorthEastCorner;

	m_topSouthWestCorner.z += CHUNK_Z;
	m_topSouthEastCorner.z += CHUNK_Z;
	m_topNorthWestCorner.z += CHUNK_Z;
	m_topNorthEastCorner.z += CHUNK_Z;

	g_theRenderer->CreateVBOs(1, &m_vboID);
}


Chunk::~Chunk()
{
	g_theRenderer->DeleteVBOs(1, &m_vboID);
}

void Chunk::Update(float deltaSeconds)
{
	deltaSeconds;
}

void Chunk::Render() const
{
	Vector3 chunkWorldMins = CalcChunkMins();

	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinates3D(chunkWorldMins);

	g_theRenderer->BindBuffer(m_vboID);
	g_theRenderer->DrawVBO(m_numVertexesInVBO, PRIMITIVE_QUADS);
	g_theRenderer->BindBuffer(0);

	g_theRenderer->PopMatrix();
}

void Chunk::RebuildVertexArray()
{
	std::vector<Vertex3D> vertexArray;
	vertexArray.reserve(CHUNK_X * CHUNK_Y * 4 * 4);

	for (int zIndex = 0; zIndex < CHUNK_Z; zIndex++)
	{
		for (int yIndex = 0; yIndex < CHUNK_Y; yIndex++)
		{
			for (int xIndex = 0; xIndex < CHUNK_X; xIndex++)
			{
				IntVector3 blockCoords = IntVector3(xIndex, yIndex, zIndex);
				int blockIndex = GetBlockIndexForBlockCoords(blockCoords);
				BlockInfo blockToDraw(this, blockIndex);

				if(BlockDefinition::s_blockDefinitions[blockToDraw.GetBlock()->GetBlockType()]->m_isOpaque)
				{
					//Get neighboring blocks
					BlockInfo bottomNeighbor = blockToDraw.GetBelowBlock();
					BlockInfo topNeighbor = blockToDraw.GetAboveBlock();
					BlockInfo northNeighbor = blockToDraw.GetNorthBlock();
					BlockInfo southNeighbor = blockToDraw.GetSouthBlock();
					BlockInfo eastNeighbor = blockToDraw.GetEastBlock();
					BlockInfo westNeighbor = blockToDraw.GetWestBlock();

					//For each face, check is neighbor is opaque
					//if false, add vertexes for face
					if (!bottomNeighbor.m_chunk || !BlockDefinition::s_blockDefinitions[bottomNeighbor.GetBlock()->GetBlockType()]->m_isOpaque)
					{
						AABB2 bottomTexCoords = g_blockSprites->GetTexCoordsForSpriteIndex(BlockDefinition::s_blockDefinitions[blockToDraw.GetBlock()->GetBlockType()]->m_bottomSpriteIndex);
						unsigned char lightRGBAValue;
						if (!bottomNeighbor.m_chunk)
							lightRGBAValue = LIGHT_RGBA_VALUES[SKY_LIGHT_VALUE];
						else
							lightRGBAValue = LIGHT_RGBA_VALUES[bottomNeighbor.GetBlock()->GetLightValue()];

						Vertex3D bottomFaceVertexes[] = 
						{
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 0.f, 0.f), Vector2(bottomTexCoords.maxs.x, bottomTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 0.f, 0.f), Vector2(bottomTexCoords.mins.x, bottomTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 1.f, 0.f), Vector2(bottomTexCoords.mins.x, bottomTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 1.f, 0.f), Vector2(bottomTexCoords.maxs.x, bottomTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255))
						};

						vertexArray.push_back(bottomFaceVertexes[0]);
						vertexArray.push_back(bottomFaceVertexes[1]);
						vertexArray.push_back(bottomFaceVertexes[2]);
						vertexArray.push_back(bottomFaceVertexes[3]);
					}

					if (!topNeighbor.m_chunk || !BlockDefinition::s_blockDefinitions[topNeighbor.GetBlock()->GetBlockType()]->m_isOpaque)
					{
						AABB2 topTexCoords = g_blockSprites->GetTexCoordsForSpriteIndex(BlockDefinition::s_blockDefinitions[blockToDraw.GetBlock()->GetBlockType()]->m_topSpriteIndex);
						unsigned char lightRGBAValue;
						if (!topNeighbor.m_chunk)
							lightRGBAValue = LIGHT_RGBA_VALUES[SKY_LIGHT_VALUE];
						else
							lightRGBAValue = LIGHT_RGBA_VALUES[topNeighbor.GetBlock()->GetLightValue()];

						Vertex3D topFaceVertexes[] =
						{
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 0.f, 1.f), Vector2(topTexCoords.mins.x, topTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 0.f, 1.f), Vector2(topTexCoords.maxs.x, topTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 1.f, 1.f), Vector2(topTexCoords.maxs.x, topTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 1.f, 1.f), Vector2(topTexCoords.mins.x, topTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255))
						};

						vertexArray.push_back(topFaceVertexes[0]);
						vertexArray.push_back(topFaceVertexes[1]);
						vertexArray.push_back(topFaceVertexes[2]);
						vertexArray.push_back(topFaceVertexes[3]);
					}

					if (!northNeighbor.m_chunk || northNeighbor.m_chunk != this || !BlockDefinition::s_blockDefinitions[northNeighbor.GetBlock()->GetBlockType()]->m_isOpaque)
					{
						AABB2 northTexCoords = g_blockSprites->GetTexCoordsForSpriteIndex(BlockDefinition::s_blockDefinitions[blockToDraw.GetBlock()->GetBlockType()]->m_northSpriteIndex);
						unsigned char lightRGBAValue;
						if (!northNeighbor.m_chunk)
							lightRGBAValue = LIGHT_RGBA_VALUES[SKY_LIGHT_VALUE];
						else
							lightRGBAValue = LIGHT_RGBA_VALUES[northNeighbor.GetBlock()->GetLightValue()];

						Vertex3D northFaceVertexes[] = 
						{
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 1.f, 0.f), Vector2(northTexCoords.mins.x, northTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 1.f, 0.f), Vector2(northTexCoords.maxs.x, northTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 1.f, 1.f), Vector2(northTexCoords.maxs.x, northTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 1.f, 1.f), Vector2(northTexCoords.mins.x, northTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255))
						};

						vertexArray.push_back(northFaceVertexes[0]);
						vertexArray.push_back(northFaceVertexes[1]);
						vertexArray.push_back(northFaceVertexes[2]);
						vertexArray.push_back(northFaceVertexes[3]);
					}

					if (!southNeighbor.m_chunk || southNeighbor.m_chunk != this || !BlockDefinition::s_blockDefinitions[southNeighbor.GetBlock()->GetBlockType()]->m_isOpaque)
					{
						AABB2 southTexCoords = g_blockSprites->GetTexCoordsForSpriteIndex(BlockDefinition::s_blockDefinitions[blockToDraw.GetBlock()->GetBlockType()]->m_southSpriteIndex);
						unsigned char lightRGBAValue;
						if (!southNeighbor.m_chunk)
							lightRGBAValue = LIGHT_RGBA_VALUES[SKY_LIGHT_VALUE];
						else
							lightRGBAValue = LIGHT_RGBA_VALUES[southNeighbor.GetBlock()->GetLightValue()];

						Vertex3D southFaceVertexes[] = 
						{
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 0.f, 0.f), Vector2(southTexCoords.mins.x, southTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 0.f, 0.f), Vector2(southTexCoords.maxs.x, southTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 0.f, 1.f), Vector2(southTexCoords.maxs.x, southTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 0.f, 1.f), Vector2(southTexCoords.mins.x, southTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255))
						};

						vertexArray.push_back(southFaceVertexes[0]);
						vertexArray.push_back(southFaceVertexes[1]);
						vertexArray.push_back(southFaceVertexes[2]);
						vertexArray.push_back(southFaceVertexes[3]);
					}

					if (!eastNeighbor.m_chunk || eastNeighbor.m_chunk != this || !BlockDefinition::s_blockDefinitions[eastNeighbor.GetBlock()->GetBlockType()]->m_isOpaque)
					{
						AABB2 eastTexCoords = g_blockSprites->GetTexCoordsForSpriteIndex(BlockDefinition::s_blockDefinitions[blockToDraw.GetBlock()->GetBlockType()]->m_eastSpriteIndex);
						unsigned char lightRGBAValue;
						if (!eastNeighbor.m_chunk)
							lightRGBAValue = LIGHT_RGBA_VALUES[SKY_LIGHT_VALUE];
						else
							lightRGBAValue = LIGHT_RGBA_VALUES[eastNeighbor.GetBlock()->GetLightValue()];

						Vertex3D eastFaceVertexes[] = 
						{
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 0.f, 0.f), Vector2(eastTexCoords.mins.x, eastTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 1.f, 0.f), Vector2(eastTexCoords.maxs.x, eastTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 1.f, 1.f), Vector2(eastTexCoords.maxs.x, eastTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(1.f, 0.f, 1.f), Vector2(eastTexCoords.mins.x, eastTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255))
						};

						vertexArray.push_back(eastFaceVertexes[0]);
						vertexArray.push_back(eastFaceVertexes[1]);
						vertexArray.push_back(eastFaceVertexes[2]);
						vertexArray.push_back(eastFaceVertexes[3]);
					}

					if (!westNeighbor.m_chunk || westNeighbor.m_chunk != this || !BlockDefinition::s_blockDefinitions[westNeighbor.GetBlock()->GetBlockType()]->m_isOpaque)
					{
						AABB2 westTexCoords = g_blockSprites->GetTexCoordsForSpriteIndex(BlockDefinition::s_blockDefinitions[blockToDraw.GetBlock()->GetBlockType()]->m_westSpriteIndex);
						unsigned char lightRGBAValue;
						if (!westNeighbor.m_chunk)
							lightRGBAValue = LIGHT_RGBA_VALUES[SKY_LIGHT_VALUE];
						else
							lightRGBAValue = LIGHT_RGBA_VALUES[westNeighbor.GetBlock()->GetLightValue()];

						Vertex3D westFaceVertexes[] = 
						{
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 1.f, 0.f), Vector2(westTexCoords.mins.x, westTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 0.f, 0.f), Vector2(westTexCoords.maxs.x, westTexCoords.maxs.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 0.f, 1.f), Vector2(westTexCoords.maxs.x, westTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255)),
							Vertex3D(Vector3(blockCoords) + Vector3(0.f, 1.f, 1.f), Vector2(westTexCoords.mins.x, westTexCoords.mins.y), Rgba(lightRGBAValue, lightRGBAValue, lightRGBAValue, 255))
						};

						vertexArray.push_back(westFaceVertexes[0]);
						vertexArray.push_back(westFaceVertexes[1]);
						vertexArray.push_back(westFaceVertexes[2]);
						vertexArray.push_back(westFaceVertexes[3]);
					}
				}
			}
		}
	}

	g_theRenderer->BindBuffer(m_vboID);
	g_theRenderer->BufferData(vertexArray.data(), vertexArray.size() * sizeof(Vertex3D));
	g_theRenderer->BindBuffer(0);
	m_numVertexesInVBO = vertexArray.size();
	m_isVBODirty = false;
}

void Chunk::GenerateChunk()
{
	std::vector<unsigned char> fileBuffer;
	std::string filePath = "Data/Save/Chunk_(" + std::to_string(m_chunkCoords.x) + "," + std::to_string(m_chunkCoords.y) + ").cnk";
	if (ReadBufferFromFile(fileBuffer, filePath))
	{
		PopulateFromFile(fileBuffer);
	}
	else
	{
		PopulateFromNoise();
	}
}

void Chunk::PopulateFromFile(const std::vector<unsigned char>& fileBuffer)
{
	ASSERT_OR_DIE(fileBuffer[0] == g_GAME_VERSION, "Chunk file version mismatch");

	int currentBlockIndex = 0;
	for (std::vector<unsigned char>::const_iterator bufferIter = fileBuffer.begin() + 1; bufferIter != fileBuffer.end(); bufferIter += 2)
	{
		BlockType currentType = (BlockType)*bufferIter;
		int numBlocksOfCurrentType = (int)*(bufferIter + 1);

		for (int blockIndex = 0; blockIndex < numBlocksOfCurrentType; blockIndex++)
		{
			m_blocks[currentBlockIndex].ChangeType(currentType);
			currentBlockIndex++;
		}
	}
}

void Chunk::PopulateFromNoise()
{
	Vector3 chunkWorldMins = CalcChunkMins();

	constexpr int mountainousnessSize = (CHUNK_X + 8) * (CHUNK_Y + 8);
	float mountainousness[mountainousnessSize];
	float mountainousnessAmplitude = 20.f;
	for (int xIndex = -4; xIndex < CHUNK_X + 4; xIndex++)
	{
		for (int yIndex = -4; yIndex < CHUNK_Y + 4; yIndex++)
		{
			mountainousness[(xIndex + 4) + ((yIndex + 4) * (CHUNK_X + 8))] = (mountainousnessAmplitude * Compute2dPerlinNoise(xIndex + chunkWorldMins.x, yIndex + chunkWorldMins.y, 500.f, 5, 0.2f, 2.f, true, 35)) + mountainousnessAmplitude;
		}
	}

	constexpr int columnHeightsSize = (CHUNK_X + 8) * (CHUNK_Y + 8);
	int columnHeights[columnHeightsSize];
	for (int xIndex = -4; xIndex < CHUNK_X + 4; xIndex++)
	{
		for (int yIndex = -4; yIndex < CHUNK_Y + 4; yIndex++)
		{
			float currentColumnHeight = (float)SEA_LEVEL;
			currentColumnHeight += mountainousness[(xIndex + 4) + ((yIndex + 4) * (CHUNK_X + 8))] * Compute2dPerlinNoise(xIndex + chunkWorldMins.x, yIndex + chunkWorldMins.y, 60.f, 5, 0.4f);
			columnHeights[(xIndex + 4) + ((yIndex + 4) * (CHUNK_X + 8))] = (int)currentColumnHeight;
		}
	}
	
	constexpr int wetnessSize = (CHUNK_X + 10) * (CHUNK_Y + 10);
	float wetness[wetnessSize];
	for (int yIndex = -5; yIndex < CHUNK_Y + 5; yIndex++)
	{
		for (int xIndex = -5; xIndex < CHUNK_X + 5; xIndex++)
		{
			int wetnessIndex = (xIndex + 5) + ((yIndex + 5) * (CHUNK_X + 10));
			wetness[wetnessIndex] = RangeMapFloat(Compute2dPerlinNoise(xIndex + chunkWorldMins.x, yIndex + chunkWorldMins.y, 40.f, 4, 0.5f, 1.5f, true, 3534879), -1.f, 1.f, 0.f, 2.f);
		}
	}

	constexpr int temperatureSize = (CHUNK_X + 8) * (CHUNK_Y + 8);
	float temperature[temperatureSize];
	for (int xIndex = -4; xIndex < CHUNK_X + 4; xIndex++)
	{
		for (int yIndex = -4; yIndex < CHUNK_Y + 4; yIndex++)
		{
			temperature[(xIndex + 4) + ((yIndex + 4) * (CHUNK_X + 8))] = RangeMapFloat(Compute2dPerlinNoise(xIndex + chunkWorldMins.x, yIndex + chunkWorldMins.y, 500.f, 5, 0.4f, 2.f, true, 543253), -1.f, 1.f, 0.f, 100.f);
		}
	}


	constexpr int treeValuesSize = (CHUNK_X + 10) * (CHUNK_Y + 10);
	float treeValues[treeValuesSize];
	for (int yIndex = -5; yIndex < CHUNK_Y + 5; yIndex++)
	{
		for (int xIndex = -5; xIndex < CHUNK_X + 5; xIndex++)
		{
			float octaveScale = wetness[(xIndex + 5) + ((yIndex + 5) * (CHUNK_X + 10))];
			treeValues[(xIndex + 5) + ((yIndex + 5) * (CHUNK_X + 10))] = Compute2dPerlinNoise(xIndex + chunkWorldMins.x, yIndex + chunkWorldMins.y, 100.f, 5, 0.2f, octaveScale, true, 1);
		}
	}

	for (int zIndex = 0; zIndex < CHUNK_Z; zIndex++)
	{
		for (int yIndex = 0; yIndex < CHUNK_Y; yIndex++)
		{
			for (int xIndex = 0; xIndex < CHUNK_X; xIndex++)
			{
				int columnHeight = columnHeights[(xIndex + 4) + ((yIndex + 4) * (CHUNK_X + 8))];
				float blockWetness = wetness[(xIndex + 5) + ((yIndex + 5) * (CHUNK_X + 10))];
				float blockTemperature = temperature[(xIndex + 4) + ((yIndex + 4) * (CHUNK_X + 8))];

				if (zIndex < columnHeight + STONE_OFFSET)
				{
					m_blocks[xIndex + (yIndex * CHUNK_X) + (zIndex * BLOCKS_PER_LAYER)].ChangeType(BLOCK_TYPE_STONE);
				}
				else if (zIndex < columnHeight + DIRT_OFFSET)
				{
					if (zIndex <= SEA_LEVEL || (blockWetness < DESERT_WETNESS_MAXIMUM && blockTemperature > DESERT_TEMPERATURE_MINIMUM))
						m_blocks[xIndex + (yIndex * CHUNK_X) + (zIndex * BLOCKS_PER_LAYER)].ChangeType(BLOCK_TYPE_SAND);
					else
						m_blocks[xIndex + (yIndex * CHUNK_X) + (zIndex * BLOCKS_PER_LAYER)].ChangeType(BLOCK_TYPE_DIRT);
				}
				else if (zIndex < columnHeight + GRASS_OFFSET)
				{
					if (zIndex <= SEA_LEVEL || (blockWetness < DESERT_WETNESS_MAXIMUM && blockTemperature > DESERT_TEMPERATURE_MINIMUM))
						m_blocks[xIndex + (yIndex * CHUNK_X) + (zIndex * BLOCKS_PER_LAYER)].ChangeType(BLOCK_TYPE_SAND);
					else if(blockTemperature < SNOW_TEMPERATURE_MAXIMUM)
						m_blocks[xIndex + (yIndex * CHUNK_X) + (zIndex * BLOCKS_PER_LAYER)].ChangeType(BLOCK_TYPE_SNOW);
					else
						m_blocks[xIndex + (yIndex * CHUNK_X) + (zIndex * BLOCKS_PER_LAYER)].ChangeType(BLOCK_TYPE_GRASS);
				}
				else
				{
					if (zIndex <= SEA_LEVEL)
						m_blocks[xIndex + (yIndex * CHUNK_X) + (zIndex * BLOCKS_PER_LAYER)].ChangeType(BLOCK_TYPE_WATER);
					else
						m_blocks[xIndex + (yIndex * CHUNK_X) + (zIndex * BLOCKS_PER_LAYER)].ChangeType(BLOCK_TYPE_AIR);
				}
			}
		}
	}

	for (int xIndex = -4; xIndex < CHUNK_X + 4; xIndex++)
	{
		for (int yIndex = -4; yIndex < CHUNK_Y + 4; yIndex++)
		{
			int columnHeight = (columnHeights[(xIndex + 4) + ((yIndex + 4) * (CHUNK_X + 8))] + GRASS_OFFSET);
			if (IsLocalMaxima(treeValues, ((xIndex + 5) + ((yIndex + 5) * (CHUNK_X + 10))), treeValuesSize, CHUNK_X + 10) && columnHeight > SEA_LEVEL)
			{
				Vector3 startPosition(m_chunkWorldMins.x + xIndex, m_chunkWorldMins.y + yIndex, (float)columnHeight);
				if(temperature[(xIndex + 4) + ((yIndex + 4) * (CHUNK_X + 8))] > 80.f)
					PlaceTreeBlocks(startPosition, *TreeDefinition::s_treeDefinitions[TREE_TYPE_WIMBA]);
				else if(temperature[(xIndex + 4) + ((yIndex + 4) * (CHUNK_X + 8))] > 60.f)
					PlaceTreeBlocks(startPosition, *TreeDefinition::s_treeDefinitions[TREE_TYPE_WILLOW]);
				else if (temperature[(xIndex + 4) + ((yIndex + 4) * (CHUNK_X + 8))] > 40.f)
					PlaceTreeBlocks(startPosition, *TreeDefinition::s_treeDefinitions[TREE_TYPE_OAK]);
				else
					PlaceTreeBlocks(startPosition, *TreeDefinition::s_treeDefinitions[TREE_TYPE_PINE]);
			}
		}
	}
}

void Chunk::InitializeLighting()
{
	for (int blockIndex = 0; blockIndex < BLOCKS_PER_CHUNK; ++blockIndex)
	{
		if (BlockDefinition::s_blockDefinitions[m_blocks[blockIndex].GetBlockType()]->m_selfIlluminationValue > 0)
		{
			BlockInfo block(this, blockIndex);
			g_theApp->m_game->m_theWorld->DirtyBlockLighting(block);
		}
	}

	//Create sky
	for (int xIndex = 0; xIndex < CHUNK_X; ++xIndex)
	{
		for (int yIndex = 0; yIndex < CHUNK_Y; ++yIndex)
		{
			for (int zIndex = CHUNK_Z - 1; zIndex >= 0; --zIndex)
			{
				int blockIndex = GetBlockIndexForBlockCoords(IntVector3(xIndex, yIndex, zIndex));
				if (m_blocks[blockIndex].GetIsOpaque())
				{
					break;
				}

				m_blocks[blockIndex].SetIsSky();
				m_blocks[blockIndex].SetLightValue(SKY_LIGHT_VALUE);
			}
		}
	}

	//Dirty adjacent non-sky blocks
	for (int xIndex = 0; xIndex < CHUNK_X; ++xIndex)
	{
		for (int yIndex = 0; yIndex < CHUNK_Y; ++yIndex)
		{
			for (int zIndex = CHUNK_Z - 1; zIndex >= 0; --zIndex)
			{
				int blockIndex = GetBlockIndexForBlockCoords(IntVector3(xIndex, yIndex, zIndex));
				if (m_blocks[blockIndex].GetIsOpaque())
				{
					break;
				}

				BlockInfo blockInfo(this, blockIndex);

				BlockInfo northNeighbor = blockInfo.GetNorthBlock();
				BlockInfo southNeighbor = blockInfo.GetSouthBlock();
				BlockInfo eastNeighbor = blockInfo.GetEastBlock();
				BlockInfo westNeighbor = blockInfo.GetWestBlock();

				if(northNeighbor.GetBlock() && !northNeighbor.GetBlock()->GetIsSky() && !northNeighbor.GetBlock()->GetIsOpaque())
					g_theApp->m_game->m_theWorld->DirtyBlockLighting(northNeighbor);

				if (southNeighbor.GetBlock() && !southNeighbor.GetBlock()->GetIsSky() && !southNeighbor.GetBlock()->GetIsOpaque())
					g_theApp->m_game->m_theWorld->DirtyBlockLighting(southNeighbor);

				if (eastNeighbor.GetBlock() && !eastNeighbor.GetBlock()->GetIsSky() && !eastNeighbor.GetBlock()->GetIsOpaque())
					g_theApp->m_game->m_theWorld->DirtyBlockLighting(eastNeighbor);

				if (westNeighbor.GetBlock() && !westNeighbor.GetBlock()->GetIsSky() && !westNeighbor.GetBlock()->GetIsOpaque())
					g_theApp->m_game->m_theWorld->DirtyBlockLighting(westNeighbor);
			}
		}
	}

	//Dirty non-opaque blocks on edges
	//East-west edges
	for (int yIndex = 0; yIndex < CHUNK_Y; ++yIndex)
	{
		for (int zIndex = 0; zIndex < CHUNK_Z; ++zIndex)
		{
			BlockInfo eastEdge(this, GetBlockIndexForBlockCoords(IntVector3(CHUNK_X - 1, yIndex, zIndex)));
			BlockInfo westEdge(this, GetBlockIndexForBlockCoords(IntVector3(0, yIndex, zIndex)));

			g_theApp->m_game->m_theWorld->DirtyBlockLighting(eastEdge);
			g_theApp->m_game->m_theWorld->DirtyBlockLighting(westEdge);
		}
	}

	//North-south edges
	for (int xIndex = 0; xIndex < CHUNK_Y; ++xIndex)
	{
		for (int zIndex = 0; zIndex < CHUNK_Z; ++zIndex)
		{
			BlockInfo northEdge(this, GetBlockIndexForBlockCoords(IntVector3(xIndex, CHUNK_Y - 1, zIndex)));
			BlockInfo southEdge(this, GetBlockIndexForBlockCoords(IntVector3(xIndex, 0, zIndex)));

			g_theApp->m_game->m_theWorld->DirtyBlockLighting(northEdge);
			g_theApp->m_game->m_theWorld->DirtyBlockLighting(southEdge);
		}
	}
}

bool Chunk::SaveToFile()
{
	std::vector<unsigned char> chunkBuffer;
	CompressToRLE(chunkBuffer);
	std::string filePath = "Data/Save/Chunk_(" + std::to_string(m_chunkCoords.x) + "," + std::to_string(m_chunkCoords.y) + ").cnk";
	return WriteBufferToFile(chunkBuffer, filePath);
}

void Chunk::CompressToRLE(std::vector<unsigned char>& out_chunkBuffer)
{
	out_chunkBuffer.reserve(BLOCKS_PER_LAYER);

	out_chunkBuffer.push_back((unsigned char)g_GAME_VERSION);

	BlockType currentType = BLOCK_TYPE_STONE;
	int numCurrentBlockType = 0;
	for (int blockIndex = 0; blockIndex < BLOCKS_PER_CHUNK; blockIndex++)
	{
		if (m_blocks[blockIndex].GetBlockType() == currentType)
		{
			numCurrentBlockType++;
		}
		else
		{
			out_chunkBuffer.push_back((unsigned char)currentType);
			out_chunkBuffer.push_back((unsigned char)numCurrentBlockType);
			currentType = m_blocks[blockIndex].GetBlockType();
			numCurrentBlockType = 1;
		}

		if (numCurrentBlockType == 255)
		{
			out_chunkBuffer.push_back((unsigned char)currentType);
			out_chunkBuffer.push_back((unsigned char)numCurrentBlockType);
			numCurrentBlockType = 0;
		}
	}
}

bool Chunk::IsLocalMaxima(float* arrayValues, int indexInArray, int numValues, int xDimension) const
{
	float valueToCheck = arrayValues[indexInArray];

	int northIndex = indexInArray + xDimension;
	if (northIndex > 0 && northIndex < numValues && valueToCheck < arrayValues[northIndex])
	{
		return false;
	}

	int northEastIndex = indexInArray + xDimension + 1;
	if (northEastIndex > 0 && northEastIndex < numValues && valueToCheck < arrayValues[northEastIndex])
	{
		return false;
	}

	int eastIndex = indexInArray + 1;
	if (eastIndex > 0 && eastIndex < numValues && valueToCheck < arrayValues[eastIndex])
	{
		return false;
	}

	int southEastIndex = indexInArray - xDimension + 1;
	if (southEastIndex > 0 && southEastIndex < numValues && valueToCheck < arrayValues[southEastIndex])
	{
		return false;
	}

	int southIndex = indexInArray - xDimension;
	if (southIndex > 0 && southIndex < numValues && valueToCheck < arrayValues[southIndex])
	{
		return false;
	}

	int southWestIndex = (indexInArray - xDimension) - 1;
	if (southWestIndex > 0 && southWestIndex < numValues && valueToCheck < arrayValues[southWestIndex])
	{
		return false;
	}

	int westIndex = indexInArray - 1;
	if (westIndex > 0 && westIndex < numValues && valueToCheck < arrayValues[westIndex])
	{
		return false;
	}

	int northWestIndex = indexInArray + xDimension - 1;
	if (northWestIndex > 0 && northWestIndex < numValues && valueToCheck < arrayValues[northWestIndex])
	{
		return false;
	}

	return true;
}

void Chunk::PlaceTreeBlocks(const Vector3& worldStartPosition, TreeDefinition treeToPlace)
{
	BlockInfo currentBlock;

	for (size_t treeBlockIndex = 0; treeBlockIndex < treeToPlace.GetTreeBlocks().size(); treeBlockIndex++)
	{
		currentBlock = g_theApp->m_game->m_theWorld->GetBlockInfoFromWorldCoords(worldStartPosition + treeToPlace.GetTreeBlocks()[treeBlockIndex].offsetFromBase);
		if (currentBlock.m_chunk == this && (currentBlock.GetBlock()->GetBlockType() == BLOCK_TYPE_AIR || currentBlock.GetBlock()->GetBlockType() == BLOCK_TYPE_LEAVES))
		{
			currentBlock.ChangeType(treeToPlace.GetTreeBlocks()[treeBlockIndex].blockType);
		}
	}
}


