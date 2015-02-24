#include "Rendering\OpenGLFunctions.hpp"
#include "Rendering\Texture.hpp"
#include "Debug Graphics\DebugGraphics.hpp"
#include "Systems\Player.hpp"
#include "Rendering\Skybox.hpp"
#include "Console\FontRenderer.hpp"
#include "Data.hpp"
#include "GameMain.hpp"
#include "Utility\XMLParser.hpp"
#include "Event System\EventSystemHelper.hpp"
#include "Entity.hpp"
#include "Connection.hpp"
#include "Primitives/Color3b.hpp"
#include "Utility/StringUtility.hpp"
#include "Console\CommandParser.hpp"
#include "RenderToMemoryBuffer.hpp"
#include "ConnectionInitializationServer.hpp"
#include "Debug Tools/ProfileSection.hpp"
#include "JPEGCompressor.hpp"
#include "NonCompressor.hpp"
#include "TheoraCompressor.hpp"
#include "RLECompressor.hpp"
#include "PNGCompressor.hpp"

Connection* g_serverConnection;
ConnectionInitializationServer* g_websocketServer;
float g_timeSinceLastSentFrame = 0.f;
int g_frameNumber = 0;
StatisticsArray g_statistics;

bool g_hasSentHeader = false;

bool WriteStatsToFile(std::string args)
{
	std::string outData = g_statistics.toCSV();
	FILE* fileOut = fopen(args.c_str(), "wb");
	fwrite(outData.c_str(), outData.length(), 1, fileOut);
	fclose(fileOut);
	//TODO use the args for the filename and write this out to a .csv file
	return true;
}

void GameMain::mouseUpdate()
{
	//TODO this is all very WIN32, abstract this in to the IOHandler
	if(m_IOHandler.m_hasFocus)
	{
		POINT p;
		GetCursorPos(&p);
		int x = 200 - p.x;
		int y = 200 - p.y;
		m_worldCamera.m_orientationDegrees.x += x*.15f;
		m_worldCamera.m_orientationDegrees.y += y*.15f;
		SetCursorPos(200, 200);
	}
	glColor3f(1.0f,1.0f,1.0f);
}

GameMain::GameMain()
{
	
	m_renderer = Renderer();
	//delete m_renderer.m_singleFrameBuffer; HACK memory leak
	m_renderer.m_singleFrameBuffer = new RenderToMemoryBuffer();
	m_internalTime = 0.f;
	m_isQuitting = m_renderer.m_fatalError;
	m_console.m_log = ConsoleLog();
	//m_world = World();
	m_worldCamera = Camera();
	m_worldCamera.m_cameraMode = PERSPECTIVE;
	m_worldCamera.m_position = Vector3f(0.f, 0.f, 80.f);

	m_displayConsole = false;

	//HACK test values
	m_console.m_log.appendLine("This is a test of the emergency broadcast system");
	m_console.m_log.appendLine("Do not be alarmed or concerned");
	m_console.m_log.appendLine("This is only a test");

	CommandParser::RegisterCommand("writestats", WriteStatsToFile);

	UnitTestXMLParser(".\\Data\\UnitTest.xml");
	unitTestEventSystem();

	initTextures();

	//TODO tie this directly to LENGTH_IN_CHUNKS
	//m_bottomLeftChunk = Vector2i(-15, -15);
	//m_topRightChunk = Vector2i(16, 16);
	m_bottomLeftChunk = Vector2i(-3, -3);
	m_topRightChunk = Vector2i(4, 4);

	for(int chunkIterX = m_bottomLeftChunk.x-1; chunkIterX <m_topRightChunk.x+1; chunkIterX++)
	{
		for(int chunkIterY = m_bottomLeftChunk.y-1; chunkIterY <m_topRightChunk.y+1; chunkIterY++)
		{
			Vector2i chunkID(chunkIterX , chunkIterY);
			//TODO: if I ever want to actually save the world, I need to redo this
			Chunk* temp = new Chunk(chunkID, &m_chunkMap, false);
			m_chunkMap[chunkID] = temp;
		}
	}
	for(int chunkIterX = m_bottomLeftChunk.x; chunkIterX <m_topRightChunk.x; chunkIterX++)
	{
		for(int chunkIterY = m_bottomLeftChunk.y; chunkIterY <m_topRightChunk.y; chunkIterY++)
		{
			Vector2i chunkID(chunkIterX , chunkIterY);
			m_chunkMap[chunkID]->m_VBOisDirty = true;
		}
	}


	g_websocketServer = new ConnectionInitializationServer();

	if(COMPRESSION_TYPE == "NONE")
	{
		m_compressor = new NonCompressor();
	}
	else if(COMPRESSION_TYPE == "RLE")
	{
		m_compressor = new RLECompressor();
	}
	else if(COMPRESSION_TYPE == "JPEG")
	{
		m_compressor = new JPEGCompressor();
	}
	else if(COMPRESSION_TYPE == "PNG")
	{
		m_compressor = new PNGCompressor();
	}
	else if(COMPRESSION_TYPE == "THEORA")
	{
		m_compressor = new TheoraCompressor();
	}
	else
	{
		m_compressor = new NonCompressor(); //semi-hackey fallback
	}
}

