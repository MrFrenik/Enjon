// @file Input.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "IO/InputManager.h" 

namespace Enjon 
{ 
	Input::Input() 
		: mMouseCoords(0.0f) 
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
		for ( auto& key : mKeyMap )
		{
			mPreviousKeyMap[key.first] = key.second;
		}

		// Update Game Controller button keys
		for ( auto& button : GamePadController.m_buttonMap )
		{
			GamePadController.m_previousButtonMap[button.first] = button.second;
		}
	}

	Result Input::Shutdown()
	{
		// Do things here...

		return Result::SUCCESS;
	}

	void Input::PressKey( u32 code )
	{ 
		mKeyMap[(KeyCode)code] = true;
	}
	
	void Input::ReleaseKey( u32 code )
	{
		mKeyMap[(KeyCode)code] = false;
	}

	//Returns true if key held down	
	bool Input::IsKeyDown( KeyCode code )
	{ 
		auto query = mKeyMap.find( code );

		if ( query != mKeyMap.end() )
		{
			return query->second;
		}

		else
		{
			return false;
		}
	}

	bool Input::WasKeyDown( KeyCode code )
	{ 
		auto query = mPreviousKeyMap.find( code );
	
		if( query != mPreviousKeyMap.end() )
		{
			return query->second;
		}
	
		else return false; 
	}

	//Returns true if key is just pressed
	bool Input::IsKeyPressed( KeyCode code )
	{ 
		if ( IsKeyDown( code ) && !WasKeyDown( code ) )
		{
			return true; 
		}

		return false; 
	}
	
	void Input::SetMouseCoords( f32 x, f32 y )
	{
		mMouseCoords.x = x;
		mMouseCoords.y = y;
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
		auto query = m_buttonMap.find( buttonID );

		if ( query != m_buttonMap.end() ) 
		{
			return query->second;
		}

		else
		{
			return false;
		}
	}

	bool Controller::WasButtonDown( u32 buttonID )
	{
		auto query = m_previousButtonMap.find( buttonID );
	
		if( query != m_previousButtonMap.end() )
		{
			return query->second;
		}
	
		else
		{
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