#include "NonCompressor.hpp"

NonCompressor::NonCompressor()
{

}

unsigned char* NonCompressor::compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight)
{
	int frameSize = frameWidth * frameHeight * 4;
	unsigned char* compressedFrame = new unsigned char[frameSize];
	m_sizeOfFinishedBuffer = frameSize;
	memcpy(compressedFrame, uncompressedFrame, frameSize);//HACK this is bad and disgusting
	m_lastSentFrame = compressedFrame;
	return compressedFrame;
}

void NonCompressor::cleanupAfterSend()
{
	delete[] m_lastSentFrame;
}