#pragma once

#include "MTQueue.h"

#include <thread>
#include <condition_variable>
#include <mutex>

#include "ProjectDefines.h"

class RenderEngine;

enum RenderCommand : UINT32
{
	eRC_Unknown = 0,
	eRC_Init,
	eRC_SetupDefaultCamera,
	eRC_SetupDefaultCompositor,
	eRC_LoadDefaultResources,
	eRC_LoadOgreHead, 
	eRC_LoadActor,
	eRC_UpdateActorPosition,
	eRC_SetupDefaultLight,
	eRC_OscillateCamera,
	eRC_BeginFrame,
	eRC_EndFrame
};

class RenderThread
{
public:
	RenderThread(RenderEngine* pRenderEngine);
	~RenderThread();

	void Start();
	void Run();

	void RC_Init();
	void RC_SetupDefaultCamera();
	void RC_SetupDefaultCompositor();
	void RC_LoadDefaultResources();
	void RC_LoadOgreHead();
	void RC_LoadPlanet(CelestialBody* actor, Ogre::Vector3 pos);
	void RC_SetupDefaultLight();
	void RC_OscillateCamera(float time);
	void RC_UpdateActorPosition(SceneObject* actor, Ogre::Vector3 pos);
	void RC_BeginFrame();
	void RC_EndFrame();

private:
	threadID m_nRenderThreadId;
	threadID m_nMainThreadId;

	volatile UINT32 m_nFlush;

	std::unique_ptr<std::thread> m_pThread;
	
	RenderEngine* m_pRenderEngine;
	
	MTQueue<byte> m_Commands[2];
	int m_nCurrentFrame;
	int m_nFrameFill;

	template <class T>
	T ReadCommand(int& nIndex);

	inline byte* AddCommand(RenderCommand eRC, size_t nParamBytes);
	inline void AddDWORD(byte*& ptr, UINT32 nVal);
	inline void AddFloat(byte*& ptr, const float fVal);
	
	template <class T>
	inline void AddWTF(byte*& ptr, T TVal);

	bool IsRenderThread();

	void ProcessCommands();
	void NextFrame();

	inline bool CheckFlushCond();
	inline void SignalMainThread();
	inline void SignalRenderThread();
	void WaitForMainThreadSignal();
	void WaitForRenderThreadSignal();
	void SyncMainWithRender();
};

