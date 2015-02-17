#include "TheoraCompressor.hpp"
#include <cassert>
#include <vector>

static const int NUM_RGBA_CHANNELS = 4;

//TODO this module currently leaks memory all over the place

//----------------------------------------------------------------------------------------
 void TheoraCompressor::translateRGBAPixelsToYCC(const unsigned char* RGBAFrame, int frameWidth, int frameHeight, th_img_plane* outYCCPlanes)
{
	//assumes RGB only for the moment, no alpha
	int imageSize = frameWidth*frameHeight;

	int redOffset = 0;
	int greenOffset = 1;
	int blueOffset = 2;
	int luminanceOffset = 0;
	int chromaBlueOffset = 1;
	int chromaRedOffset = 2;

	outYCCPlanes[0].data = new unsigned char[imageSize];
	outYCCPlanes[1].data = new unsigned char[imageSize];
	outYCCPlanes[2].data = new unsigned char[imageSize];
	m_YFrame = outYCCPlanes[0].data;
	m_CbFrame = outYCCPlanes[1].data;
	m_CrFrame = outYCCPlanes[2].data;

	for(int i = 0; i < imageSize; i++)
	{
		int RGBAIndex = NUM_RGBA_CHANNELS * i;
		//uses JPEG conversion ratios, as described by http://en.wikipedia.org/wiki/YCbCr#JPEG_conversion
		outYCCPlanes[luminanceOffset].data[i] = (unsigned char)( 0 + (0.299f * RGBAFrame[RGBAIndex+redOffset]) + (0.587f * RGBAFrame[RGBAIndex+greenOffset]) + (0.114f * RGBAFrame[RGBAIndex+blueOffset]) );
		outYCCPlanes[chromaBlueOffset].data[i] = (unsigned char)( 128 - (0.168736f * RGBAFrame[RGBAIndex+redOffset]) - (0.331264f * RGBAFrame[RGBAIndex+greenOffset]) + (0.5f * RGBAFrame[RGBAIndex+blueOffset]) );
		outYCCPlanes[chromaRedOffset].data[i] = (unsigned char)( 128 + (0.5f * RGBAFrame[RGBAIndex+redOffset]) - (0.418688f * RGBAFrame[RGBAIndex+greenOffset]) - (0.081312f * RGBAFrame[RGBAIndex+blueOffset]) );
	}

}

TheoraCompressor::TheoraCompressor()
{
	th_info_init(&m_info);
	/*m_info.version_major = 1;
	m_info.version_minor = 1;
	m_info.version_subminor = 1;*/
	m_info.frame_width = SCREEN_WIDTH;
	m_info.frame_height = SCREEN_HEIGHT;
	m_info.pic_width = SCREEN_WIDTH;
	m_info.pic_height = SCREEN_HEIGHT;
	m_info.pic_x = 0 ;
	m_info.pic_y = 0;
	m_info.fps_numerator = 10;
	m_info.fps_denominator = 1;
	/*m_info.aspect_numerator = 0;
	m_info.aspect_denominator = 0;
	m_info.colorspace = TH_CS_ITU_REC_470BG; //UNSURE if this is correct TODO test this*/
	m_info.pixel_fmt = TH_PF_444;
	m_info.target_bitrate = 0; //variable bitrate
	m_info.quality = 31; //0-63, picked a middle number
	/*m_info.keyframe_granule_shift = 6; //suggested value*/

	m_state = th_encode_alloc(&m_info);

	m_nextPacket = new ogg_packet;
	m_nextPage = new ogg_page;
	m_oggStream = new ogg_stream_state;

	int errorcode = ogg_stream_init(m_oggStream, 555); //giving the stream a hardcoded serial number
	assert(errorcode == 0);
	//int errorcode = theora_encode_init(&m_state, &m_info);

	m_headerBuffer = generateHeader(m_headerBufferLength);

	int maxSpeed = 0;
	errorcode = th_encode_ctl(m_state, TH_ENCCTL_GET_SPLEVEL_MAX, &maxSpeed, sizeof(maxSpeed));
	errorcode = th_encode_ctl(m_state, TH_ENCCTL_SET_SPLEVEL, &maxSpeed, sizeof(maxSpeed));
}


