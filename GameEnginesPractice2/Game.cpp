#include "Game.h"

Game::Game() :
	m_pRenderEngine(nullptr)
{
	m_pRenderEngine = new RenderEngine();

	m_Timer.Start();
}

Game::~Game()
{
}

void Game::Run()
{
	m_Timer.Reset();

	// So delivery of scene nodes executes on next frame, so we take wait entire next tick
	bool isDone = false;
	int count = 0;
	int tickOffset = 2;
	while (true)
	{
		m_pRenderEngine->GetRT()->RC_BeginFrame();

		m_Timer.Tick();

		if (!isDone && m_pRenderEngine->isInited)
		{
			GenerateSolarSystem();
			isDone = true;
		}
		if (isDone)
			count++;
		if(count > tickOffset)
			if (!Update())
				break;
		m_pRenderEngine->GetRT()->RC_EndFrame();
	}
}

bool Game::Update()
{
	static float t = 0;
	// t += m_Timer.DeltaTime();
	t += 0.001f;

	for (auto& body : solarSystem)
		body->UpdateVelocity(solarSystem, GravTimestep);

	for (auto& body : solarSystem)
	{
		body->UpdatePosition(GravTimestep);
		m_pRenderEngine->GetRT()->RC_UpdateActorPosition(body->GetActor(), body->GetPosition());
	}

	return true;
}

void Game::GenerateSolarSystem()
{
	CelestialBody* Sun = new CelestialBody("Sun");
	Sun->SetBodyParameters(50000, Ogre::Vector3(0, 0, 0), Ogre::Vector3(0, 0, 0), true);
	m_pRenderEngine->GetRT()->RC_LoadPlanet(Sun, Ogre::Vector3(0, 0, 0));
	solarSystem.push_back(Sun);

	CelestialBody* Earth = new CelestialBody("Earth");
	Earth->SetBodyParameters(1, Ogre::Vector3(0, 0, 50), Ogre::Vector3(81.91f, 0, 0), true);
	m_pRenderEngine->GetRT()->RC_LoadPlanet(Earth, Ogre::Vector3(0, 0, 50));
	solarSystem.push_back(Earth);

	CelestialBody* Mars = new CelestialBody("Earth2");
	Mars->SetBodyParameters(1, Ogre::Vector3(0, 0, 100), Ogre::Vector3(8.17f, 0, 0), true);
	m_pRenderEngine->GetRT()->RC_LoadPlanet(Mars, Ogre::Vector3(0, 0, 100));
	solarSystem.push_back(Mars);
}