void GameMain::update(float deltaTime)
{
	StatisticsNode lastFrameNode;
	lastFrameNode.m_compressionTime = ProfileSection::s_accumulatedTimeInSections["Compression"];
	lastFrameNode.m_frameNumber = g_frameNumber;
	lastFrameNode.m_framerateFPS = 1.f/deltaTime;
	lastFrameNode.m_gpuTime = ProfileSection::s_accumulatedTimeInSections["GPU"];
	lastFrameNode.m_networkTime = ProfileSection::s_accumulatedTimeInSections["Network"];

	ProfileSection::Preframe();

	bool forwardVelocity = m_IOHandler.m_keyIsDown['W'];
	bool backwardVelocity = m_IOHandler.m_keyIsDown['S'];
	bool leftwardVelocity = m_IOHandler.m_keyIsDown['A'];
	bool rightwardVelocity = m_IOHandler.m_keyIsDown['D'];

	//HACK
	const float SPEED_OF_CAMERA = 5.f;

	m_worldCamera.m_position.x += (cos(m_worldCamera.m_orientationDegrees.x*(PI/180.f))*(rightwardVelocity - leftwardVelocity)-sin(m_worldCamera.m_orientationDegrees.x*(PI/180.f))*(forwardVelocity - backwardVelocity))*SPEED_OF_CAMERA*deltaTime;
	m_worldCamera.m_position.y += (sin(m_worldCamera.m_orientationDegrees.x*(PI/180.f))*(rightwardVelocity - leftwardVelocity)+cos(m_worldCamera.m_orientationDegrees.x*(PI/180.f))*(forwardVelocity - backwardVelocity))*SPEED_OF_CAMERA*deltaTime;
	m_worldCamera.m_position.z += ((m_IOHandler.m_keyIsDown[VK_SHIFT] - m_IOHandler.m_keyIsDown[VK_CONTROL])*SPEED_OF_CAMERA)*deltaTime;

	m_worldCamera.m_orientationDegrees.x += (m_IOHandler.m_keyIsDown[VK_LEFT] - m_IOHandler.m_keyIsDown[VK_RIGHT])*180.f*deltaTime;
	m_worldCamera.m_orientationDegrees.y += (m_IOHandler.m_keyIsDown[VK_UP] - m_IOHandler.m_keyIsDown[VK_DOWN])*180.f*deltaTime; 

	mouseUpdate();

	m_internalTime += deltaTime;

	for (unsigned int ii = 0; ii < m_elements.size(); ii++)
	{
		m_elements[ii]->update(deltaTime);
	}


	ProfileSection::StartProfile("Compression");
	//TODO this allocates new memory and some of it is unused
	unsigned char* compressedFrame = m_compressor->compressFrame(((RenderToMemoryBuffer*)m_renderer.m_singleFrameBuffer)->m_currentFrameInMemory, SCREEN_WIDTH, SCREEN_HEIGHT);
	ProfileSection::StopProfile("Compression");


	g_timeSinceLastSentFrame+= deltaTime;

	ProfileSection::StartProfile("Network");
	if(g_activeWebSocket != 0 && !g_hasSentHeader && COMPRESSION_TYPE == "THEORA")
	{
		g_hasSentHeader = true;
		g_websocketServer->sendFrameToClient(((TheoraCompressor*)m_compressor)->m_headerBuffer, ((TheoraCompressor*)m_compressor)->m_headerBufferLength);
	}
	if(g_timeSinceLastSentFrame > 0.01f && (g_hasSentHeader || COMPRESSION_TYPE != "THEORA"))
	{
		g_websocketServer->sendFrameToClient(compressedFrame, m_compressor->m_sizeOfFinishedBuffer);
		//g_websocketServer->sendFrameToClient((((RenderToMemoryBuffer*)m_renderer.m_singleFrameBuffer)->m_currentFrameInMemory), SCREEN_WIDTH * SCREEN_HEIGHT * 4);
		g_timeSinceLastSentFrame = 0.f;
	}
	ProfileSection::StopProfile("Network");

	int clientInputBufferSize = 1024;
	unsigned char clientInputBuffer[1024];
	int pollResult = g_websocketServer->pollFrameFromClient(clientInputBuffer, clientInputBufferSize);
	bool clientEventsRemain = true;
	for(int clientInputIndex = 0; clientInputIndex < clientInputBufferSize && clientEventsRemain; clientInputIndex+=2)
	{
		unsigned char keyCode = clientInputBuffer[clientInputIndex];
		if(keyCode != 0)
		{
			if(clientInputBuffer[clientInputIndex+1] == 0)
			{
				keyUpEvent(keyCode);
			}
			else
			{
				keyDownEvent(keyCode);
			}
		}
		else
		{
			clientEventsRemain = false;
		}
	}

	lastFrameNode.m_frameSizeBytes = m_compressor->m_sizeOfFinishedBuffer;
	//delete[] compressedFrame;
	m_compressor->cleanupAfterSend();

	std::map<Vector2i, Chunk*>::iterator vboUpdaterIndex = m_chunkMap.begin();
	bool updatedOneVbo = false;
	while(!updatedOneVbo && vboUpdaterIndex != m_chunkMap.end())
	{
		if(vboUpdaterIndex->second->m_VBOisDirty)
		{
			vboUpdaterIndex->second->generateVBO();
			updatedOneVbo = true;
		}
		vboUpdaterIndex++;
	}

	g_statistics.m_collectedData.push_back(lastFrameNode);
}

