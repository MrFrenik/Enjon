#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <unordered_map> 

#include "Math/Maths.h"

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Input {

	class InputManager{

	public:
		InputManager();
		~InputManager();

		void Update();
		void PressKey(unsigned int keyID);
		void ReleaseKey(unsigned int keyID);
		bool IsKeyDown(unsigned int keyID);
		bool IsKeyPressed(unsigned int keyID);
		bool WasKeyDown(unsigned int keyID);

		//Setters
		void SetMouseCoords(float x, float y);
		
		//Getters
		inline Enjon::Math::Vec2 GetMouseCoords() const { return m_mouseCoords; }
	
	private:
		std::unordered_map<unsigned int, bool> m_keyMap;
		std::unordered_map<unsigned int, bool> m_previousKeyMap;
		Enjon::Math::Vec2 m_mouseCoords;

	};

}}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#endif