#include "RenderEngine.h"

#include "ProjectDefines.h"

RenderEngine::RenderEngine() :
	m_pRoot(nullptr),
	m_pRenderWindow(nullptr),
	m_pSceneManager(nullptr),
	m_pD3D11Plugin(nullptr),
	m_pCamera(nullptr),
	m_pWorkspace(nullptr),
	m_pRT(nullptr),
	m_bQuit(false)
{
	isInited = false;
	m_pRT = new RenderThread(this);

	m_pRT->RC_Init();
	m_pRT->RC_SetupDefaultCamera();
	m_pRT->RC_SetupDefaultCompositor();
	m_pRT->RC_LoadDefaultResources();

	m_pRT->RC_SetupDefaultLight();
	m_pRT->RC_LoadOgreHead();

	m_pRT->Start();
}

RenderEngine::~RenderEngine()
{
	SAFE_OGRE_DELETE(m_pRoot);
}

bool RenderEngine::SetOgreConfig()
{
#ifdef _DEBUG
	constexpr bool bAlwaysShowConfigWindow = true;
	if (bAlwaysShowConfigWindow || !m_pRoot->restoreConfig())
#else
	if (!m_pRoot->restoreConfig())
#endif
	{
		if (!m_pRoot->showConfigDialog())
		{
			return false;
		}
	}

	return true;
}

void RenderEngine::Update()
{
	Ogre::WindowEventUtilities::messagePump();

	if (m_pRenderWindow->isVisible())
		m_bQuit |= !m_pRoot->renderOneFrame();
}

void RenderEngine::RT_Init()
{
	m_pRoot = OGRE_NEW Ogre::Root();
	m_pD3D11Plugin = OGRE_NEW Ogre::D3D11Plugin();

	m_pRoot->installPlugin(m_pD3D11Plugin);

	if (!SetOgreConfig())
	{
		m_bQuit = true;
		return;
	}

	m_pRoot->initialise(false);

	// Creating window
	Ogre::uint32 width = 1280;
	Ogre::uint32 height = 720;
	Ogre::String sTitleName = "Game Engine";

	m_pRenderWindow = Ogre::Root::getSingleton().createRenderWindow(sTitleName, width, height, false);

	// Scene manager
	m_pSceneManager = m_pRoot->createSceneManager(Ogre::SceneType::ST_GENERIC, 1);
}

void RenderEngine::RT_SetupDefaultCamera()
{
	m_pCamera = m_pSceneManager->createCamera("Main Camera");

	m_pCamera->setPosition(Ogre::Vector3(100, 100, 100));
	m_pCamera->lookAt(Ogre::Vector3(0, 0, 0));
	m_pCamera->setNearClipDistance(0.2f);
	m_pCamera->setFarClipDistance(1000.0f);
	m_pCamera->setAutoAspectRatio(true);
}

void RenderEngine::RT_SetupDefaultCompositor()
{
	Ogre::CompositorManager2* compositorManager = m_pRoot->getCompositorManager2();

	const Ogre::String workspaceName("WorkSpace");

	if (!compositorManager->hasWorkspaceDefinition(workspaceName))
	{
		compositorManager->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue::Blue);
	}

	m_pWorkspace = compositorManager->addWorkspace(m_pSceneManager, m_pRenderWindow->getTexture(), m_pCamera, workspaceName, true);
}

void RenderEngine::RT_LoadDefaultResources()
{
	Ogre::ConfigFile cf;
	cf.load(RESOURCE_CONFIG);

	LoadConfigSections(cf);

	LoadHlms(cf);

	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(true);
}

void RenderEngine::LoadConfigSections(Ogre::ConfigFile& cf)
{
	// Go through all sections & settings in the file
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();

		if (secName != "Hlms")
		{
			Ogre::ConfigFile::SettingsMultiMap::iterator i;
			for (i = settings->begin(); i != settings->end(); ++i)
			{
				typeName = i->first;
				archName = i->second;
				Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
					archName, typeName, secName);
			}
		}
	}
}

void RenderEngine::LoadHlms(Ogre::ConfigFile& cf)
{
	// Load hlms (high level material system) files
	Ogre::String rootHlmsFolder = GetRootHlmsFolder(cf);
	RegisterHlms(rootHlmsFolder);

}

Ogre::String RenderEngine::GetRootHlmsFolder(Ogre::ConfigFile& cf)
{
	// Load hlms (high level material system) files
	Ogre::String rootHlmsFolder = cf.getSetting("DoNotUseAsResource", "Hlms", "");

	if (rootHlmsFolder.empty())
		rootHlmsFolder = "./";
	else if (*(rootHlmsFolder.end() - 1) != '/')
		rootHlmsFolder += "/";
	return rootHlmsFolder;
}

