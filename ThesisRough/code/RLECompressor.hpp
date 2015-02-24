#pragma once
#ifndef include_RLECOMPRESSOR
#define include_RLECOMPRESSOR

#include "CompressionInterface.hpp"


class RLECompressor : public CompressionInterface
{
public:
	RLECompressor();

	virtual unsigned char* compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight);
	virtual void cleanupAfterSend();
};


#endif