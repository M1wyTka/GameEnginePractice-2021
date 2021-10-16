#include "SceneObject.h"

SceneObject::SceneObject(Ogre::SceneManager& sceneManager, Ogre::String meshName)
{
	LoadMeshModel(meshName);

	LoadManagerItems(sceneManager);

}

SceneObject::~SceneObject()
{
	delete m_pSceneNode;
	delete m_pItem;
}

void SceneObject::LoadMeshModel(Ogre::String meshName)
{
	static int counter = 1;
	Ogre::v1::MeshPtr v1Mesh;

	v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
		meshName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
		Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC);

	//Create a v2 mesh to import to, with a different name (arbitrary).
	v2Mesh = Ogre::MeshManager::getSingleton().createManual(
		IMPORT_NAME(meshName)+ std::to_string(counter++), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	bool halfPosition = true;
	bool halfUVs = true;
	bool useQtangents = true;

	//Import the v1 mesh to v2
	v2Mesh->importV1(v1Mesh.get(), halfPosition, halfUVs, useQtangents);

	//We don't need the v1 mesh. Free CPU memory, get it out of the GPU.
	//Leave it loaded if you want to use athene with v1 Entity.
	v1Mesh->unload();
}

void SceneObject::LoadManagerItems(Ogre::SceneManager& sceneManager)
{
	//Create an Item with the model we just imported.
	//Notice we use the name of the imported model. We could also use the overload
	//with the mesh pointer:
	m_pItem = sceneManager.createItem(v2Mesh->getName(),
		Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
		Ogre::SCENE_DYNAMIC);
	m_pSceneNode = sceneManager.getRootSceneNode(Ogre::SCENE_DYNAMIC)->
		createChildSceneNode(Ogre::SCENE_DYNAMIC);
	m_pSceneNode->attachObject(m_pItem);
	m_pSceneNode->scale(0.1f, 0.1f, 0.1f);
}

void SceneObject::SetPosition(Ogre::Vector3 newPos)
{
	m_pSceneNode->setPosition(newPos);
}

void SceneObject::SetScale(Ogre::Real x, Ogre::Real y, Ogre::Real z)
{
	m_pSceneNode->scale(x, y, z);
}

Ogre::Vector3 SceneObject::GetPosition()
{
	return m_pSceneNode->getPosition();
}