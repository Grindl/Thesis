#include "StatisticsArray.hpp"
#include <sstream>


StatisticsArray::StatisticsArray()
{

}

std::string StatisticsArray::toCSV()
{
	std::string CSVString = "";
	for(unsigned int nodeIndex = 0; nodeIndex < m_collectedData.size(); nodeIndex++)
	{
		std::stringstream currentNode;
		currentNode<<m_collectedData[nodeIndex].m_frameNumber<<",";
		currentNode<<m_collectedData[nodeIndex].m_compressionTime<<",";
		currentNode<<m_collectedData[nodeIndex].m_gpuTime<<",";
		currentNode<<m_collectedData[nodeIndex].m_networkTime<<",";
		currentNode<<m_collectedData[nodeIndex].m_framerateFPS<<",";
		currentNode<<m_collectedData[nodeIndex].m_frameSizeBytes<<"\n";
		CSVString.append(currentNode.str());
	}
	return CSVString;
}

StatisticsNode* StatisticsArray::calculateMaxMinAndMean()
{
	for(int i = 0; i < 3; i++)
	{
		m_maxMinAndMean[i].m_compressionTime = 0;
		m_maxMinAndMean[i].m_gpuTime = 0;
		m_maxMinAndMean[i].m_networkTime = 0;
		m_maxMinAndMean[i].m_framerateFPS = 0;
		m_maxMinAndMean[i].m_frameSizeBytes = 0;
	}

	for(unsigned int nodeIndex = 0; nodeIndex < m_collectedData.size(); nodeIndex++)
	{
		if(m_maxMinAndMean[0].m_compressionTime < m_collectedData[nodeIndex].m_compressionTime)
		{
			m_maxMinAndMean[0].m_compressionTime = m_collectedData[nodeIndex].m_compressionTime;
		}
		if(m_maxMinAndMean[1].m_compressionTime > m_collectedData[nodeIndex].m_compressionTime)
		{
			m_maxMinAndMean[1].m_compressionTime = m_collectedData[nodeIndex].m_compressionTime;
		}
		m_maxMinAndMean[2].m_compressionTime += m_collectedData[nodeIndex].m_compressionTime;

		if(m_maxMinAndMean[0].m_gpuTime < m_collectedData[nodeIndex].m_gpuTime)
		{
			m_maxMinAndMean[0].m_gpuTime = m_collectedData[nodeIndex].m_gpuTime;
		}
		if(m_maxMinAndMean[1].m_gpuTime > m_collectedData[nodeIndex].m_gpuTime)
		{
			m_maxMinAndMean[1].m_gpuTime = m_collectedData[nodeIndex].m_gpuTime;
		}
		m_maxMinAndMean[2].m_gpuTime += m_collectedData[nodeIndex].m_gpuTime;

		if(m_maxMinAndMean[0].m_networkTime < m_collectedData[nodeIndex].m_networkTime)
		{
			m_maxMinAndMean[0].m_networkTime = m_collectedData[nodeIndex].m_networkTime;
		}
		if(m_maxMinAndMean[1].m_networkTime > m_collectedData[nodeIndex].m_networkTime)
		{
			m_maxMinAndMean[1].m_networkTime = m_collectedData[nodeIndex].m_networkTime;
		}
		m_maxMinAndMean[2].m_networkTime += m_collectedData[nodeIndex].m_networkTime;

		if(m_maxMinAndMean[0].m_framerateFPS < m_collectedData[nodeIndex].m_framerateFPS)
		{
			m_maxMinAndMean[0].m_framerateFPS = m_collectedData[nodeIndex].m_framerateFPS;
		}
		if(m_maxMinAndMean[1].m_framerateFPS > m_collectedData[nodeIndex].m_framerateFPS)
		{
			m_maxMinAndMean[1].m_framerateFPS = m_collectedData[nodeIndex].m_framerateFPS;
		}
		m_maxMinAndMean[2].m_framerateFPS += m_collectedData[nodeIndex].m_framerateFPS;

		if(m_maxMinAndMean[0].m_frameSizeBytes < m_collectedData[nodeIndex].m_frameSizeBytes)
		{
			m_maxMinAndMean[0].m_frameSizeBytes = m_collectedData[nodeIndex].m_frameSizeBytes;
		}
		if(m_maxMinAndMean[1].m_frameSizeBytes > m_collectedData[nodeIndex].m_frameSizeBytes)
		{
			m_maxMinAndMean[1].m_frameSizeBytes = m_collectedData[nodeIndex].m_frameSizeBytes;
		}
		m_maxMinAndMean[2].m_frameSizeBytes += m_collectedData[nodeIndex].m_frameSizeBytes;
	}

	m_maxMinAndMean[2].m_compressionTime = m_maxMinAndMean[2].m_compressionTime / m_collectedData.size();
	m_maxMinAndMean[2].m_gpuTime = m_maxMinAndMean[2].m_gpuTime / m_collectedData.size();
	m_maxMinAndMean[2].m_networkTime = m_maxMinAndMean[2].m_networkTime / m_collectedData.size();
	m_maxMinAndMean[2].m_framerateFPS = m_maxMinAndMean[2].m_framerateFPS / m_collectedData.size();
	m_maxMinAndMean[2].m_frameSizeBytes = m_maxMinAndMean[2].m_frameSizeBytes / m_collectedData.size();

	return m_maxMinAndMean;
}