unsigned char* TheoraCompressor::compressFrame(const unsigned char* uncompressedFrame, int frameWidth, int frameHeight)
{
	int errorcode;
	th_img_plane frameAsYCC[3];
	for(int i = 0; i < 3; i++)
	{
		frameAsYCC[i].width = frameWidth;
		frameAsYCC[i].height = frameHeight;
		frameAsYCC[i].stride = frameWidth;
	}
	
	translateRGBAPixelsToYCC(uncompressedFrame, frameWidth, frameHeight, frameAsYCC);

	errorcode = th_encode_ycbcr_in(m_state, frameAsYCC);
	assert(errorcode == 0);

	errorcode = th_encode_packetout(m_state, 0, m_nextPacket);
	assert(errorcode != 0);

	errorcode = ogg_stream_packetin(m_oggStream, m_nextPacket);
	assert(errorcode == 0);

	errorcode = ogg_stream_flush(m_oggStream, m_nextPage);
	assert(errorcode != 0); //this is intentional; ogg_stream_flush and ogg_stream_pageout return 0 as "insufficient data to make a page", and nonzero as success

	unsigned char* bitstream = new unsigned char[m_nextPage->body_len+m_nextPage->header_len];
	memcpy(bitstream, m_nextPage->header, m_nextPage->header_len);
	memcpy(bitstream+m_nextPage->header_len, m_nextPage->body, m_nextPage->body_len);

	/*unsigned char* bitstream = new unsigned char[m_headerBufferLength+m_nextPage->body_len+m_nextPage->header_len];
	memcpy(bitstream, m_headerBuffer, m_headerBufferLength);
	memcpy(bitstream+m_headerBufferLength, m_nextPage->header, m_nextPage->header_len);
	memcpy(bitstream+m_headerBufferLength+m_nextPage->header_len, m_nextPage->body, m_nextPage->body_len);*/

	//m_sizeOfFinishedBuffer = m_nextPacket->bytes + sizeof(m_nextPacket);
	m_sizeOfFinishedBuffer = m_nextPage->body_len+m_nextPage->header_len;

	//return (unsigned char*)&m_nextPacket;
	return bitstream;
}

void TheoraCompressor::cleanupAfterSend()
{
	/*delete[] m_YFrame;
	delete[] m_CbFrame;
	delete[] m_CrFrame;*/
}

unsigned char* TheoraCompressor::generateHeader(int& out_sizeofHeader)
{
	th_comment* theoraCommentBuffer = new th_comment;
	th_comment_init(theoraCommentBuffer);
	ogg_packet* headerPacket = new ogg_packet; //TODO this probably doesn't need to be newed off; check for later
	ogg_page* headerPage = new ogg_page;
	std::vector<unsigned char*> headerArray;
	std::vector<size_t> headerLengthArray;
	

	theoraCommentBuffer->comments = 1;
	theoraCommentBuffer->comment_lengths = new int[1]; 
	theoraCommentBuffer->comment_lengths[0] = 18;
	theoraCommentBuffer->user_comments = new char*; //TODO not sure if this needs to be heap allocated; this might be a rogue pointer
	theoraCommentBuffer->user_comments[0] = "THIS IS A COMMENT\0"; 
	theoraCommentBuffer->vendor = "LIBTHEORA";


	int hasHeaderRemaining = 1;
	int errorcode = 0;
	int headerIndex = 0;

	hasHeaderRemaining = th_encode_flushheader(m_state, theoraCommentBuffer, headerPacket); //done first outside the loop to prevent creating pages for empty headers

	while(hasHeaderRemaining != 0)
	{
		errorcode = ogg_stream_packetin(m_oggStream, headerPacket);
		assert(errorcode == 0);

		errorcode = ogg_stream_flush(m_oggStream, headerPage);
		assert(errorcode != 0); //this is intentional; ogg_stream_flush and ogg_stream_pageout return 0 as "insufficient data to make a page", and nonzero as success

		unsigned char* bitstream = new unsigned char[headerPage->body_len+headerPage->header_len];
		memcpy(bitstream, headerPage->header, headerPage->header_len);
		memcpy(bitstream+headerPage->header_len, headerPage->body, headerPage->body_len);
		headerArray.push_back(bitstream);
		headerLengthArray.push_back(headerPage->body_len+headerPage->header_len);

		hasHeaderRemaining = th_encode_flushheader(m_state, theoraCommentBuffer, headerPacket);
		headerIndex++;
	}
	
	size_t totalHeaderSize = 0;
	unsigned char* headerStream = 0;
	for(unsigned int i = 0; i < headerLengthArray.size(); i++)
	{
		totalHeaderSize += headerLengthArray[i];
	}

	if(totalHeaderSize != 0)
	{
		headerStream = new unsigned char[totalHeaderSize];

		memcpy(headerStream, headerArray[0], headerLengthArray[0]);

		for(unsigned int i = 1; i < headerArray.size(); i++) //assumes at least one header
		{
			memcpy(headerStream + headerLengthArray[i-1], headerArray[i], headerLengthArray[i]);
		}
	}
	
	out_sizeofHeader = totalHeaderSize;
	return headerStream;
}