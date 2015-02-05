#pragma once
#ifndef include_FRAMEPACKET
#define include_FRAMEPACKET

#include "Poco/Net/HTTPServer.h"

const int MAX_FRAME_HEIGHT = 1080;
const int MAX_FRAME_WIDTH = 1920;

class FramePacket 
{
public:
	FramePacket();

	static int s_lastFrameNum;

	char m_ID;
	int m_frameNum;
	short m_frameHeightPixels;
	short m_frameWidthPixels;
	char m_frameData[MAX_FRAME_HEIGHT*MAX_FRAME_WIDTH];
	
};

#endif