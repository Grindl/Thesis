#include "RLECompressor.hpp"
#include <Winsock2.h>

RLECompressor::RLECompressor()
{
	//no initialization needed
}

unsigned char* RLECompressor::compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight)
{
	//let's make some assumptions
	//first, we're reading in at 24 bits, but we could probably get away with 15 bits
	//second, we can RLE each channel independently, and it will allow us to gloss over certain boundaries
	//third, we can use the first two ints as a header for when the other two channels start
	//fourth, we can bitpack the number of consecutive pixels in to those last three bits
	//fifth, we can use 0x7 as a denotation that the next char is a count, not a new pixel
	unsigned char significanceMask = 0xf8;
	int imageSize = frameWidth*frameHeight;
	unsigned char* compressedBuffer = new unsigned char[imageSize*4];

	int currentBufferIndex = 8;//first 8 bytes for channel positions

	//TODO loop this for all three channels
	for(int colorIndex = 0; colorIndex < 3; colorIndex++)
	{
		unsigned char lastPixel = uncompressedFrame[0 + colorIndex] & significanceMask;
		unsigned char currentRunCount = 0;
		for(int i = 0; i < imageSize; i++)
		{
			unsigned char currentPixel = uncompressedFrame[i*4 + colorIndex];
			currentPixel = currentPixel & significanceMask;
			if (currentPixel == lastPixel && currentRunCount < 255) //we're writing to a max of one char for length
			{
				currentRunCount++;
			}
			else
			{
				//write the last pixel and run count in to the buffer
				if(currentRunCount < 7)
				{
					unsigned char RLEbyte = lastPixel + currentRunCount;
					compressedBuffer[currentBufferIndex] = RLEbyte;
					currentBufferIndex++;
				}
				else
				{
					unsigned char colorByte = lastPixel + 0x7;
					unsigned char countByte = currentRunCount;
					compressedBuffer[currentBufferIndex] = colorByte;
					compressedBuffer[currentBufferIndex+1] = countByte;
					currentBufferIndex += 2;
				}
				currentRunCount = 1;
				lastPixel = currentPixel;
			}
		}
		//write the last run to the buffer now
		/*if(currentRunCount < 7)
		{
			unsigned char RLEbyte = lastPixel + currentRunCount;
			compressedBuffer[currentBufferIndex] = RLEbyte;
			currentBufferIndex++;
		}
		else
		{
			unsigned char colorByte = lastPixel + 0x7;
			unsigned char countByte = currentRunCount;
			compressedBuffer[currentBufferIndex] = colorByte;
			compressedBuffer[currentBufferIndex+1] = countByte;
			currentBufferIndex += 2;
		}*/

		if(colorIndex == 0)
		{
			((int*)compressedBuffer)[0] = currentBufferIndex;
		}
		else if(colorIndex == 1)
		{
			((int*)compressedBuffer)[1] = currentBufferIndex;
		}
	}
	m_sizeOfFinishedBuffer = currentBufferIndex;
	m_lastSentFrame = compressedBuffer;
	return compressedBuffer;
}

void RLECompressor::cleanupAfterSend()
{
	delete[] m_lastSentFrame;
}