void RenderEngine::RegisterHlms(Ogre::String rootHlmsFolder)
{
	//For retrieval of the paths to the different folders needed
	Ogre::String mainFolderPath;
	Ogre::StringVector libraryFoldersPaths;

	Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();

	//Create & Register HlmsUnlit
		//Get the path to all the subdirectories used by HlmsUnlit
	Ogre::HlmsUnlit::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
	Ogre::Archive* archiveUnlit = archiveManager.load(rootHlmsFolder + mainFolderPath,
		"FileSystem", true);

	Ogre::ArchiveVec archiveUnlitLibraryFolders;
	GetHlmArchiveVec(archiveUnlitLibraryFolders, rootHlmsFolder, libraryFoldersPaths);

	//Create and register the unlit Hlms
	Ogre::HlmsUnlit* hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit(archiveUnlit, &archiveUnlitLibraryFolders);
	Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);

	//Create & Register HlmsPbs
	//Do the same for HlmsPbs:
	Ogre::HlmsPbs::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
	Ogre::Archive* archivePbs = archiveManager.load(rootHlmsFolder + mainFolderPath,
		"FileSystem", true);

	//Get the library archive(s)
	Ogre::ArchiveVec archivePbsLibraryFolders;
	GetHlmArchiveVec(archivePbsLibraryFolders, rootHlmsFolder, libraryFoldersPaths);

	//Create and register
	Ogre::HlmsPbs* hlmsPbs = OGRE_NEW Ogre::HlmsPbs(archivePbs, &archivePbsLibraryFolders);
	Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);

	SetHlmsTextureBufferSize(hlmsPbs, hlmsUnlit);
}

void RenderEngine::GetHlmArchiveVec(Ogre::ArchiveVec& archiveVec, Ogre::String rootHlmsFolder, Ogre::StringVector libraryFoldersPaths)
{
	//Get the library archive(s)
	Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();
	Ogre::StringVector::const_iterator libraryFolderPathIt = libraryFoldersPaths.begin();;
	Ogre::StringVector::const_iterator libraryFolderPathEn = libraryFoldersPaths.end();;
	while (libraryFolderPathIt != libraryFolderPathEn)
	{
		Ogre::Archive* archiveLibrary =
			archiveManager.load(rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true);
		archiveVec.push_back(archiveLibrary);
		++libraryFolderPathIt;
	}
}

void RenderEngine::SetHlmsTextureBufferSize(Ogre::HlmsPbs* hlmsPbs, Ogre::HlmsUnlit* hlmsUnlit)
{
	Ogre::RenderSystem* renderSystem = m_pRoot->getRenderSystem();
	bool supportsNoOverwriteOnTextureBuffers;
	renderSystem->getCustomAttribute("MapNoOverwriteOnDynamicBufferSRV", &supportsNoOverwriteOnTextureBuffers);

	if (!supportsNoOverwriteOnTextureBuffers)
	{
		hlmsPbs->setTextureBufferDefaultSize(512 * 1024);
		hlmsUnlit->setTextureBufferDefaultSize(512 * 1024);
	}
}

void RenderEngine::RT_LoadOgreHead()
{
	OgreHead = new SceneObject(*m_pSceneManager, "fish.mesh");
	OgreHead->SetPosition(Ogre::Vector3(0, 0, 0));
	isInited = true;
}

void RenderEngine::RT_SetupDefaultLight()
{
	// Lightning
	Ogre::Light* light = m_pSceneManager->createLight();
	Ogre::SceneNode* lightNode = m_pSceneManager->getRootSceneNode()->createChildSceneNode();
	lightNode->attachObject(light);
	light->setPowerScale(Ogre::Math::PI); //Since we don't do HDR, counter the PBS' division by PI
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());
}

void RenderEngine::RT_OscillateCamera(float time)
{
	m_pCamera->setPosition(Ogre::Vector3(50, time, 50));
}

void RenderEngine::RT_LoadPlanet(CelestialBody* actor, Ogre::Vector3 pos)
{
	actor->SetSceneNode(new SceneObject(*m_pSceneManager, "Sphere.mesh"));
	actor->GetActor()->SetPosition(pos);
}

void RenderEngine::RT_UpdateActorPosition(SceneObject* actor, Ogre::Vector3 pos)
{
	actor->SetPosition(pos);
}

SceneObject* RenderEngine::CreateSceneObject(Ogre::String actorName, Ogre::String meshName)
{
	//std::lock_guard<std::mutex> lock(creation);
	return new SceneObject(*m_pSceneManager, meshName);

	//return new SceneObject(*m_pSceneManager, meshName);
}