#pragma once

#include "Ogre.h"
#include "OgreRoot.h"
#include "OgreWindow.h"
#include "OgreItem.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "CelestialBody.h"
#include "SceneObject.h"

#include "Compositor/OgreCompositorManager2.h"

#include "OgreHlms.h"
#include "Hlms/Unlit/OgreHlmsUnlit.h"
#include "Hlms/Pbs/OgreHlmsPbs.h"

#include "RenderSystems/Direct3D11/OgreD3D11Plugin.h"

#include "RenderThread.h"

#include <atomic>

class RenderEngine
{
	friend class RenderThread;

public:
	RenderEngine();
	~RenderEngine();
	RenderEngine(const RenderEngine&) = delete;
	RenderEngine& operator=(const RenderEngine&) = delete;

	void Update();

	bool GetQuit() { return m_bQuit; }
	void SetQuit(bool bQuit) { m_bQuit = bQuit; }

	RenderThread* GetRT() const { return m_pRT; }
	SceneObject* CreateSceneObject(Ogre::String actorName, Ogre::String meshName);

	std::atomic<bool> isInited;
private:
	bool SetOgreConfig();
	void LoadConfigSections(Ogre::ConfigFile& cf);
	void LoadHlms(Ogre::ConfigFile& cf);
	Ogre::String GetRootHlmsFolder(Ogre::ConfigFile& cf);
	void RegisterHlms(Ogre::String rootHlmsFolder);
	void GetHlmArchiveVec(Ogre::ArchiveVec& archivePbsLibraryFolders, Ogre::String rootHlmsFolder, Ogre::StringVector libraryFoldersPaths);
	void SetHlmsTextureBufferSize(Ogre::HlmsPbs* hlmsPbs, Ogre::HlmsUnlit* hlmsUnlit);


	void RT_Init();
	void RT_SetupDefaultCamera();
	void RT_SetupDefaultCompositor();
	void RT_LoadDefaultResources();
	void RT_LoadOgreHead();
	void RT_LoadPlanet(CelestialBody* actor, Ogre::Vector3 pos);
	void RT_UpdateActorPosition(SceneObject* actor, Ogre::Vector3 pos);
	void RT_SetupDefaultLight();
	void RT_OscillateCamera(float time);

	Ogre::Root* m_pRoot;
	Ogre::Window* m_pRenderWindow;
	Ogre::SceneManager* m_pSceneManager;
	Ogre::Camera* m_pCamera;
	Ogre::CompositorWorkspace* m_pWorkspace;
	Ogre::D3D11Plugin* m_pD3D11Plugin;
	SceneObject* OgreHead;

	RenderThread* m_pRT;
	
	bool m_bQuit;
};
