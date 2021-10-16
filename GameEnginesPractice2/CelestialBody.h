#pragma once
#pragma once
#include "Ogre.h"
#include "SceneObject.h"

class CelestialBody
{
public:
	CelestialBody() = delete;

	CelestialBody(Ogre::String name);

	~CelestialBody();

	void SetSceneNode(SceneObject* node) { m_pSceneNode = node; }
	void SetBodyParameters(int mass = 1, Ogre::Vector3 InitialPosition = Ogre::Vector3(0, 0, 0),
		Ogre::Vector3 InitialVelocity = Ogre::Vector3(0, 0, 0), bool IsMovable = true);
	void UpdateVelocity(std::vector<CelestialBody*> allBodies, float timeStep);
	void UpdatePosition(float timeStep);
	Ogre::Vector3 GetPosition() { return m_vecCurrentPos; }
	Ogre::Real GetMass() { return m_nMass; }
	Ogre::String GetName() { return m_sName; }
	Ogre::Vector3 GetVelocity() { return m_vecCurrentVelocity; }
	SceneObject* GetActor() { return m_pSceneNode; }

private:
	Ogre::String m_sName;

	int m_nMass;
	bool m_bIsMovable;
	Ogre::Vector3 m_vecCurrentVelocity;
	Ogre::Vector3 m_vecCurrentPos;
	SceneObject* m_pSceneNode;
};

