#include "IO/InputManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Input {

	InputManager::InputManager() : m_mouseCoords(0.0f)
	{}

	InputManager::~InputManager(){}

	void InputManager::Update()
	{ 
		for(auto& it : m_keyMap)
		{
			m_previousKeyMap[it.first] = it.second;
		}
	}

	void InputManager::PressKey(unsigned int keyID)
	{ 
		m_keyMap[keyID] = true;
	}
	
	void InputManager::ReleaseKey(unsigned int keyID)
	{
		m_keyMap[keyID] = false;
	}

	//Returns true if key held down	
	bool InputManager::IsKeyDown(unsigned int keyID)
	{ 
		auto it = m_keyMap.find(keyID);

		if(it != m_keyMap.end()){
			return it->second;
		}

		else return false;
	}

	bool InputManager::WasKeyDown(unsigned int keyID)
	{ 
		auto it = m_previousKeyMap.find(keyID);
	
		if(it != m_previousKeyMap.end()){
			return it->second;
		}
	
		else return false; 
	}

	//Returns true if key is just pressed
	bool InputManager::IsKeyPressed(unsigned int keyID)
	{ 
		if(IsKeyDown(keyID) == true && WasKeyDown(keyID) == false) 
			return true; 
		else 
			return false; 
	}
	
	void InputManager::SetMouseCoords(float x, float y)
	{
		m_mouseCoords.x = x;
		m_mouseCoords.y = y;
	}
}}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////