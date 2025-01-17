#include "InputHandler.h"
#include "../ProjectDefines.h"

#include <algorithm>
#include <windows.h>

InputHandler::InputHandler(const std::string& strResourceRoot) : m_pWinHandle(nullptr)
{
	m_strMapFilePath = strResourceRoot + "actionmap.ini";
	std::replace(m_strMapFilePath.begin(), m_strMapFilePath.end(), '\\', '/');

	MapSymbol("a", 'A');
	MapSymbol("d", 'D');
	MapSymbol("left", VK_LEFT);
	MapSymbol("right", VK_RIGHT);

	MapCommandSymbol("GoLeft", eIC_GoLeft, "a");
	MapCommandSymbol("GoRight", eIC_GoRight, "d");

	LoadConfiguration();
	Remap();
}

InputHandler::~InputHandler()
{
}

bool InputHandler::IsKeyDown(size_t vk_key)
{
	if (GetAsyncKeyState(vk_key) & 0x8000)
		return true;

	return false;
}

void InputHandler::MapSymbol(std::string strSymbol, size_t nSymbol)
{
	m_symbolMap[strSymbol] = nSymbol;
}

void InputHandler::MapInputEvent(std::size_t nSymbol, size_t nCommand)
{
	m_inputEventMap[nSymbol] = nCommand;
}

void InputHandler::MapCommandSymbol(std::string strCommand, size_t nCommand, std::string strDefaultSymbol)
{
	m_commandMap[strCommand] = nCommand;
	m_commandSymbolMap[strCommand] = strDefaultSymbol;
}

void InputHandler::LoadConfiguration()
{
	Ogre::ConfigFile cf;
	cf.load(m_strMapFilePath);

	Ogre::ConfigFile::SectionIterator secIter = cf.getSectionIterator();

	Ogre::String strInputType, strCommand, strSymbol;
	while (secIter.hasMoreElements())
	{
		strInputType = secIter.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap* settings = secIter.getNext();

		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			strCommand = i->first;
			strSymbol = i->second;
			
			m_commandSymbolMap[strCommand] = strSymbol;
		}
	}
}

void InputHandler::Remap()
{
	for (auto& it : m_commandSymbolMap)
	{
		MapInputEvent(m_symbolMap[it.second], m_commandMap[it.first]);
	}
}

// We used int as return type just for demonstration. It should be done another way
void InputHandler::Update()
{
	for (auto& it : m_inputEventMap)
	{
		m_InputState.set(it.second, IsKeyDown(it.first));
	}

	if (m_pWinHandle) 
	{
		m_pPrevMousePos = m_pCurMousePos;

		GetCursorPos(&m_pMousePoint);
		ScreenToClient(m_pWinHandle, &m_pMousePoint);
		
		float x = float(m_pMousePoint.x);
		float y = float(m_pMousePoint.y);
		m_pCurMousePos = Ogre::Vector2(x, y);
	}
}

void InputHandler::SetWinHandle(HWND window)
{
	m_pWinHandle = window;
	GetCursorPos(&m_pMousePoint);
	ScreenToClient(m_pWinHandle, &m_pMousePoint);

	float x = float(m_pMousePoint.x);
	float y = float(m_pMousePoint.y);
	m_pCurMousePos = Ogre::Vector2(x, y);
}

const std::bitset<eIC_Max>& InputHandler::GetInputState() const
{
	return m_InputState;
}

Ogre::Vector2 InputHandler::MousePos() const
{
	return m_pCurMousePos;
}

Ogre::Vector2 InputHandler::DeltaMousePos() const
{
	return m_pCurMousePos - m_pPrevMousePos;
}