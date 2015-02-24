#include "CompressAndSendAPI.hpp"

#include "TheoraCompressor.hpp"
#include "JPEGCompressor.hpp"
#include "RLECompressor.hpp"
#include "PNGCompressor.hpp"
#include "NonCompressor.hpp"
#include "ConnectionInitializationServer.hpp"

#include <cstring>
#include <string>

CompressionInterface* g_compressor = 0;
unsigned int g_imageWidthPixels = 0;
unsigned int g_imageHeightPixels = 0;
bool g_hasInitializedConnection = false;
ConnectionInitializationServer* g_websocketServer = 0;
std::string g_compressionType;


void InitCompressionSystem(const char* compressionType, const unsigned int imageWidthPixels, const unsigned int imageHeightPixels)
{
	g_compressionType = compressionType;
	g_imageWidthPixels = imageWidthPixels;
	g_imageHeightPixels = imageHeightPixels;

	if(strcmp(compressionType, "NONE") == 0)
	{
		g_compressor = new NonCompressor();
	}
	else if(strcmp(compressionType, "JPEG") == 0)
	{
		g_compressor = new JPEGCompressor();
	}
	else if(strcmp(compressionType, "PNG") == 0)
	{
		g_compressor = new PNGCompressor();
	}
	else if(strcmp(compressionType, "RLE") == 0)
	{
		g_compressor = new RLECompressor();
	}
	else if(strcmp(compressionType, "THEORA") == 0)
	{
		g_compressor = new TheoraCompressor();
	}
	else
	{
		g_compressor = new NonCompressor(); //semi-hackey fallback
	}
}


unsigned char* CompressFrame(const unsigned char* frame, size_t out_sizeOfCompressedFrame)
{
	unsigned char* compressed = g_compressor->compressFrame(frame, g_imageWidthPixels, g_imageHeightPixels);
	out_sizeOfCompressedFrame = g_compressor->m_sizeOfFinishedBuffer;
	return compressed;
}

void PostSentFrame()
{
	g_compressor->cleanupAfterSend();
}

void InitWebsocketServer(unsigned int portNumber)
{
	g_websocketServer = new ConnectionInitializationServer(portNumber);
}

void SendFrameToClient()
{
	g_websocketServer->sendFrameToClient(g_compressor->m_currentFrameBuffer, g_compressor->m_sizeOfFinishedBuffer);
}

int ReceiveFrameFromClient(unsigned char* out_receivedFrame, size_t maxSizeOfReceivedFrame)
{
	int numReceivedBytes = g_websocketServer->pollFrameFromClient(out_receivedFrame, maxSizeOfReceivedFrame);
	return numReceivedBytes;
}