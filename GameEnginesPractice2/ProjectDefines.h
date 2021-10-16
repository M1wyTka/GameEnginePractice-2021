#pragma once

#include "OgreMemoryAllocatorConfig.h"

typedef unsigned long threadID;

#define SAFE_OGRE_DELETE(x) { if(x) { OGRE_DELETE(x); (x) = nullptr; } }

#define RESOURCE_CONFIG "resources2.cfg"