#pragma once
#ifndef include_STATISTICSNODE
#define include_STATISTICSNODE

struct StatisticsNode
{
	int m_frameNumber;
	float m_compressionTime;
	float m_gpuTime;
	float m_networkTime;
	float m_framerateFPS;
	int m_frameSizeBytes;
};

#endif