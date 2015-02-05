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

#include "Data.hpp"

using namespace Poco::Net;

WebSocket* g_activeWebSocket = 0;

class PageRequestHandler: public HTTPRequestHandler
	/// Return a HTML document with some JavaScript creating
	/// a WebSocket connection.
{
public:
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		std::string javascriptFileName = "./Code/javascript/WebsocketConnection.js";
		FILE* javascriptFile = std::fopen(javascriptFileName.c_str(), "rb");
		fseek(javascriptFile, 0, SEEK_END);
		int fileSizeInBytes = (int)ftell(javascriptFile) + 1;
		rewind(javascriptFile);
		char* javascriptRaw = new char[fileSizeInBytes];
		fread(javascriptRaw, sizeof(char), fileSizeInBytes, javascriptFile);
		javascriptRaw[fileSizeInBytes-1] = 0;
		std::fclose(javascriptFile);

		response.setChunkedTransferEncoding(true);
		response.setContentType("text/html");
		std::ostream& ostr = response.send();
		ostr << "<html>\n";
		ostr << "<head>\n";
		ostr << "<title>WebSocketServer</title>\n";
		ostr << "</head>\n";
		ostr << "<body>\n";
		ostr << "  <h1>Jeff Listman Thesis Proof of Concept</h1>\n";
		ostr << "	<canvas id='CanvasStream' width='640' height='480'></canvas>\n";
		ostr << "	<img id='ImageStream' src=''>\n";
		ostr << "<script type=\"text/javascript\">\n";
		ostr << "var serverAddress = \""<<request.serverAddress().toString()<<"\";\n";
		ostr << "var compressionType = \""<<COMPRESSION_TYPE<<"\";\n";//HACK, this should be based on a const or parameter for ease
		ostr << javascriptRaw;
		ostr << "</script>\n";
		ostr << "  <p><a href=\"javascript:WebSocketTest()\">Begin Streaming</a></p>\n";
		ostr << "</body>\n";
		ostr << "</html>\n";
	}
};


class WebSocketRequestHandler: public HTTPRequestHandler
	/// Handle a WebSocket connection.
{
public:
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		//Application& app = Application::instance();
		try
		{
			if(g_activeWebSocket != 0)
			{
				delete g_activeWebSocket;
			}
			g_activeWebSocket = new WebSocket(request, response);
			//WebSocket ws(request, response);
			//app.logger().information("WebSocket connection established.");
			char buffer[1024];
			int flags;
			int n;
			//do
			//{
				n = g_activeWebSocket->receiveFrame(buffer, sizeof(buffer), flags);
				//g_activeWebSocket->sendFrame(buffer, n, flags);
			//}
			//while (n > 0 || (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
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
			return new WebSocketRequestHandler;
		else
			return new PageRequestHandler;
	}
};

class ConnectionInitializationServer
{
public:
	ConnectionInitializationServer();
	~ConnectionInitializationServer();

	HTTPServer* m_HTTPServer;
	int m_port;

	int sendFrameToClient(unsigned char* imageFrame, int imageBufferSize);
};

ConnectionInitializationServer::ConnectionInitializationServer()
{
	m_port = 8080;
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