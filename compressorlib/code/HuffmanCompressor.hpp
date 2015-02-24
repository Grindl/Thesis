#pragma once
#ifndef include_HUFFMANCOMPRESSOR
#define include_HUFFMANCOMPRESSOR

#include "CompressionInterface.hpp"
#include <map>

struct HuffmanNode
{
	unsigned short code; //this assumes no code will be more than twice as long as its original
	unsigned char length;

	bool operator < (const HuffmanNode& other) const 
	{
		return code < other.code;
	}
};

class HuffmanCompressor : public CompressionInterface
{
public:
	HuffmanCompressor();
	std::map<unsigned char, HuffmanNode> m_huffmanTable;


	virtual unsigned char* compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight);
	virtual void cleanupAfterSend();

	void generateTableFromImage(const unsigned char* uncompressedFrame, const int frameSize);
};


#endif