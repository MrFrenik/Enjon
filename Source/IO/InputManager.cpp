// @file Input.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "IO/InputManager.h" 
#include "Graphics/Window.h"

namespace Enjon 
{ 
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
		if ( GamePadController.ControllerHandle )
		{
			SDL_GameControllerClose( GamePadController.ControllerHandle );
		}
 
		mKeyMap.clear( );
		mPreviousKeyMap.clear();
		mButtonMap.clear();
		mPreviousButtonMap.clear();

		return Result::SUCCESS;
	}

	void Input::SetButtonState( KeyCode code, bool currentState, bool previousState )
	{
		mKeyMap[ KeyCode( code ) ] = currentState;
		mPreviousKeyMap[KeyCode( code )] = previousState;
	}

	void Input::PressKey( u32 code )
	{ 
		mKeyMap[(KeyCode)code] = true;
	}
	
	void Input::ReleaseKey( u32 code )
	{
		mKeyMap[(KeyCode)code] = false;
	}

	bool Input::IsKeyReleased( KeyCode code ) const
	{
		auto query = mKeyMap.find( code );
		if ( query != mKeyMap.end( ) )
		{
			bool isDown = query->second;
			bool wasDown = WasKeyDown( code );
			if ( wasDown && !isDown )
			{
				return true;
			} 
		}

		return false;
	}

	//Returns true if key held down	
	bool Input::IsKeyDown( KeyCode code ) const
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

	bool Input::WasKeyDown( KeyCode code ) const
	{ 
		auto query = mPreviousKeyMap.find( code );
	
		if( query != mPreviousKeyMap.end() )
		{
			return query->second;
		}
	
		else return false; 
	}

	//Returns true if key is just pressed
	bool Input::IsKeyPressed( KeyCode code ) const
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

	void Input::SetMouseWheel( const Vec2& wheel )
	{
		mMouseWheel = wheel;
	}

	Vec2 Input::GetMouseWheel( ) const
	{
		return mMouseWheel;
	}

	void Controller::PressButton( u32 buttonID )
	{
		m_buttonMap[buttonID] = true;
	}

	void Controller::ReleaseButton( u32 buttonID )
	{
		m_buttonMap[buttonID] = false;
	}

	bool Controller::IsButtonDown( u32 buttonID ) const
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

	bool Controller::WasButtonDown( u32 buttonID ) const
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

	bool Controller::IsButtonPressed( u32 buttonID ) const
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