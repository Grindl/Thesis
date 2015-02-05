#include "Chunk.hpp"

#include "Rendering/OpenGLFunctions.hpp"

Chunk::Chunk(Vector2i id, std::map<Vector2i, Chunk*>* worldptr, bool isOnDisk)
{
	if(g_textureID == 0)
	{
		initTextures();
	}
	m_ID = id;
	m_world = worldptr;
	m_lightingUpdateQueue = std::queue<int>();
	m_ambientLight = DAYLIGHT;
	if(isOnDisk)
	{
		readFromFile();
	}
	else
	{
		generateRandomTerrain();
	}
	generateLighting();
	m_needsToChangeTimeOfDay = false;
	m_VBOisDirty = true;
	m_hasVBO = false;
	m_markForWrite = false;
}


void Chunk::generateRandomTerrain()
{
	for(int i = 0; i < CHUNK_SIZE; i++)
	{
		int position[3];
		position[0] = i & XMASK;
		position[1] = (i>>LENGTH_SHIFT) & XMASK;
		position[2] = i>>ZSHIFT;
		int hillVal = NoiseGen::height(m_ID.x, m_ID.y, position[0], position[1]);
		if( position[2] < hillVal && position[2] >= hillVal-4)
		{
			unsigned char cubeType = DIRT;
			m_blockData[i] = cubeType;
		}
		else if(position[2] < hillVal-4 && position[2] > MAGMA_LEVEL)
		{
			m_blockData[i] = STONE;
		}
		else if(position[2] < SEA_LEVEL && position[2] > MAGMA_LEVEL)
		{
			m_blockData[i] = WATER;
		}
		else
		{
			m_blockData[i] = AIR;
		}
	}
}

bool Chunk::readFromFile()
{
	std::fstream in;
	std::stringstream filename;

	filename << "chunks\\" << m_ID.x<<","<<m_ID.y<<".cnk\0";
	std::string filetemp = filename.str();

	//TODO: if the file is not found, we need to generate random terrain, because it's better than nothing
	in.open(filetemp, std::ios::in | std::ios::binary);
	int index = 0;
	while(!in.eof())
	{
		unsigned char type = 0;
		short count = 0;
		in.read((char*)&type, 1);
		in.read((char*)&count, 2);

		for(int originalIndex = index; index < originalIndex+count; index++)
		{
			m_blockData[index] = type;
		}
	}

	in.close();
	return true;
}

void Chunk::render()
{	
	if(m_hasVBO)
	{
		glUseProgram(0);
		glEnable(GL_DEPTH_TEST);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		glBindBuffer(GL_ARRAY_BUFFER, m_OGLBuffer);
		glBindTexture(GL_TEXTURE_2D, g_textureID);
		glVertexPointer(3, GL_FLOAT, 36, (float*)(0));
		glColorPointer(4, GL_FLOAT, 36, (float*)(12));
		glTexCoordPointer(2, GL_FLOAT, 36, (float*)(28));
		glDrawArrays(GL_QUADS, 0, m_VBOOffset);
	}
}


void Chunk::generateLighting()
{
	memset(m_lightData, 0, CHUNK_SIZE);
	for(int i = CHUNK_SIZE-1; i >= CHUNK_SIZE - 256; i--)
	{
		m_lightData[i] = m_ambientLight;
	}
	for(int i = CHUNK_SIZE - 256 - 1; i >= 0; i--)
	{
		if(CUBE_TABLE[m_blockData[i]].transparent && m_lightData[i+256]==m_ambientLight)
		{
			m_lightData[i] = m_ambientLight;
		}
	}
}

void Chunk::changeTimeOfDay(bool daytime)
{
	if(daytime && m_ambientLight != DAYLIGHT)
	{
		m_ambientLight = DAYLIGHT;
		m_needsToChangeTimeOfDay = true;
	}
	else if(!daytime && m_ambientLight != MOONLIGHT)
	{
		m_ambientLight = MOONLIGHT;
		m_needsToChangeTimeOfDay = true;
	}
	m_VBOisDirty = true;
}

