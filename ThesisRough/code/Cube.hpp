#pragma  once
#ifndef include_CUBE
#define include_CUBE

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <stdlib.h>

#include "BlockIdentifier.hpp"

#pragma comment( lib, "opengl32" )
#pragma comment( lib, "glu32" )

class CubeMaker
{
public:
	float verteces[8][3];

	CubeMaker();
	CubeMaker(const float* origin, const float* farCorner);
	const void render();
	//const void render(CUBE_MATERIAL cubeType, unsigned char sideFlags);
	void move(const float* translation);
	void setPosition(const float* position);
	void verticiesFromOriginAndFarCorner(const float* origin, const float* farCorner);
	void verticiesFromOriginAndFarCorner(const int* origin, const int* farCorner);
};


#endif