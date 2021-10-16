#pragma once
#include "Ogre.h"
#include "OgreRoot.h"
#include "OgreWindow.h"
#include "OgreItem.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#define IMPORT_NAME(x) x + " Imported" 

class SceneObject
{
public:
	SceneObject() = delete;
	SceneObject(Ogre::SceneManager& sceneManager, Ogre::String meshName);
	~SceneObject();

	void SetPosition(Ogre::Vector3 newPos);
	void SetScale(Ogre::Real x, Ogre::Real y, Ogre::Real z);
	Ogre::Vector3 GetPosition();

private:
	Ogre::MeshPtr v2Mesh;
	Ogre::SceneNode* m_pSceneNode;
	Ogre::Item* m_pItem;

private:
	void LoadMeshModel(Ogre::String meshName);
	void LoadManagerItems(Ogre::SceneManager& sceneManager);
};
