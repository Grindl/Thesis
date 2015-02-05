#pragma once
#ifndef include_RENDERTOMEMORYBUFFER
#define include_RENDERTOMEMORYBUFFER

#include "Rendering/FrameBuffer.hpp"

class RenderToMemoryBuffer: public FrameBuffer
{
public:
	RenderToMemoryBuffer();

	unsigned char* m_currentFrameInMemory;

	virtual void preRenderStep();
	virtual void postRenderStep();

};

#endif