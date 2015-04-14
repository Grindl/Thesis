#pragma once
#ifndef include_PANZERFAUST
#define include_PANZERFAUST


#include <vector>

#include "Systems\Game.hpp"
#include "Primitives\Element.hpp"
#include "Rendering\Renderer.hpp"
#include "Console\ConsoleLog.hpp"
#include "Console\Console.hpp"
#include "Camera.hpp"
#include "Chunk.hpp"
#include "CompressionInterface.hpp"
#include "StatisticsArray.hpp"

class Player;

class GameMain : public Game
{
public:
	GameMain();

	std::vector<Element*> m_elements;
	Renderer m_renderer;
	float m_internalTime;
	bool m_isQuitting;
	bool m_displayConsole;
	Console m_console;
	Camera m_worldCamera;
	std::map<Vector2i, Chunk*> m_chunkMap;
	//std::vector<Vector2i> m_chunksOnFile;
	Vector2i m_bottomLeftChunk;
	Vector2i m_topRightChunk;
	CompressionInterface* m_compressor;

	virtual void update(float deltaTime);
	virtual void render();
	virtual bool keyDownEvent(unsigned char asKey);
	virtual bool characterEvent(unsigned char asKey, unsigned char scanCode);
	virtual bool typingEvent(unsigned char asKey);
	virtual bool mouseEvent(unsigned int eventType);

	void mouseUpdate();
	void renderUI();
};



#endif 

