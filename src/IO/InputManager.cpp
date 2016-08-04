#include "IO/InputManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Input {

	InputManager::InputManager() : m_mouseCoords(0.0f) 
	{
		// Set up GamePadController
		GamePadController = {};
		GamePadController.ControllerHandle = nullptr;

		int MaxJoysticks = SDL_NumJoysticks();
		int ControllerIndex = 0;
		for(int JoystickIndex = 0; JoystickIndex < MaxJoysticks; ++JoystickIndex)
		{
		    if (!SDL_IsGameController(JoystickIndex))
		    {
		        continue;
		    }
		    if (ControllerIndex >= MAX_CONTROLLERS)
		    {
		        break;
		    }

		    auto CHandle = SDL_GameControllerOpen(JoystickIndex);
		   	GamePadController.ControllerHandle = CHandle; 
		    ControllerIndex++;
		}	
	}

	InputManager::~InputManager()
	{
		if (GamePadController.ControllerHandle) SDL_GameControllerClose(GamePadController.ControllerHandle);
	}

	void InputManager::Update()
	{ 
		// Update keyboard keys
		for(auto& it : m_keyMap)
		{
			m_previousKeyMap[it.first] = it.second;
		}

		// Update Game Controller button keys
		for (auto& it : GamePadController.m_buttonMap)
		{
			GamePadController.m_previousButtonMap[it.first] = it.second;
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

	////////////////////////////
	// Controller //////////////

	void Controller::PressButton(unsigned int buttonID)
	{
		m_buttonMap[buttonID] = true;
	}

	void Controller::ReleaseButton(unsigned int buttonID)
	{
		m_buttonMap[buttonID] = false;
	}

	bool Controller::IsButtonDown(unsigned int buttonID)
	{
		auto it = m_buttonMap.find(buttonID);

		if(it != m_buttonMap.end()){
			return it->second;
		}

		else return false;
	}

	bool Controller::WasButtonDown(unsigned int buttonID)
	{
		auto it = m_previousButtonMap.find(buttonID);
	
		if(it != m_previousButtonMap.end()){
			return it->second;
		}
	
		else return false;
	}

	bool Controller::IsButtonPressed(unsigned int buttonID)
	{
		if(IsButtonDown(buttonID) == true && WasButtonDown(buttonID) == false) 
			return true; 
		else 
			return false;
	}
}}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////