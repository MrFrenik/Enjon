// @file Input.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_INPUTMANAGER_H
#define ENJON_INPUTMANAGER_H 

#define MAX_CONTROLLERS 4

#include "Math/Maths.h"
#include "Subsystem.h"
#include "System/Types.h"
#include "Defines.h"

#include <unordered_map> 
#include <SDL2/SDL.h>

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

namespace Enjon 
{ 
	class Window;

	enum class KeyCode : u32
	{
		A				= SDLK_a,
		B				= SDLK_b,
		C				= SDLK_c,
		D				= SDLK_d,
		E				= SDLK_e,
		F				= SDLK_f,
		G				= SDLK_g,
		H				= SDLK_h,
		I				= SDLK_i,
		J				= SDLK_j,
		K				= SDLK_k,
		L				= SDLK_l,
		M				= SDLK_m,
		N				= SDLK_n,
		O				= SDLK_o,
		P				= SDLK_p,
		Q				= SDLK_q,
		R				= SDLK_r,
		S				= SDLK_s,
		T				= SDLK_t, 
		U				= SDLK_u,
		V				= SDLK_v,
		W				= SDLK_w,
		X				= SDLK_x,
		Y				= SDLK_y,
		Z				= SDLK_z,
		LeftShift		= SDLK_LSHIFT,
		RightShift		= SDLK_RSHIFT,
		LeftAlt			= SDLK_LALT,
		RightAlt		= SDLK_RALT,
		LeftCtrl		= SDLK_LCTRL,
		RightCtrl		= SDLK_RCTRL,
		Backspace		= SDLK_BACKSPACE,
		Backslash		= SDLK_BACKSLASH,
		QuestionMark	= SDLK_QUESTION,
		Tilde			= SDLK_BACKQUOTE,
		Comma			= SDLK_COMMA,
		Period			= SDLK_PERIOD,
		Escape			= SDLK_ESCAPE, 
		Space			= SDLK_SPACE,
		Left			= SDLK_LEFT,
		Up				= SDLK_UP,
		Right			= SDLK_RIGHT,
		Down			= SDLK_DOWN,
		Zero			= SDLK_0,
		One				= SDLK_1,
		Two				= SDLK_2,
		Three			= SDLK_3,
		Four			= SDLK_4,
		Five			= SDLK_5,
		Six				= SDLK_6,
		Seven			= SDLK_7,
		Eight			= SDLK_8,
		Nine			= SDLK_9,
		NumPadZero		= SDLK_KP_0,
		NumPadOne		= SDLK_KP_1,
		NumPadTwo		= SDLK_KP_2,
		NumPadThree		= SDLK_KP_3,
		NumPadFour		= SDLK_KP_4,
		NumPadFive		= SDLK_KP_5,
		NumPadSix		= SDLK_KP_6,
		NumPadSeven		= SDLK_KP_7,
		NumPadEight		= SDLK_KP_8,
		NumPadNine		= SDLK_KP_9,
		CapsLock		= SDLK_CAPSLOCK,
		Delete			= SDLK_DELETE,
		End				= SDLK_END,
		F1				= SDLK_F1,
		F2				= SDLK_F2,
		F3				= SDLK_F3,
		F4				= SDLK_F4,
		F5				= SDLK_F5,
		F6				= SDLK_F6,
		F7				= SDLK_F7,
		F8				= SDLK_F8,
		F9				= SDLK_F9,
		F10				= SDLK_F10,
		F11				= SDLK_F11,
		F12				= SDLK_F12,
		Home			= SDLK_HOME,
		Plus			= SDLK_PLUS,
		Minus			= SDLK_MINUS,
		LeftBracket		= SDLK_LEFTBRACKET,
		RightBracket	= SDLK_RIGHTBRACKET,
		SemiColon		= SDLK_SEMICOLON,
		Enter			= SDLK_RETURN,
		Insert			= SDLK_INSERT,
		PageUp			= SDLK_PAGEUP,
		PageDown		= SDLK_PAGEDOWN,
		NumLock			= SDLK_NUMLOCKCLEAR,
		Tab				= SDLK_TAB,
		NumPadMultiply	= SDLK_KP_MULTIPLY,
		NumPadDivide	= SDLK_KP_DIVIDE,
		NumPadPlus		= SDLK_KP_PLUS,
		NumPadMinus		= SDLK_KP_MINUS,
		NumPadEnter		= SDLK_KP_ENTER,
		NumPadDelete	= SDLK_KP_DECIMAL,
		NumPadDecimal	= SDLK_KP_DECIMAL,
		NumPadPageDown	= SDLK_KP_3,
		NumPadPageUp	= SDLK_KP_9,
		NumPadEnd		= SDLK_KP_1,
		NumPadHome		= SDLK_KP_7,
		Mute			= SDLK_MUTE,
		VolumeUp		= SDLK_VOLUMEUP,
		VolumeDown		= SDLK_VOLUMEDOWN,
		Pause			= SDLK_PAUSE,
		PrintScreen		= SDLK_PRINTSCREEN,
		LeftMouseButton = SDL_BUTTON_LEFT, 
		RightMouseButton = SDL_BUTTON_RIGHT,
		MiddleMouseButton = SDL_BUTTON_MIDDLE
	}; 

