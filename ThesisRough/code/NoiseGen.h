#pragma once
#ifndef include_NOISEGEN
#define include_NOISEGEN

#include <math.h>
#include <time.h>
#include "Data.hpp"

class NoiseGen
{
public:
	static int randVal(int x, int y);
	static int height(int chunkx, int chunky, int localx, int localy);

};



#endif