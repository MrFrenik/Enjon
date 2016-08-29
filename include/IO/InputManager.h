#ifndef ENJON_INPUTMANAGER_H
#define ENJON_INPUTMANAGER_H


#define MAX_CONTROLLERS 4

#include "Math/Maths.h"

#include <unordered_map> 
#include <SDL2/SDL.h>

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Input {
	
	struct Controller
	{
		std::unordered_map<unsigned int, bool> m_buttonMap;
		std::unordered_map<unsigned int, bool> m_previousButtonMap;
		float Axis0Value;
		float Axis1Value;
		SDL_GameController* ControllerHandle;

		// GamePad
		void PressButton(unsigned int buttonID);
		void ReleaseButton(unsigned int buttonID);
		bool IsButtonDown(unsigned int buttonID);
		bool WasButtonDown(unsigned int buttonID);
		bool IsButtonPressed(unsigned int buttonID);
	};

	class InputManager
	{
		public:
			InputManager();
			~InputManager();


			void Update();

			// Keyboard
			void PressKey(unsigned int keyID);
			void ReleaseKey(unsigned int keyID);
			bool IsKeyDown(unsigned int keyID);
			bool IsKeyPressed(unsigned int keyID);
			bool WasKeyDown(unsigned int keyID);

			// GamePad
			void PressButton(unsigned int buttonID);
			void ReleaseButton(unsigned int buttonID);
			bool IsButtonDown(unsigned int buttonID);
			bool IsButtonPressed(unsigned int buttonID);
			bool WasButtonDown(unsigned int buttonID);

			//Setters
			void SetMouseCoords(float x, float y);
			
			//Getters
			inline Enjon::Math::Vec2 GetMouseCoords() const { return m_mouseCoords; }

		public:
			std::vector<SDL_GameController*> ControllerHandles;
			Controller GamePadController;
		
		private:
			std::unordered_map<unsigned int, bool> m_keyMap;
			std::unordered_map<unsigned int, bool> m_previousKeyMap;
			std::unordered_map<unsigned int, bool> m_buttonMap;
			std::unordered_map<unsigned int, bool> m_previousButtonMap;
			Enjon::Math::Vec2 m_mouseCoords;
	};

}}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#endif