void Chunk::exhaustiveGenerateLighting()
{
	memset(m_lightData, 0, CHUNK_SIZE);
	int highestObstructor = 0;
	for(int i = CHUNK_SIZE-1; i >= CHUNK_SIZE - 256; i--)
	{
		m_lightData[i] = m_ambientLight;
	}
	for(int i = CHUNK_SIZE - 256 - 1; i >= 0; i--)
	{
		int localPosition[3];
		localPosition[0] = (i & XMASK);
		localPosition[1] = ((i>>LENGTH_SHIFT) & XMASK);
		localPosition[2] = (i>>ZSHIFT);
		bool canSeeSky = true;
		for(int a = localPosition[2]; a < HEIGHT_OF_CHUNK; a++)
		{
			BlockIdentifier toCheckForTransparency = getCubeIndex(localPosition[0], localPosition[1], a, m_ID);
			bool checkedIsTransparent = CUBE_TABLE[m_world->operator[](toCheckForTransparency.homeChunk)->m_blockData[toCheckForTransparency.indexInChunk]].transparent;
			canSeeSky = canSeeSky && checkedIsTransparent;
		}
		if(canSeeSky && CUBE_TABLE[m_blockData[i]].transparent)
		{
			m_lightData[i] = m_ambientLight;
		}
		else
		{
			highestObstructor = max(highestObstructor, localPosition[2]);
		}	
	}
	//HACK this is probably incredibly inefficient
	for(int i = (highestObstructor+1)*LENGTGH_OF_CHUNK*LENGTGH_OF_CHUNK; i >= 0; i--)
	{
		BlockIdentifier blockToBeUpdated = {m_ID, i, D_NONE};
		unsigned char toBeUpdatedMaterial = m_world->operator[](blockToBeUpdated.homeChunk)->m_blockData[blockToBeUpdated.indexInChunk];
		if(CUBE_TABLE[toBeUpdatedMaterial].maxLightVal > 0)
		{
			updateLighting(blockToBeUpdated, 0);
		}

	}
	m_needsToChangeTimeOfDay = false;
	m_VBOisDirty = true;
}

unsigned char Chunk::updateLighting(const BlockIdentifier &target, int distanceFromSource)
{
	if(target.indexInChunk > 0 && target.indexInChunk < CHUNK_SIZE)
	{
		std::map< Vector2i, Chunk* >& world = *m_world;
		Chunk* chunk = world[ target.homeChunk ];
		int blockIndex = target.indexInChunk;
		unsigned char initialLight = chunk->m_lightData[ blockIndex ];
		unsigned char expectedLight = 0;
		unsigned char material = chunk->m_blockData[blockIndex];
		unsigned char minLight = CUBE_TABLE[material].minLightVal;
		unsigned char maxLight = CUBE_TABLE[material].maxLightVal;
		int localPosition[3];
		localPosition[0] = (target.indexInChunk & XMASK);
		localPosition[1] = ((target.indexInChunk>>LENGTH_SHIFT) & XMASK);
		localPosition[2] = (target.indexInChunk>>ZSHIFT);
		BlockIdentifier neighbors[SIZEOF_DIRECTION];
		neighbors[D_UP] = getCubeIndex(localPosition[0], localPosition[1], localPosition[2]+1, target.homeChunk);
		neighbors[D_DOWN] = getCubeIndex(localPosition[0], localPosition[1], localPosition[2]-1, target.homeChunk);
		neighbors[D_NORTH] = getCubeIndex(localPosition[0], localPosition[1]+1, localPosition[2], target.homeChunk);
		neighbors[D_SOUTH] = getCubeIndex(localPosition[0], localPosition[1]-1, localPosition[2], target.homeChunk);
		neighbors[D_EAST] = getCubeIndex(localPosition[0]+1, localPosition[1], localPosition[2], target.homeChunk);
		neighbors[D_WEST] = getCubeIndex(localPosition[0]-1, localPosition[1], localPosition[2], target.homeChunk);
		//check to see if I can still see the sky
		bool canSeeSky = true;
		for(int i = localPosition[2]; i < HEIGHT_OF_CHUNK; i++)
		{
			BlockIdentifier toCheckForTransparency = getCubeIndex(localPosition[0], localPosition[1], i, target.homeChunk);
			bool checkedIsTransparent = CUBE_TABLE[m_world->operator[](toCheckForTransparency.homeChunk)->m_blockData[toCheckForTransparency.indexInChunk]].transparent;
			canSeeSky = canSeeSky && checkedIsTransparent;
		}
		if(canSeeSky)
		{
			expectedLight = m_ambientLight;
		}

		for(int i = 0; i < SIZEOF_DIRECTION; i++)
		{

			if(neighbors[i].indexInChunk >= 0 && world.find(neighbors[i].homeChunk) != world.end())
			{
				Chunk* chunkOfNeighbor = world[neighbors[i].homeChunk];
				expectedLight = max(expectedLight, chunkOfNeighbor->m_lightData[neighbors[i].indexInChunk]);
			}
		}

		bool isLitBySky = canSeeSky && expectedLight == m_ambientLight;
		if(expectedLight > 0 && !isLitBySky)
		{
			expectedLight--;
		}

		expectedLight = max(expectedLight, minLight);
		expectedLight = min(expectedLight, maxLight);

		if(expectedLight != initialLight)
		{
			m_world->operator[](target.homeChunk)->m_lightData[target.indexInChunk] = expectedLight;
			m_world->operator[](target.homeChunk)->m_VBOisDirty = true;
			if(distanceFromSource < 16)
			{
				//TODO here is where I ought to be setting other vbos to dirty
				for(int recursiveIterator = 0; recursiveIterator < 6; recursiveIterator++)
				{
					if(neighbors[recursiveIterator].indexInChunk >= 0 && world.find(neighbors[recursiveIterator].homeChunk) != world.end())
					{
						Chunk* chunkOfNeighbor = world[neighbors[recursiveIterator].homeChunk];
						unsigned char neighborLight = chunkOfNeighbor->m_lightData[neighbors[recursiveIterator].indexInChunk];
						unsigned char neighborIsTransparent = CUBE_TABLE[chunkOfNeighbor->m_blockData[neighbors[recursiveIterator].indexInChunk]].transparent;
						chunkOfNeighbor->m_VBOisDirty = true;
						if(expectedLight-1 > neighborLight || initialLight-1 == neighborLight )
						{
							//recursive call
							updateLighting(neighbors[recursiveIterator], distanceFromSource+1);
						}
						else if(!canSeeSky && recursiveIterator == D_DOWN && neighborIsTransparent && neighborLight == m_ambientLight)
						{
							updateLighting(neighbors[recursiveIterator], distanceFromSource+1);
						}
					}
				}
			}
		}
		return expectedLight;
	}
	else
	{
		return INVALID_CHUNK_LIGHT;
	}
}