	class Mouse
	{
		public:
			Mouse( ) {}
			~Mouse( ) {}

		private:
			Vec2 mCoords;
			s32 mScroll;
	};

	enum class MouseButtonCode
	{
		LeftMouseButton		= SDL_BUTTON_LEFT,
		RightMouseButton	= SDL_BUTTON_RIGHT,
		MiddleMouseButton	= SDL_BUTTON_MIDDLE
	};

	using KeyCodeMap		= std::unordered_map< KeyCode, bool >;
	using MouseButtonMap	= std::unordered_map< MouseButtonCode, bool >;

	struct Controller
	{
		std::unordered_map< u32, bool > m_buttonMap;
		std::unordered_map< u32, bool > m_previousButtonMap;
		float Axis0Value;
		float Axis1Value;
		SDL_GameController* ControllerHandle;

		// GamePad
		void PressButton( u32 buttonID );
		void ReleaseButton( u32 buttonID );
		bool IsButtonDown( u32 buttonID ) const;
		bool WasButtonDown( u32 buttonID ) const;
		bool IsButtonPressed( u32 buttonID ) const; 
	};

	ENJON_CLASS( )
	class Input : public Subsystem
	{ 
		ENJON_CLASS_BODY( )

		friend Window;

		public: 

			/**
			* @brief Constructor
			*/
			Input( ) = default;

			/**
			* @brief Destructor
			*/
			~Input( ) = default;

			/**
			* @brief
			*/
			virtual Result Initialize() override;

			/**
			* @brief
			*/
			virtual void Update( const f32 dT ) override;

			/**
			* @brief
			*/
			virtual Result Shutdown() override;

			/**
			* @brief
			*/
			void PressKey( u32 key );
			
			/**
			* @brief
			*/
			void ReleaseKey( u32 key );
			
			/**
			* @brief
			*/
			bool IsKeyDown( KeyCode code ) const;
			
			/**
			* @brief
			*/
			bool IsKeyPressed( KeyCode code ) const;

			/**
			* @brief
			*/
			bool IsKeyReleased( KeyCode code ) const;
			
			/**
			* @brief
			*/
			bool WasKeyDown( KeyCode code ) const;
			
			/**
			* @brief
			*/
			bool IsMouseClicked( MouseButtonCode code ) const;
			
			/**
			* @brief
			*/
			bool IsMouseDoubleClicked( MouseButtonCode code ) const;
			
			/**
			* @brief
			*/
			bool IsMouseDown( MouseButtonCode code ) const;
			
			/**
			* @brief
			*/
			bool IsMouseReleased( MouseButtonCode code ) const;
			
			/**
			* @brief
			*/
			bool WasMouseDown( MouseButtonCode code ) const;

			/**
			* @brief
			*/
			void PressButton( u32 buttonID );
			
			/**
			* @brief
			*/
			void ReleaseButton( u32 buttonID );
			
			/**
			* @brief
			*/
			bool IsButtonDown( u32 buttonID ) const;
			
			/**
			* @brief
			*/
			bool IsButtonPressed( u32 buttonID ) const;
			
			/**
			* @brief
			*/
			bool WasButtonDown( u32 buttonID ) const;

			/**
			* @brief
			*/
			void SetMouseCoords( f32 x, f32 y );

			/**
			* @brief
			*/
			void SetMouseWheel( const Vec2& wheel );

			/**
			* @brief
			*/
			Vec2 GetMouseWheel( ) const;
			
			/**
			* @brief
			*/
			Vec2 GetMouseCoords() const 
			{ 
				return mMouseCoords; 
			}

		protected: 
			void SetButtonState( KeyCode code, bool currentState, bool previousState );

		public:
			std::vector<SDL_GameController*> ControllerHandles;
			Controller GamePadController;
		
		private:
			KeyCodeMap mKeyMap;
			KeyCodeMap mPreviousKeyMap;
			KeyCodeMap mButtonMap;
			KeyCodeMap mPreviousButtonMap;
			Vec2 mMouseCoords;
			Vec2 mMouseWheel;
	}; 
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#endif