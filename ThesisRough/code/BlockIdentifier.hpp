#pragma once
#ifndef include_BLOCKIDENTIFIER
#define include_BLOCKIDENTIFIER

//#include "Vector2i.hpp"
#include "Primitives/Vector2i.hpp"

enum DIRECTIONS {D_NONE = -1, D_UP, D_DOWN, D_NORTH, D_SOUTH, D_EAST, D_WEST, SIZEOF_DIRECTION};

struct BlockIdentifier
{
public:
	Vector2i homeChunk;
	int indexInChunk;
	DIRECTIONS side;
};
#endif