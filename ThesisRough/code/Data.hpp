#pragma once
#ifndef include_DATA
#define include_DATA

#include "Primitives/Vector2f.hpp"
#include "Rendering/Texture.hpp"

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 480;


static const float SPEED_OF_CAMERA = 10.f;
static const int LENGTGH_OF_CHUNK = 16;
static const int HEIGHT_OF_CHUNK = 128;
static const int CHUNK_SIZE = LENGTGH_OF_CHUNK*LENGTGH_OF_CHUNK*HEIGHT_OF_CHUNK;
static const int XMASK = 0xF;
static const int LENGTH_SHIFT = 4;
static const int ZSHIFT = 8;
//static const int LENGTH_IN_CHUNKS = 31;
static const int LENGTH_IN_CHUNKS = 7;
static const int NUM_CHUNKS = LENGTH_IN_CHUNKS*LENGTH_IN_CHUNKS;

static enum CUBE_MATERIAL {AIR, DIRT, STONE, WATER, GLOWSTONE, INVALID_MATERIAL};
struct cubedata
{
	bool transparent;
	float color[3];
	Vector2f texCoords; //"top right" corner TODO confirm this is true
	unsigned char minLightVal;
	unsigned char maxLightVal;
};

static const float TEXTURE_CATALOG_DIVISIONS = 1/32.f;
static const cubedata CUBE_TABLE[] = {
	{true, 1.f, 1.f, 1.f,	Vector2f(0.f*TEXTURE_CATALOG_DIVISIONS, 0.f*TEXTURE_CATALOG_DIVISIONS),		0, 15},
	{false, 1.f, 1.f, 1.f,	Vector2f(18.f*TEXTURE_CATALOG_DIVISIONS, 19.f*TEXTURE_CATALOG_DIVISIONS),	0, 0},
	{false, 1.f, 1.f, 1.f,	Vector2f(17.f*TEXTURE_CATALOG_DIVISIONS, 19.f*TEXTURE_CATALOG_DIVISIONS),	0, 0},
	{false, 1.f, 1.f, 1.f,	Vector2f(15.f*TEXTURE_CATALOG_DIVISIONS, 12.f*TEXTURE_CATALOG_DIVISIONS),	0, 0},
	{false, 1.f, 1.f, 1.f,	Vector2f(25.f*TEXTURE_CATALOG_DIVISIONS, 25.f*TEXTURE_CATALOG_DIVISIONS),	12, 12},
	{false, 1.f, 0.1f, 0.1f, Vector2f(2.f*TEXTURE_CATALOG_DIVISIONS, 0.f*TEXTURE_CATALOG_DIVISIONS),		0, 0}
};

static const char* COMPRESSION_TYPE = "JPEG";

#endif