void GameMain::render()
{
	m_renderer.m_singleFrameBuffer->preRenderStep();

	m_worldCamera.preRenderStep();

	for (unsigned int ii = 0; ii < m_elements.size(); ii++)
	{
		m_elements[ii]->render();
	}

	float length = 10.f;

	//Use Red, Green, Blue for X Y Z respectively
	glUseProgram(0);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	//X axis
	glColor3f(1.0f,0.0f,0.0f);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(length,0.0f,0.0f);
	//Y axis
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(0.0f,length,0.0f);
	//Z axis
	glColor3f(0.0f,0.0f,1.0f);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(0.0f,0.0f,length);
	glEnd();


	glEnable(GL_DEPTH_TEST);
	for(int chunkX = m_bottomLeftChunk.x; chunkX < m_topRightChunk.x; chunkX++)
	{
		for(int chunkY = m_bottomLeftChunk.y; chunkY < m_topRightChunk.y; chunkY++)
		{
			float distanceSquared = (m_worldCamera.m_position.x/LENGTGH_OF_CHUNK - chunkX)*(m_worldCamera.m_position.x/LENGTGH_OF_CHUNK - chunkX) + (m_worldCamera.m_position.y/LENGTGH_OF_CHUNK - chunkY)*(m_worldCamera.m_position.y/LENGTGH_OF_CHUNK - chunkY);
			if(distanceSquared < LENGTH_IN_CHUNKS*LENGTH_IN_CHUNKS/4 - LENGTH_IN_CHUNKS/2)
			{
				Vector2i chunkToRender = Vector2i(chunkX, chunkY);
				glPushMatrix();
				glTranslatef((float)chunkX*LENGTGH_OF_CHUNK, (float)chunkY*LENGTGH_OF_CHUNK, 0.f);
				m_chunkMap[chunkToRender]->render();
				glPopMatrix();
			}
		}
	}

	glDisable(GL_DEPTH_TEST);

	glUseProgram(m_renderer.m_shaderID);

	m_worldCamera.postRenderStep();

	ProfileSection::StartProfile("GPU");
	m_renderer.m_singleFrameBuffer->postRenderStep();
	ProfileSection::StopProfile("GPU");

	if(m_displayConsole)
	{
		m_console.render();
	}

	glPushMatrix();
	//HACK hardcoded values
	glOrtho(1.0, 1024.0, 1.0, 1024.0/(16.0/9.0), 0, 1);
	int currentOffset = 0;
	for (auto profileSectionIterator = ProfileSection::s_movingAverageTimeOverFrames.begin(); profileSectionIterator != ProfileSection::s_movingAverageTimeOverFrames.end(); profileSectionIterator++)
	{
		float heightOfRows = HEIGHT_OF_LOG/ROWS_TO_DISPLAY_ON_SCREEN; //HACK from other files
		Vector3f startLocationForLine = Vector3f(500.f, (currentOffset+2)*heightOfRows, 0.f);
		m_console.m_log.m_fontRenderer.drawString(startLocationForLine, profileSectionIterator->first);
		std::stringstream timeVal;
		timeVal<<profileSectionIterator->second;
		m_console.m_log.m_fontRenderer.drawString(startLocationForLine+Vector3f(200.f, 0.f, 0.f), timeVal.str());
		currentOffset++;
	}
	glPopMatrix();

	glPopMatrix();

	g_frameNumber++;
}

