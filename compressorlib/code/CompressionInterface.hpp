#pragma once
#ifndef include_COMPRESSIONINTERFACE
#define include_COMPRESSIONINTERFACE


class CompressionInterface
{
public:
	//CompressionInterface()=0;

	int m_sizeOfFinishedBuffer;
	unsigned char* m_currentFrameBuffer;

	virtual unsigned char* compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight)=0;
	virtual void cleanupAfterSend()=0;
};

#endif