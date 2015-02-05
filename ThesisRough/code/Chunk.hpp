#pragma  once
#ifndef include_CHUNK
#define include_CHUNK

#include <map>
#include <queue>
#include <fstream>
#include <string>
#include <sstream>

#include "Data.hpp"
#include "NoiseGen.h"
//#include "RenderVector.hpp"
#include "Primitives/RenderVector.hpp"
#include "Cube.hpp"

static std::string TEXTURE_FILE = ".\\Data\\catalog.png";
static Texture* textureCatalog;
static GLuint g_textureID;

static void initTextures()
{
	textureCatalog = new Texture(TEXTURE_FILE);
	g_textureID = textureCatalog->m_openGLTextureID;
}

const int SIZEOFVBO = 100000;
const unsigned char DAYLIGHT = 15;
const unsigned char MOONLIGHT = 6;
const int SEA_LEVEL = 59;
const int MAGMA_LEVEL = 3;
const unsigned char INVALID_CHUNK_LIGHT = 255;

class Chunk
{
public:
	std::map<Vector2i, Chunk*>* m_world;
	unsigned char m_blockData[CHUNK_SIZE];
	unsigned char m_lightData[CHUNK_SIZE];
	RenderVector* m_VBO;
	std::queue<int> m_lightingUpdateQueue;
	Vector2i m_ID;
	int m_VBOOffset;
	GLuint m_OGLBuffer;
	bool m_VBOisDirty;
	bool m_hasVBO;
	bool m_needsToChangeTimeOfDay;
	bool m_markForWrite;
	unsigned char m_ambientLight;

	Chunk(Vector2i id, std::map<Vector2i, Chunk*>* world, bool isOnDisk);
	void generateRandomTerrain();
	bool readFromFile();
	void render();
	void generateLighting();
	void changeTimeOfDay(bool daytime);
	void exhaustiveGenerateLighting();
	unsigned char updateLighting(const BlockIdentifier &target, int distanceFromSource);
	void generateVBO();
	const unsigned char operator[](int index);
	const unsigned char getMaterialAtIndex(int index);
	void setMaterialAtIndex(unsigned char material, int index);
	static const int getCubeIndex(int xPos, int yPos, int zPos, const Vector2i& localChunk, Vector2i& actualChunk);
	static const BlockIdentifier getCubeIndex(int xPos, int yPos, int zPos, const Vector2i& localChunk);
	bool writeToFile();
};



#endif