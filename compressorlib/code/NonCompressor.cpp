#include "NonCompressor.hpp"
#include <cstring>
using namespace std;

NonCompressor::NonCompressor()
{
	m_sizeOfFinishedBuffer = 0;
}

unsigned char* NonCompressor::compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight)
{
	int frameSize = frameWidth * frameHeight * 4;
	unsigned char* compressedFrame = new unsigned char[frameSize];
	m_sizeOfFinishedBuffer = frameSize;
	memcpy(compressedFrame, uncompressedFrame, frameSize);//HACK this is bad and disgusting
	m_currentFrameBuffer = compressedFrame;
	return compressedFrame;
}

void NonCompressor::cleanupAfterSend()
{
	delete[] m_currentFrameBuffer;
}