bool GameMain::keyDownEvent(unsigned char asKey)
{
	
	if (asKey == VK_OEM_3)
	{
		m_displayConsole = !m_displayConsole;
	}
	else if(!m_displayConsole)
	{
		m_IOHandler.KeyDownEvent(asKey);
	}
	else
	{
		if(asKey == VK_LEFT)
		{
			if(m_console.m_currentTextOffset > 0)
			{
				m_console.m_currentTextOffset--;
			}		
		}

		else if (asKey == VK_RIGHT)
		{
			if(m_console.m_currentTextOffset < m_console.m_command.size())
			{
				m_console.m_currentTextOffset++;
			}
		}
	}
	return true;
}

bool GameMain::characterEvent(unsigned char asKey, unsigned char scanCode)
{
	return false;
}

bool GameMain::typingEvent(unsigned char asKey)
{
	if (m_displayConsole && asKey != '`' && asKey != '~')
	{
		if(asKey == '\n' || asKey == '\r')
		{
			m_console.executeCommand();
		}
		else if (asKey == '\b')
		{
			if(m_console.m_currentTextOffset > 0)
			{
				m_console.m_command.erase(m_console.m_command.begin()+(m_console.m_currentTextOffset-1));
				m_console.m_currentTextOffset--;
			}
			
		}
		//HACK reject all other non-printable characters
		else if(asKey > 31)
		{
			m_console.insertCharacterIntoCommand(asKey);
		}
		
		return true;
	}
	else
	{
		return false;
	}
}

bool GameMain::mouseEvent(unsigned int eventType)
{
	switch(eventType)
	{
	case WM_LBUTTONDOWN:
		{
			break;
		}
	case WM_RBUTTONDOWN:
		{
			break;
		}
	}

	return true;
}
