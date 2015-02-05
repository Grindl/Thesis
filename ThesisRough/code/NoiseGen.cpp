#include "NoiseGen.h"

int NoiseGen::randVal(int x, int y)
{
	/*
	srand((x+17)*(y+23)+x+7919);
	return rand()%360;
	/*/
	int n = x + y * 57; 
	n = (n<<13) ^ n;    
	return (int)(( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0)*360);
	//*/
}

//TODO include another larger octave with the same amplitude as major
int NoiseGen::height(int chunkx, int chunky, int localx, int localy)
{
	int majorCorners[] = {randVal((chunkx & (MAXINT - 1)), (chunky & (MAXINT - 1))),
						randVal((chunkx & (MAXINT - 1))+2, (chunky & (MAXINT - 1))),
						randVal((chunkx & (MAXINT - 1))+2, (chunky & (MAXINT - 1))+2),
						randVal((chunkx & (MAXINT - 1)), (chunky & (MAXINT - 1))+2)};

	int minorCorners[] = {randVal(chunkx, chunky),
						randVal(chunkx+1, chunky),
						randVal(chunkx+1, chunky+1),
						randVal(chunkx, chunky+1)};

	int leastCorners[] = {randVal(chunkx*2+(localx>>3), chunky*2+(localy>>3)),
						randVal(chunkx*2+(localx>>3)+1, chunky*2+(localy>>3)),
						randVal(chunkx*2+(localx>>3)+1, chunky*2+(localy>>3)+1),
						randVal(chunkx*2+(localx>>3), chunky*2+(localy>>3)+1)};

	float leastHeight =	cos((float)leastCorners[2]*PI/180.f)*(localx&7)*(localy&7)*(1/64.f) +
						cos((float)leastCorners[3]*PI/180.f)*(7-(localx&7))*(localy&7)*(1/64.f) +
						cos((float)leastCorners[0]*PI/180.f)*(7-(localx&7))*(7-(localy&7))*(1/64.f) +
						cos((float)leastCorners[1]*PI/180.f)*(localx&7)*(7-(localy&7))*(1/64.f);

	float minorHeight =	cos((float)minorCorners[2]*PI/180.f)*localx*localy*(1/256.f) +
						cos((float)minorCorners[3]*PI/180.f)*(15-localx)*localy*(1/256.f) +
						cos((float)minorCorners[0]*PI/180.f)*(15-localx)*(15-localy)*(1/256.f) +
						cos((float)minorCorners[1]*PI/180.f)*localx*(15-localy)*(1/256.f);
											  
	float majorHeight =	cos((float)majorCorners[2]*PI/180.f)*(localx + LENGTGH_OF_CHUNK*(chunkx&1))*(localy + LENGTGH_OF_CHUNK*(chunky&1))*(1/1024.f) +
						cos((float)majorCorners[3]*PI/180.f)*(15-localx + LENGTGH_OF_CHUNK*(1-(chunkx&1)))*(localy + LENGTGH_OF_CHUNK*(chunky&1))*(1/1024.f) +
						cos((float)majorCorners[0]*PI/180.f)*(15-localx + LENGTGH_OF_CHUNK*(1-(chunkx&1)))*(15-localy + LENGTGH_OF_CHUNK*(1-(chunky&1)))*(1/1024.f) +
						cos((float)majorCorners[1]*PI/180.f)*(localx + LENGTGH_OF_CHUNK*(chunkx&1))*(15-localy + LENGTGH_OF_CHUNK*(1-(chunky&1)))*(1/1024.f);

	return (int)(leastHeight*2 + minorHeight*4 + majorHeight*8 +64);
	//return (int)(majorHeight*10+20);
	//return (int)(minorHeight*10 + 20);
	//return (int)(leastHeight*10 +20);
}
