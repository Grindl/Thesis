#include "HuffmanCompressor.hpp"


unsigned char* HuffmanCompressor::compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight)
{
	int imageSize = frameWidth * frameHeight * 4;
	generateTableFromImage(uncompressedFrame, imageSize);
	unsigned char* outBuffer = new unsigned char[imageSize];
	unsigned int headerOffset = 0;
	unsigned int bufferIndex = headerOffset;
	unsigned int octetIndex = 0;
	//TODO insert the table as a header
	//TODO skip the alpha channel
	for (int i = 0; i < imageSize; i++)
	{
		unsigned int huffmanCode = m_huffmanTable[uncompressedFrame[i]].code;
		unsigned int huffmanLength = m_huffmanTable[uncompressedFrame[i]].length;
		for(unsigned int encodingIndex = 0; encodingIndex < huffmanLength; encodingIndex++)
		{
			if(octetIndex == 8)
			{
				octetIndex = 0;
				bufferIndex++;
			}
			unsigned int byteToEncode = (huffmanCode >> encodingIndex) & 0x1;
			unsigned char bytePosition = byteToEncode << octetIndex;
			outBuffer[bufferIndex] += bytePosition;
			octetIndex++;

		}
	}
	m_currentFrameBuffer = outBuffer;
	m_sizeOfFinishedBuffer = bufferIndex + 1; //possible incomplete bytes, blegh
	return outBuffer;
}

void HuffmanCompressor::generateTableFromImage(const unsigned char* uncompressedFrame, const int frameSize)
{
	//run through the entire image, counting probabilities in an array
	unsigned int pixelCount[256];
	for(int frameIndex = 0; frameIndex < frameSize; frameIndex++)
	{
		pixelCount[uncompressedFrame[frameIndex]]++;
	}
	//TODO then weight the probabilities in a tree
	//then insert the codes and lengths in to the map
}