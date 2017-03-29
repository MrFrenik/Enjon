#include "IO/InputManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { 

	Input::Input() 
		: m_mouseCoords(0.0f) 
	{
		Initialize();
	}

	Input::~Input()
	{
		if ( GamePadController.ControllerHandle )
		{
			SDL_GameControllerClose( GamePadController.ControllerHandle );
		}
	}

	Result Input::Initialize()
	{
		// Set up GamePadController
		GamePadController = {};
		GamePadController.ControllerHandle = nullptr;

		s32 MaxJoysticks = SDL_NumJoysticks();
		s32 ControllerIndex = 0;
		for ( s32 JoystickIndex = 0; JoystickIndex < MaxJoysticks; ++JoystickIndex )
		{
			if ( !SDL_IsGameController( JoystickIndex ) )
			{
				continue;
			}
			if ( ControllerIndex >= MAX_CONTROLLERS )
			{
				break;
			}

			auto CHandle = SDL_GameControllerOpen( JoystickIndex );
			GamePadController.ControllerHandle = CHandle;
			ControllerIndex++; 
		}
			
		return Result::SUCCESS;
	}

	void Input::Update( const f32 dT )
	{ 
		// Update keyboard keys
		for ( auto& it : m_keyMap )
		{
			m_previousKeyMap[it.first] = it.second;
		}

		// Update Game Controller button keys
		for ( auto& it : GamePadController.m_buttonMap )
		{
			GamePadController.m_previousButtonMap[it.first] = it.second;
		}
	}

	Result Input::Shutdown()
	{
		// Do things here...

		return Result::SUCCESS;
	}

	void Input::PressKey( u32 keyID )
	{ 
		m_keyMap[keyID] = true;
	}
	
	void Input::ReleaseKey( u32 keyID )
	{
		m_keyMap[keyID] = false;
	}

	//Returns true if key held down	
	bool Input::IsKeyDown( u32 keyID )
	{ 
		auto it = m_keyMap.find( keyID );

		if ( it != m_keyMap.end() )
		{
			return it->second;
		}

		else
		{
			return false;
		}
	}

	bool Input::WasKeyDown( u32 keyID )
	{ 
		auto it = m_previousKeyMap.find( keyID );
	
		if( it != m_previousKeyMap.end() )
		{
			return it->second;
		}
	
		else return false; 
	}

	//Returns true if key is just pressed
	bool Input::IsKeyPressed( u32 keyID )
	{ 
		if ( IsKeyDown( keyID ) == true && WasKeyDown( keyID ) == false )
		{
			return true; 
		}
		else
		{
			return false; 
		}
	}
	
	void Input::SetMouseCoords( f32 x, f32 y )
	{
		m_mouseCoords.x = x;
		m_mouseCoords.y = y;
	}

	void Controller::PressButton( u32 buttonID )
	{
		m_buttonMap[buttonID] = true;
	}

	void Controller::ReleaseButton( u32 buttonID )
	{
		m_buttonMap[buttonID] = false;
	}

	bool Controller::IsButtonDown( u32 buttonID )
	{
		auto it = m_buttonMap.find( buttonID );

		if ( it != m_buttonMap.end() ) 
		{
			return it->second;
		}

		else
		{
			return false;
		}
	}

	bool Controller::WasButtonDown( u32 buttonID )
	{
		auto it = m_previousButtonMap.find( buttonID );
	
		if( it != m_previousButtonMap.end() )
		{
			return it->second;
		}
	
		else
		{
			return false;
			return false;
		}
	}

	bool Controller::IsButtonPressed( u32 buttonID )
	{
		if ( IsButtonDown( buttonID ) == true && WasButtonDown( buttonID ) == false )
		{
			return true; 
		}
		else
		{
			return false; 
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////