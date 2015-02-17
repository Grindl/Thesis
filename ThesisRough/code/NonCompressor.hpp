#pragma once
#ifndef include_NONCOMPRESSOR
#define include_NONCOMPRESSOR

#include "CompressionInterface.hpp"


class NonCompressor : public CompressionInterface
{
public:
	NonCompressor();

	unsigned char* m_lastSentFrame;

	virtual unsigned char* compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight);
	virtual void cleanupAfterSend();
};


#endif