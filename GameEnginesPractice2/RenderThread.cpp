#include "RenderEngine.h"

// Creating Critical section interface
std::mutex RC_CriticalSection;
#define LOADINGCOMMAND_CRITICAL_SECTION std::scoped_lock<std::mutex> criticalSection (RC_CriticalSection);

// Function to run render thread
static unsigned RunThisThread(void* thisPtr)
{
	RenderThread* const self = (RenderThread*)thisPtr;
	self->Run();

	return 0;
}

RenderThread::RenderThread(RenderEngine* pRenderEngine) :
	m_pRenderEngine(pRenderEngine),
	m_nRenderThreadId(0),
	m_nCurrentFrame(0),
	m_nFrameFill(1),
	m_nFlush(0),
	m_pThread(nullptr)
{
	m_nMainThreadId = ::GetCurrentThreadId();

	m_Commands[0].Clear();
	m_Commands[1].Clear();
}

RenderThread::~RenderThread()
{

}

// Render Loop
void RenderThread::Run()
{
	m_nRenderThreadId = ::GetCurrentThreadId();

	while (true)
	{
		WaitForMainThreadSignal();

		RC_BeginFrame();

		ProcessCommands();

		SignalMainThread();

		m_pRenderEngine->Update();

		RC_EndFrame();

		if (m_pRenderEngine->GetQuit())
			break;
	}
}

void RenderThread::Start()
{
	if (!m_pThread)
	{
		m_pThread = std::unique_ptr<std::thread>(new std::thread(RunThisThread, this));
	}
}

bool RenderThread::IsRenderThread()
{
	return m_nRenderThreadId == ::GetCurrentThreadId();
}

// We have 2 buffers. One we fill, another - execute
void RenderThread::NextFrame()
{
	m_nCurrentFrame = (m_nCurrentFrame + 1) & 1;
	m_nFrameFill = (m_nFrameFill + 1) & 1;
}

bool RenderThread::CheckFlushCond()
{
	return *(int*)&m_nFlush != 0;
}

// Signal main thread, that he can continue his work
void RenderThread::SignalMainThread()
{
	m_nFlush = 0;
}

// Signal render thread, that he can continue his work
void RenderThread::SignalRenderThread()
{
	m_nFlush = 1;
}

// Process commands that render thread received from main thread
void RenderThread::ProcessCommands()
{
	assert(IsRenderThread());

	if (!CheckFlushCond())
		return;

	int n = 0;

	while (n < m_Commands[m_nCurrentFrame].Capacity())
	{
		byte* ptr = *m_Commands[m_nCurrentFrame] + n;	
		n += sizeof(UINT32);
		UINT32 nCommandType =*((UINT32*)ptr);

		switch (nCommandType)
		{
			case eRC_Init:
			{
				m_pRenderEngine->RT_Init();
				break;
			}
			case eRC_SetupDefaultCamera:
			{
				m_pRenderEngine->RT_SetupDefaultCamera();
				break;
			}
			case eRC_SetupDefaultCompositor:
			{
				m_pRenderEngine->RT_SetupDefaultCompositor();
				break;
			}
			case eRC_LoadDefaultResources:
			{
				m_pRenderEngine->RT_LoadDefaultResources();
				break;
			}
			case eRC_LoadOgreHead:
			{
				m_pRenderEngine->RT_LoadOgreHead();
				break;
			}
			case eRC_LoadActor:
			{
				// Read command
				CelestialBody* actor = ReadCommand<CelestialBody*>(n);
				float x = ReadCommand<float>(n);
				float y = ReadCommand<float>(n);
				float z = ReadCommand<float>(n);
				m_pRenderEngine->RT_LoadPlanet(actor, Ogre::Vector3(x, y, z));
				break;
			}
			case eRC_UpdateActorPosition:
			{
				SceneObject* actor = ReadCommand<SceneObject*>(n);
				float x = ReadCommand<float>(n);
				float y = ReadCommand<float>(n);
				float z = ReadCommand<float>(n);
				//Ogre::Vector3 pos = ReadCommand<Ogre::Vector3>(n);
				m_pRenderEngine->RT_UpdateActorPosition(actor, Ogre::Vector3(x, y, z));
				break;
			}
			case eRC_SetupDefaultLight:
			{
				m_pRenderEngine->RT_SetupDefaultLight();
				break;
			}
			case eRC_OscillateCamera:
			{
				float time = ReadCommand<float>(n);
				m_pRenderEngine->RT_OscillateCamera(time);
				break;
			}
		}
	}

	m_Commands[m_nCurrentFrame].Clear();
}

// We process comands via byte* using std::vector as raw data.
template <class T>
T RenderThread::ReadCommand(int& nIndex)
{
	byte* Res = *m_Commands[m_nCurrentFrame] + nIndex;
	nIndex += sizeof(T);
	return *reinterpret_cast<const T*>(Res);
}

