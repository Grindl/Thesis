#pragma once
#ifndef include_NONCOMPRESSOR
#define include_NONCOMPRESSOR

#include "CompressionInterface.hpp"


class NonCompressor : public CompressionInterface
{
public:
	NonCompressor();


	virtual unsigned char* compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight);
	virtual void cleanupAfterSend();
};


#endif