#pragma once
#ifndef include_CONNECTIONINITIALIZATIONSERVER
#define include_CONNECTIONINITIALIZATIONSERVER

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"

#include <string>

using namespace Poco::Net;

WebSocket* g_activeWebSocket = 0;
extern unsigned int g_imageWidthPixels; //these externs are a somewhat hacky way of doing this
extern unsigned int g_imageHeightPixels;
extern std::string g_compressionType;

class PageRequestHandler: public HTTPRequestHandler
{
public:
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		//Reads the javascript file from disk to prevent it from cluttering up the HTML excessively
		std::string javascriptFileName = "./Code/javascript/WebsocketConnection.js";
		FILE* javascriptFile = std::fopen(javascriptFileName.c_str(), "rb");
		fseek(javascriptFile, 0, SEEK_END);
		int fileSizeInBytes = (int)ftell(javascriptFile) + 1;
		rewind(javascriptFile);
		char* javascriptRaw = new char[fileSizeInBytes];
		fread(javascriptRaw, sizeof(char), fileSizeInBytes, javascriptFile);
		javascriptRaw[fileSizeInBytes-1] = 0;
		std::fclose(javascriptFile);

		//Generating the page by hand rather than reading from a file because there are a number of variables which must be set at runtime
		response.setChunkedTransferEncoding(true);
		response.setContentType("text/html");
		std::ostream& ostr = response.send();
		ostr << "<html>\n";
		ostr << "<head>\n";
		ostr << "<title>WebSocketServer</title>\n";
		ostr << "</head>\n";
		ostr << "<body>\n";
		ostr << "  <h1>Jeff Listman Thesis Proof of Concept</h1>\n";
		ostr << "	<canvas id='RenderTarget' width='"<<g_imageWidthPixels<<"' height='"<<g_imageHeightPixels<<"'></canvas>\n";
		ostr << "	<canvas id='CanvasStream' width='"<<g_imageWidthPixels<<"' height='"<<g_imageHeightPixels<<"'></canvas>\n";
		ostr << "	<img id='ImageStream' src=''></img>\n";
		ostr << "	<video id='VideoStream' src='somefile.ogv' type='video/ogg' codecs='theora' autoplay='autoplay'></video>\n"; //
		ostr << "<script type=\"text/javascript\">\n";
		ostr << "var serverAddress = \""<<request.serverAddress().toString()<<"\";\n";
		ostr << "var compressionType = \""<<g_compressionType.c_str()<<"\";\n";
		ostr << "var frameHeight = "<<g_imageHeightPixels<<";\n";
		ostr << "var frameWidth = "<<g_imageWidthPixels<<";\n";
		ostr << javascriptRaw;
		ostr << "</script>\n";
		ostr << "  <p><a href=\"javascript:WebSocketTest()\">Begin Streaming</a></p>\n";
		ostr << "</body>\n";
		ostr << "</html>\n";
	}
};


class WebSocketRequestHandler: public HTTPRequestHandler
{
public:
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		try
		{
			if(g_activeWebSocket != 0)
			{
				delete g_activeWebSocket;
			}
			g_activeWebSocket = new WebSocket(request, response);

			char buffer[1024];
			int flags;
			int n;

			n = g_activeWebSocket->receiveFrame(buffer, sizeof(buffer), flags);

		}
		catch (WebSocketException& exc)
		{
			switch (exc.code())
			{
			case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
				response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
				// fallthrough
			case WebSocket::WS_ERR_NO_HANDSHAKE:
			case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
			case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
				response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
				response.setContentLength(0);
				response.send();
				break;
			}
		}
	}
};


class RequestHandlerFactory: public HTTPRequestHandlerFactory
{
public:
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
		if(request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
		{
			return new WebSocketRequestHandler;
		}
		else
		{
			return new PageRequestHandler;
		}
	}
};

class ConnectionInitializationServer
{
public:
	ConnectionInitializationServer(int port);
	~ConnectionInitializationServer();

	HTTPServer* m_HTTPServer;
	int m_port;

	int sendFrameToClient(unsigned char* imageFrame, int imageBufferSize);
};

ConnectionInitializationServer::ConnectionInitializationServer(int port)
{
	m_port = port;
	// set-up a server socket
	ServerSocket svs(m_port);
	// set-up a HTTPServer instance
	m_HTTPServer = new HTTPServer(new RequestHandlerFactory, svs, new HTTPServerParams);
	// start the HTTPServer
	m_HTTPServer->start();
}

ConnectionInitializationServer::~ConnectionInitializationServer()
{
	m_HTTPServer->stop();
}

int ConnectionInitializationServer::sendFrameToClient(unsigned char* imageFrame, int imageBufferSize)
{
	if(g_activeWebSocket != 0)
	{
		g_activeWebSocket->sendFrame(imageFrame, imageBufferSize, WebSocket::FRAME_BINARY);
		return 0;
	}
	else
	{
		return -1;
	}
}

#endif