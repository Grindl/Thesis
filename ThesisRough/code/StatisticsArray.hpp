#pragma once
#ifndef include_STATISTICSARRAY
#define include_STATISTICSARRAY

#include "StatisticsNode.hpp"
#include <vector>
#include <string>

class StatisticsArray
{
public:
	StatisticsArray();

	std::vector<StatisticsNode> m_collectedData;
	StatisticsNode m_maxMinAndMean[3];

	std::string toCSV();
	StatisticsNode* calculateMaxMinAndMean();
};



#endif