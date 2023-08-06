#include "Game.h"
#include <iostream>
Game::Game() :
	m_pRenderEngine(nullptr)
{
	m_pFileSystem = new FileSystem();
	m_pResourceManager = new ResourceManager(m_pFileSystem->GetMediaRoot());
	m_pInputHandler = new InputHandler(m_pFileSystem->GetMediaRoot());
	m_pRenderEngine = new RenderEngine(m_pResourceManager);

	m_Timer.Start();
}

Game::~Game()
{	
}

void Game::Run()
{
	m_Timer.Reset();
	bool isDone = false;
	while (true)
	{
		m_pRenderEngine->GetRT()->RC_BeginFrame();

		m_Timer.Tick();

		if (!isDone && m_pRenderEngine->isInited) 
		{
			m_pInputHandler->SetWinHandle(m_pRenderEngine->GetWinHandle());
			isDone = true;
		}

		if (m_pInputHandler) {
			m_pInputHandler->Update();
		}
			

		if(isDone)
			if (!Update())
				break;

		m_pRenderEngine->GetRT()->RC_EndFrame();
	}
}

bool Game::Update()
{
	static float t = 0;
	t += m_Timer.DeltaTime();
	
	float offset = 0.0f;
	if (m_pInputHandler->GetInputState().test(eIC_GoLeft))
		offset -= 1.0f;
	if (m_pInputHandler->GetInputState().test(eIC_GoRight))
		offset += 1.0f;
	offset *= m_Timer.DeltaTime();

	Ogre::Vector2 MousePos = m_pInputHandler->MousePos();
	Ogre::Vector2 delta = m_pInputHandler->DeltaMousePos();
	float x = MousePos.x;
	float y = MousePos.y;
	m_pRenderEngine->GetRT()->RC_OscillateCamera(10 + sin(t));
	m_pRenderEngine->GetRT()->RC_MoveOgreHead(delta.x/100);

	return true;
}
