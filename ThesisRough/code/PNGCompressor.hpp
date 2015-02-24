#pragma once
#ifndef include_PNGCOMPRESSOR
#define include_PNGCOMPRESSOR

#include "CompressionInterface.hpp"


class PNGCompressor : public CompressionInterface
{
public:
	PNGCompressor();

	virtual unsigned char* compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight);
	virtual void cleanupAfterSend();
};


#endif