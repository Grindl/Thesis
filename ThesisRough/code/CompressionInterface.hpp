#pragma once
#ifndef include_COMPRESSIONINTERFACE
#define include_COMPRESSIONINTERFACE

#include "Data.hpp"

const int SIZE_OF_SCREEN_BUFFER = SCREEN_HEIGHT * SCREEN_WIDTH * 4;

class CompressionInterface
{
public:
	//CompressionInterface()=0;

	unsigned char* m_lastSentFrame;
	int m_sizeOfFinishedBuffer;

	virtual unsigned char* compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight)=0;
	virtual void cleanupAfterSend()=0;
};

#endif