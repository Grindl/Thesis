#pragma once
#ifndef include_THEORACOMPRESSOR
#define include_THEORACOMPRESSOR


#include "CompressionInterface.hpp"
#include "3rdParty/libtheora/lib/encint.h"
#include "theora/theoraenc.h"

class TheoraCompressor : public CompressionInterface
{
public:
	TheoraCompressor();

	th_enc_ctx* m_state;
	th_info m_info;
	unsigned char* m_YFrame;
	unsigned char* m_CbFrame;
	unsigned char* m_CrFrame;
	ogg_packet* m_nextPacket;
	ogg_page* m_nextPage;
	ogg_stream_state* m_oggStream;
	unsigned char* m_headerBuffer;
	int m_headerBufferLength;

	virtual unsigned char* compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight);
	void translateRGBAPixelsToYCC(const unsigned char* RGBAFrame, int frameWidth, int frameHeight, th_img_plane* outYCCPlanes);
	virtual void cleanupAfterSend();
	virtual unsigned char* generateHeader(int& out_sizeofHeader);

};

#endif