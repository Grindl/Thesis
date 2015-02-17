#pragma once
#ifndef include_COMPRESSANDSENDAPI
#define include_COMPRESSANDSENDAPI


/*
Initializes the compression system, including any necessary stream states for video compression.
Arguments:
compressionType - a null terminated string, either NONE, JPEG, or THEORA, representing the type of compression to use
imageWidthPixels - the width of the rendering frame in pixels
imageHeightPixels - the height of the rendering frame in pixels
*/
extern void InitCompressionSystem(const char* compressionType, const unsigned int imageWidthPixels, const unsigned int imageHeightPixels);


//Will be implemented at a later date.
//extern void FreeCompressionSystem();


/*
Compresses an RGBA frame in to the type specified on InitCompressionSystem.
It is not necessary to keep track of either the returned frame or the size of the frame if using
the websocket section of this API, but the data is provided for those who wish to only use the
compression section.
Arguments:
frame - a pointer to the RGBA frame to compress
out_sizeOfCompressedFrame - an out variable representing the size of the returned buffer
Returns: a pointer to a heap-allocated buffer containing the compressed frame
*/
extern unsigned char* CompressFrame(const unsigned char* frame, size_t out_sizeOfCompressedFrame);


/*
Call after the send frame to clean up heap allocated memory from CompressFrame
*/
extern void PostSentFrame();


/*
Initializes the webserver and websocket subsystem.
Must be done after initializing the compression system due to assumptions and dependencies.
Arguments:
portNumber - the port on which the web server will listen for connections and serve the webpage/websocket from
*/
extern void InitWebsocketServer(unsigned int portNumber);


/*
Sends the last compressed frame to the connected client.
InitCompressionSystem, InitWebsocketServer, and at least one CompressFrame must all be called first.
Fails silently if no client has connected yet.
*/
extern void SendFrameToClient();


#endif