#pragma once
#ifndef include_JPEGCOMPRESSOR
#define include_JPEGCOMPRESSOR

#include "CompressionInterface.hpp"


class JPEGCompressor : public CompressionInterface
{
public:
	JPEGCompressor();

	//returns a pointer to heap-allocated byte-stream
	virtual unsigned char* compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight);

};



#endif