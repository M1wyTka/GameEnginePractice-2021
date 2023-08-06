#pragma once

#include "Input.h"

#include <string>
#include <unordered_map>
#include <bitset>

#include "Ogre.h"

#include <windows.h>

class InputHandler
{
public:
	InputHandler(const std::string& strResourceRoot);
	~InputHandler();

	void Update();

	const std::bitset<eIC_Max>& GetInputState() const;
	Ogre::Vector2 MousePos() const;
	Ogre::Vector2 DeltaMousePos() const;

	void SetWinHandle(HWND window);
private:
	void LoadConfiguration();

	bool IsKeyDown(size_t vk_key);

	void MapSymbol(std::string strSymbol, size_t nSymbol);
	void MapInputEvent(std::size_t nSymbol, size_t nCommand);
	void MapCommandSymbol(std::string strCommand, size_t nCommand, std::string strDefaultSymbol);
	void Remap();

	std::string m_strMapFilePath;

	typedef std::unordered_map<std::string, size_t> TCommandMap;
	typedef std::unordered_map<std::string, size_t> TSymbolMap;
	typedef std::unordered_map<size_t, size_t> TInputEventMap;
	typedef std::unordered_map<std::string, std::string> TCommandSymbolMap;

	TCommandMap m_commandMap;
	TSymbolMap m_symbolMap;
	TInputEventMap m_inputEventMap;
	TCommandSymbolMap m_commandSymbolMap;

	std::bitset<eIC_Max> m_InputState;
	
	HWND m_pWinHandle;
	POINT m_pMousePoint;
	Ogre::Vector2 m_pCurMousePos;
	Ogre::Vector2 m_pPrevMousePos;


};