void Chunk::generateVBO()
{
	if(m_needsToChangeTimeOfDay)
	{
		exhaustiveGenerateLighting();
	}
	//TODO only generate a vbo if all the chunks around it are defined
	const float lightDivisor = 1.f/16.f;
	m_VBO = new RenderVector[SIZEOFVBO];
	m_VBOOffset = 0;
	for(int i = 0; i < CHUNK_SIZE; i++)
	{
		if(!CUBE_TABLE[m_blockData[i]].transparent)
		{
			int localPosition[3];
			float position[3];
			localPosition[0] = (i & XMASK);
			localPosition[1] = ((i>>LENGTH_SHIFT) & XMASK);
			localPosition[2] = (i>>ZSHIFT);
			position[0] = (float)localPosition[0];
			position[1] = (float)localPosition[1];
			position[2] = (float)localPosition[2];

			unsigned char renderFlags = 0;
			int checkIndex = 0;
			Vector2i checkChunk(0,0);
			float light = 1.f;
			//Left
			light = .9f;
			checkIndex = getCubeIndex(localPosition[0]-1, localPosition[1], localPosition[2], m_ID, checkChunk);
			if(m_world->find(checkChunk) == m_world->end())
			{
				//renderFlags = renderFlags | 8;
			}
			else if (CUBE_TABLE[m_world->operator[](checkChunk)->getMaterialAtIndex(checkIndex)].transparent)
			{
				light = (m_world->operator[](checkChunk)->m_lightData[checkIndex]+1)*lightDivisor;
				renderFlags = renderFlags | 8;
				//010
				RenderVector vert1 = {position[0], position[1]+1, position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert1;
				m_VBOOffset++;
				//000
				RenderVector vert2 = {position[0], position[1], position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f,
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert2;
				m_VBOOffset++;
				//001
				RenderVector vert3 = {position[0], position[1], position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert3;
				m_VBOOffset++;
				//011
				RenderVector vert4 = {position[0], position[1]+1, position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert4;
				m_VBOOffset++;
			}
			//Front
			light = 0.8f;
			checkIndex = getCubeIndex(localPosition[0], localPosition[1]-1, localPosition[2], m_ID, checkChunk);
			if(m_world->find(checkChunk) == m_world->end())
			{
				//renderFlags = renderFlags | 32;
			}
			else if (CUBE_TABLE[m_world->operator[](checkChunk)->getMaterialAtIndex(checkIndex)].transparent)
			{
				light = (m_world->operator[](checkChunk)->m_lightData[checkIndex]+1)*lightDivisor;
				renderFlags = renderFlags | 32;
				//000
				RenderVector vert1 = {position[0], position[1], position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert1;
				m_VBOOffset++;
				//100
				RenderVector vert2 = {position[0]+1, position[1], position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert2;
				m_VBOOffset++;
				//101
				RenderVector vert3 = {position[0]+1, position[1], position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert3;
				m_VBOOffset++;
				//001
				RenderVector vert4 = {position[0], position[1], position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert4;
				m_VBOOffset++;
			}
			//Bottom
			light = 0.5f;
			checkIndex = getCubeIndex(localPosition[0], localPosition[1], localPosition[2]-1, m_ID, checkChunk);
			if(m_world->find(checkChunk) == m_world->end() || localPosition[2] < 0)
			{
				//renderFlags = renderFlags | 4;
			}
			else if (CUBE_TABLE[m_world->operator[](checkChunk)->getMaterialAtIndex(checkIndex)].transparent)
			{
				light = (m_world->operator[](checkChunk)->m_lightData[checkIndex]+1)*lightDivisor;
				renderFlags = renderFlags | 4;
				//010
				RenderVector vert1 = {position[0], position[1]+1, position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f,
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert1;
				m_VBOOffset++;
				//110
				RenderVector vert2 = {position[0]+1, position[1]+1, position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert2;
				m_VBOOffset++;
				//100
				RenderVector vert3 = {position[0]+1, position[1], position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f,
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert3;
				m_VBOOffset++;
				//000
				RenderVector vert4 = {position[0], position[1], position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert4;
				m_VBOOffset++;
			}
			//Right
			light = 0.6f;
			checkIndex = getCubeIndex(localPosition[0]+1, localPosition[1], localPosition[2], m_ID, checkChunk);
			if(m_world->find(checkChunk) == m_world->end())
			{
				//renderFlags = renderFlags | 16;
			}
			else if (CUBE_TABLE[m_world->operator[](checkChunk)->getMaterialAtIndex(checkIndex)].transparent)
			{
				light = (m_world->operator[](checkChunk)->m_lightData[checkIndex]+1)*lightDivisor;
				renderFlags = renderFlags | 16;
				//100
				RenderVector vert1 = {position[0]+1, position[1], position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert1;
				m_VBOOffset++;
				//110
				RenderVector vert2 = {position[0]+1, position[1]+1, position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert2;
				m_VBOOffset++;
				//111
				RenderVector vert3 = {position[0]+1, position[1]+1, position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert3;
				m_VBOOffset++;
				//101
				RenderVector vert4 = {position[0]+1, position[1], position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert4;
				m_VBOOffset++;
			}
			//Far
			light = .7f;
			checkIndex = getCubeIndex(localPosition[0], localPosition[1]+1, localPosition[2], m_ID, checkChunk);
			if(m_world->find(checkChunk) == m_world->end())
			{
				//renderFlags = renderFlags | 1;
			}
			else if (CUBE_TABLE[m_world->operator[](checkChunk)->getMaterialAtIndex(checkIndex)].transparent)
			{
				light = (m_world->operator[](checkChunk)->m_lightData[checkIndex]+1)*lightDivisor;
				renderFlags = renderFlags | 1;
				//110
				RenderVector vert1 = {position[0]+1, position[1]+1, position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert1;
				m_VBOOffset++;
				//010
				RenderVector vert2 = {position[0], position[1]+1, position[2], 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert2;
				m_VBOOffset++;
				//011
				RenderVector vert3 = {position[0], position[1]+1, position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert3;
				m_VBOOffset++;
				//111
				RenderVector vert4 = {position[0]+1, position[1]+1, position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f,
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert4;
				m_VBOOffset++;
			}

			//Top
			light = 1.f;
			checkIndex = getCubeIndex(localPosition[0], localPosition[1], localPosition[2]+1, m_ID, checkChunk);
			if(m_world->find(checkChunk) == m_world->end() || localPosition[2] >= 255)
			{
				//renderFlags = renderFlags | 2;
			}
			else if (CUBE_TABLE[m_world->operator[](checkChunk)->getMaterialAtIndex(checkIndex)].transparent)
			{
				light = (m_world->operator[](checkChunk)->m_lightData[checkIndex]+1)*lightDivisor;
				renderFlags = renderFlags | 2;
				//001
				RenderVector vert1 = {position[0], position[1], position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert1;
				m_VBOOffset++;
				//101
				RenderVector vert2 = {position[0]+1, position[1], position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f, 
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y+TEXTURE_CATALOG_DIVISIONS};
				m_VBO[m_VBOOffset] = vert2;
				m_VBOOffset++;
				//111
				RenderVector vert3 = {position[0]+1, position[1]+1, position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f,
					CUBE_TABLE[m_blockData[i]].texCoords.x+TEXTURE_CATALOG_DIVISIONS, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert3;
				m_VBOOffset++;
				//011
				RenderVector vert4 = {position[0], position[1]+1, position[2]+1, 
					CUBE_TABLE[m_blockData[i]].color[0]*light, CUBE_TABLE[m_blockData[i]].color[1]*light, CUBE_TABLE[m_blockData[i]].color[2]*light, 1.f,
					CUBE_TABLE[m_blockData[i]].texCoords.x, CUBE_TABLE[m_blockData[i]].texCoords.y};
				m_VBO[m_VBOOffset] = vert4;
				m_VBOOffset++;
			}
		}
	}
	glGenBuffers(1, &m_OGLBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_OGLBuffer);
	glBufferData(GL_ARRAY_BUFFER, 36*m_VBOOffset, m_VBO, GL_STATIC_DRAW);
	delete m_VBO;
	m_VBOisDirty = false;
	m_hasVBO = true;
}


const unsigned char Chunk::operator[](int index)
{
	return m_blockData[index];
}

const unsigned char Chunk::getMaterialAtIndex(int index)
{
	return m_blockData[index];
}

void Chunk::setMaterialAtIndex(unsigned char material, int index)
{
	unsigned char previousMaterial = m_blockData[index];
	m_blockData[index] = material;
	if(CUBE_TABLE[previousMaterial].transparent != CUBE_TABLE[material].transparent )
	{
		BlockIdentifier starting = {m_ID, index, D_NONE};
		updateLighting(starting, 0);
	}

	if(previousMaterial != material)
	{
		m_VBOisDirty = true;
	}
}

const int Chunk::getCubeIndex(int xPos, int yPos, int zPos, const Vector2i& localChunk, Vector2i& actualChunk)
{
	int indexInChunk = -1;
	actualChunk = localChunk;
	while(xPos >= LENGTGH_OF_CHUNK)
	{
		actualChunk.x += 1;
		xPos -= LENGTGH_OF_CHUNK;
	}
	while(xPos < 0)
	{
		actualChunk.x -= 1;
		xPos += LENGTGH_OF_CHUNK;
	}
	while(yPos >= LENGTGH_OF_CHUNK)
	{
		actualChunk.y += 1;
		yPos -= LENGTGH_OF_CHUNK;
	}
	while(yPos < 0)
	{
		actualChunk.y -= 1;
		yPos += LENGTGH_OF_CHUNK;
	}

	if(zPos >= 0 && zPos < HEIGHT_OF_CHUNK)
	{
		indexInChunk = zPos*LENGTGH_OF_CHUNK*LENGTGH_OF_CHUNK+yPos*LENGTGH_OF_CHUNK+xPos;
	}
	return indexInChunk;
}

const BlockIdentifier Chunk::getCubeIndex(int xPos, int yPos, int zPos, const Vector2i& localChunk)
{
	Vector2i temp(0,0);
	int index = getCubeIndex(xPos, yPos, zPos, localChunk, temp);
	BlockIdentifier val = {temp, index, D_NONE};
	return val;
}


bool Chunk::writeToFile()
{
	std::fstream out;
	std::stringstream filename;

	filename << "chunks\\" << m_ID.x<<","<<m_ID.y<<".cnk\0";
	std::string filetemp = filename.str();

	out.open(filetemp, std::ios::out | std::ios::binary | std::ios::trunc);

	//do some writing
	int index = 1;
	while(index < CHUNK_SIZE)
	{
		//this looks kind of ugly, probably a better, safer way to do it
		out.write((const char*)&m_blockData[index-1], 1);
		short sequence = 1;
		while(m_blockData[index-1] == m_blockData[index] && index < CHUNK_SIZE)
		{
			index++;
			sequence++;
		}

		out.write((const char*)&sequence, 2);
		index++;
	}

	//to write our the last block; it ought to be just air, but you never know
	out.write((const char*)&m_blockData[index-1], 1);
	short temp = 1;
	out.write((const char*)&temp, 2);


	out.close();
	return true;
}