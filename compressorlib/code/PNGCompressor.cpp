#include "PNGCompressor.hpp"

#include "3rdParty/lodepng.h"


PNGCompressor::PNGCompressor()
{

}


unsigned char* PNGCompressor::compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight)
{

	lodepng_encode32(&m_currentFrameBuffer, ((unsigned int*)&m_sizeOfFinishedBuffer), uncompressedFrame, frameWidth, frameHeight);
	return m_currentFrameBuffer;
}

void PNGCompressor::cleanupAfterSend()
{
	free(m_currentFrameBuffer);
}