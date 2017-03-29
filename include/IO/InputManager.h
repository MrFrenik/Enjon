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
		bool IsButtonDown( u32 buttonID );
		bool WasButtonDown( u32 buttonID );
		bool IsButtonPressed( u32 buttonID );
	};

	class Input : public Subsystem
	{
		public: 

			/**
			* @brief Constructor
			*/
			Input();

			/**
			* @brief Destructor
			*/
			~Input(); 

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
			void PressKey( u32 keyID );
			
			/**
			* @brief
			*/
			void ReleaseKey( u32 keyID );
			
			/**
			* @brief
			*/
			bool IsKeyDown( u32 keyID );
			
			/**
			* @brief
			*/
			bool IsKeyPressed( u32 keyID );
			
			/**
			* @brief
			*/
			bool WasKeyDown( u32 keyID );

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
			bool IsButtonDown( u32 buttonID );
			
			/**
			* @brief
			*/
			bool IsButtonPressed( u32 buttonID );
			
			/**
			* @brief
			*/
			bool WasButtonDown( u32 buttonID );

			/**
			* @brief
			*/
			void SetMouseCoords( f32 x, f32 y );
			
			/**
			* @brief
			*/
			Vec2 GetMouseCoords() const { return m_mouseCoords; }

		public:
			std::vector<SDL_GameController*> ControllerHandles;
			Controller GamePadController;
		
		private:
			std::unordered_map<u32, bool> m_keyMap;
			std::unordered_map<u32, bool> m_previousKeyMap;
			std::unordered_map<u32, bool> m_buttonMap;
			std::unordered_map<u32, bool> m_previousButtonMap;
			Enjon::Vec2 m_mouseCoords;
	}; 
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#endif