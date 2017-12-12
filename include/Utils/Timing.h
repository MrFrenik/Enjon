#ifndef ENJON_TIMING_H
#define ENJON_TIMING_H

#include <SDL2/SDL.h> 

#define MILLISECOND       1.0f  
#define SECOND            MILLISECOND * 1000.0f

namespace Enjon { namespace Utils { 

	class FPSLimiter
	{
	public:
		FPSLimiter();
		~FPSLimiter();

		//Init limiter
		void Init(float maxFPS);

		//Begin limiter
		void Begin();

		//End limiter and return FPS
		float End();

		//Setters
		void SetMaxFPS(float maxFPS) { m_maxFPS = maxFPS; }

		float GetDT( ) const
		{
			return mDT;
		}

	private: 
		float m_maxFPS;
		Uint32 m_startTicks;
		float m_FPS;
		float m_frameTime;
		float mDT = 0.1f;

	private:
		void CalculateFPS();
	};


}} 

#endif