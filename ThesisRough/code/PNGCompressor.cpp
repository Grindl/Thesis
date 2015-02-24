#include "PNGCompressor.hpp"

#include "Internal Dependencies/lodepng.h"


PNGCompressor::PNGCompressor()
{

}


unsigned char* PNGCompressor::compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight)
{

	lodepng_encode32(&m_lastSentFrame, ((unsigned int*)&m_sizeOfFinishedBuffer), uncompressedFrame, frameWidth, frameHeight);
	return m_lastSentFrame;
}

void PNGCompressor::cleanupAfterSend()
{
	free(m_lastSentFrame);
}