#include "OgreGraphicsFramework.h"

OgreGraphicsFramework::OgreGraphicsFramework() :
	m_pRoot(nullptr),
	m_pRenderWindow(nullptr),
	m_pSceneManager(nullptr),
	m_pD3D11Plugin(nullptr),
	m_pCamera(nullptr),
	m_pWorkspace(nullptr),
	m_bQuit(false)
{
}

void OgreGraphicsFramework::initialize()
{
	//Init
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
	Ogre::String sTitleName = "Game Engines Lecture 1";

	Ogre::NameValuePairList params;
	params.insert(std::make_pair("title", sTitleName));
	params.insert(std::make_pair("gamma", "true"));
	params.insert(std::make_pair("FSAA", "false"));
	params.insert(std::make_pair("vsync", "false"));

	m_pRenderWindow = Ogre::Root::getSingleton().createRenderWindow(sTitleName, width, height, false, &params);

	// Scene manager
	m_pSceneManager = m_pRoot->createSceneManager(Ogre::SceneType::ST_GENERIC, 1);

	// Camera
	m_pCamera = m_pSceneManager->createCamera("Main Camera");

	m_pCamera->setPosition(Ogre::Vector3(0, 10, 15));
	m_pCamera->lookAt(Ogre::Vector3(0, 0, 0));
	m_pCamera->setNearClipDistance(0.2f);
	m_pCamera->setFarClipDistance(1000.0f);
	m_pCamera->setAutoAspectRatio(true);


	// Compositor Manager
	Ogre::CompositorManager2* compositorManager = m_pRoot->getCompositorManager2();

	const Ogre::String workspaceName("WorkSpace");

	if (!compositorManager->hasWorkspaceDefinition(workspaceName))
	{
		compositorManager->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue::Blue);
	}

	m_pWorkspace = compositorManager->addWorkspace(m_pSceneManager, m_pRenderWindow->getTexture(), m_pCamera, workspaceName, true);

	// Add resources locations
	Ogre::ConfigFile cf;
	cf.load("resources2.cfg");

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

	// Load hlms (high level material system) files
	Ogre::String rootHlmsFolder = cf.getSetting("DoNotUseAsResource", "Hlms", "");

	if (rootHlmsFolder.empty())
		rootHlmsFolder = "./";
	else if (*(rootHlmsFolder.end() - 1) != '/')
		rootHlmsFolder += "/";

	//At this point rootHlmsFolder should be a valid path to the Hlms data folder

	Ogre::HlmsUnlit* hlmsUnlit = nullptr;
	Ogre::HlmsPbs* hlmsPbs = nullptr;

	//For retrieval of the paths to the different folders needed
	Ogre::String mainFolderPath;
	Ogre::StringVector libraryFoldersPaths;
	Ogre::StringVector::const_iterator libraryFolderPathIt;
	Ogre::StringVector::const_iterator libraryFolderPathEn;

	Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();

	{
		//Create & Register HlmsUnlit
		//Get the path to all the subdirectories used by HlmsUnlit
		Ogre::HlmsUnlit::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
		Ogre::Archive* archiveUnlit = archiveManager.load(rootHlmsFolder + mainFolderPath,
			"FileSystem", true);
		Ogre::ArchiveVec archiveUnlitLibraryFolders;
		libraryFolderPathIt = libraryFoldersPaths.begin();
		libraryFolderPathEn = libraryFoldersPaths.end();
		while (libraryFolderPathIt != libraryFolderPathEn)
		{
			Ogre::Archive* archiveLibrary =
				archiveManager.load(rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true);
			archiveUnlitLibraryFolders.push_back(archiveLibrary);
			++libraryFolderPathIt;
		}

		//Create and register the unlit Hlms
		hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit(archiveUnlit, &archiveUnlitLibraryFolders);
		Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);
	}

	{
		//Create & Register HlmsPbs
		//Do the same for HlmsPbs:
		Ogre::HlmsPbs::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
		Ogre::Archive* archivePbs = archiveManager.load(rootHlmsFolder + mainFolderPath,
			"FileSystem", true);

		//Get the library archive(s)
		Ogre::ArchiveVec archivePbsLibraryFolders;
		libraryFolderPathIt = libraryFoldersPaths.begin();
		libraryFolderPathEn = libraryFoldersPaths.end();
		while (libraryFolderPathIt != libraryFolderPathEn)
		{
			Ogre::Archive* archiveLibrary =
				archiveManager.load(rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true);
			archivePbsLibraryFolders.push_back(archiveLibrary);
			++libraryFolderPathIt;
		}

		//Create and register
		hlmsPbs = OGRE_NEW Ogre::HlmsPbs(archivePbs, &archivePbsLibraryFolders);
		Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);
	}


	Ogre::RenderSystem* renderSystem = m_pRoot->getRenderSystem();
	bool supportsNoOverwriteOnTextureBuffers;
	renderSystem->getCustomAttribute("MapNoOverwriteOnDynamicBufferSRV",
			&supportsNoOverwriteOnTextureBuffers);

	if (!supportsNoOverwriteOnTextureBuffers)
	{
		hlmsPbs->setTextureBufferDefaultSize(512 * 1024);
		hlmsUnlit->setTextureBufferDefaultSize(512 * 1024);
	}

	// Load resources
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(true);

	// Create Entity
	Ogre::Item* item = m_pSceneManager->createItem("Cube.mesh",
		Ogre::ResourceGroupManager::
		AUTODETECT_RESOURCE_GROUP_NAME,
		Ogre::SCENE_DYNAMIC);

	Ogre::SceneNode* sceneNode = m_pSceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->
		createChildSceneNode(Ogre::SCENE_DYNAMIC);

	sceneNode->setPosition(0, -1, 0);

	sceneNode->attachObject(item);
}

bool OgreGraphicsFramework::SetOgreConfig()
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

void OgreGraphicsFramework::update()
{
	if (m_pRenderWindow->isVisible())
		m_bQuit |= !m_pRoot->renderOneFrame();
}