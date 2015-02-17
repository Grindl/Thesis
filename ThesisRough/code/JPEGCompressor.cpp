#include "Internal Dependencies/jpge.h"


#include "JPEGCompressor.hpp"

JPEGCompressor::JPEGCompressor()
{
	m_sizeOfFinishedBuffer = SIZE_OF_SCREEN_BUFFER;
}


unsigned char* JPEGCompressor::compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight)
{
	m_sizeOfFinishedBuffer = SIZE_OF_SCREEN_BUFFER;
	unsigned char* compressedFrame = new unsigned char[m_sizeOfFinishedBuffer]; //assumes at least 4x compression
	jpge::compress_image_to_jpeg_file_in_memory(compressedFrame, m_sizeOfFinishedBuffer, frameWidth, frameHeight, 4, uncompressedFrame);
	m_lastSentFrame = compressedFrame;
	return compressedFrame;
}

void JPEGCompressor::cleanupAfterSend()
{
	delete[] m_lastSentFrame;
}