byte* RenderThread::AddCommand(RenderCommand eRC, size_t nParamBytes)
{
	UINT32 cmdSize = sizeof(RenderCommand) + nParamBytes;
	byte* ptr = m_Commands[m_nFrameFill].Resize(m_Commands[m_nFrameFill].Capacity() * sizeof(byte) + cmdSize);
	AddDWORD(ptr, eRC);
	return ptr;
}

void RenderThread::AddDWORD(byte*& ptr, UINT32 nVal)
{
	*(UINT32*)ptr = nVal;
	ptr += sizeof(UINT32);
}

void RenderThread::AddFloat(byte*& ptr, const float fVal)
{
	*(float*)ptr = fVal;
	ptr += sizeof(float);
}

template <class T>
void RenderThread::AddWTF(byte*& ptr, T TVal)
{
	*(T*)ptr = TVal;
	ptr += sizeof(T);
}


void RenderThread::RC_Init()
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_Init();
		return;
	}

	LOADINGCOMMAND_CRITICAL_SECTION;
	byte* p = AddCommand(eRC_Init, 0);
}

void RenderThread::RC_SetupDefaultCamera()
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_SetupDefaultCamera();
		return;
	}

	LOADINGCOMMAND_CRITICAL_SECTION;
	byte* p = AddCommand(eRC_SetupDefaultCamera, 0);
}

void RenderThread::RC_SetupDefaultCompositor()
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_SetupDefaultCompositor();
		return;
	}

	LOADINGCOMMAND_CRITICAL_SECTION;
	byte* p = AddCommand(eRC_SetupDefaultCompositor, 0);
}

void RenderThread::RC_LoadDefaultResources()
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_LoadDefaultResources();
		return;
	}

	LOADINGCOMMAND_CRITICAL_SECTION;
	byte* p = AddCommand(eRC_LoadDefaultResources, 0);
}

void RenderThread::RC_LoadOgreHead()
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_LoadOgreHead();
		return;
	}

	LOADINGCOMMAND_CRITICAL_SECTION;
	byte* p = AddCommand(eRC_LoadOgreHead, 0);
}

void RenderThread::RC_LoadPlanet(CelestialBody* actor, Ogre::Vector3 pos)
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_LoadPlanet(actor, pos);
		return;
	}

	LOADINGCOMMAND_CRITICAL_SECTION;
	size_t total = sizeof(CelestialBody*) + 3 * sizeof(float);
	byte* p = AddCommand(eRC_LoadActor, total);
	AddWTF<CelestialBody*>(p, actor);

	float x = pos.x;
	float y = pos.y;
	float z = pos.z;
	AddFloat(p, x);
	AddFloat(p, y);
	AddFloat(p, z);
}

void RenderThread::RC_UpdateActorPosition(SceneObject* actor, Ogre::Vector3 pos)
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_UpdateActorPosition(actor, pos);
		return;
	}
	LOADINGCOMMAND_CRITICAL_SECTION;

	float x = pos.x;
	float y = pos.y;
	float z = pos.z;

	size_t total = sizeof(SceneObject*) + 3 * sizeof(float);
	byte* p = AddCommand(eRC_UpdateActorPosition, total);
	AddWTF<SceneObject*>(p, actor);
	AddFloat(p, x);
	AddFloat(p, y);
	AddFloat(p, z);
}

void RenderThread::RC_SetupDefaultLight()
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_SetupDefaultLight();
		return;
	}

	LOADINGCOMMAND_CRITICAL_SECTION;
	byte* p = AddCommand(eRC_SetupDefaultLight, 0);
}

void RenderThread::RC_OscillateCamera(float time)
{
	if (IsRenderThread())
	{
		m_pRenderEngine->RT_OscillateCamera(time);
		return;
	}

	LOADINGCOMMAND_CRITICAL_SECTION;
	byte* p = AddCommand(eRC_OscillateCamera, sizeof(float));
	AddFloat(p, time);
}

void RenderThread::RC_BeginFrame()
{

}

void RenderThread::RC_EndFrame()
{
	if (IsRenderThread())
		return;

	SyncMainWithRender();
}

void RenderThread::SyncMainWithRender()
{
	assert(!IsRenderThread());

	WaitForRenderThreadSignal();

	// Switch buffers
	{
		LOADINGCOMMAND_CRITICAL_SECTION;
		NextFrame();
		m_Commands[m_nFrameFill].Clear();
	}

	SignalRenderThread();
}

// Wait signal from main thread
void RenderThread::WaitForMainThreadSignal()
{
	assert(IsRenderThread());

	while (!m_nFlush)
	{
	}
}

// Wait signal from render thread
void RenderThread::WaitForRenderThreadSignal()
{
	assert(!IsRenderThread());

	while (m_nFlush)
	{
	}
}