
#pragma once
#ifndef ENJON_Basic_H
#define ENJON_Basic_H

// Enjon includes
#include <Enjon.h>

namespace Enjon
{
	ENJON_CLASS( Construct )
	class Basic : public Enjon::Application
	{
		ENJON_MODULE_BODY( Basic )

		public:
			/**
			* @brief Overrides application initialization method. Gets called on startup of application.
			*/
			virtual Enjon::Result Initialize() override;

			/**
			* @brief Overrides application update method. Gets called once per frame.
			*/
			virtual Enjon::Result Update( Enjon::f32 dt ) override;

			/**
			* @brief Overrides application shutdown method. Gets called when application is stopped.
			*/
			virtual Enjon::Result Shutdown( ) override;

			virtual void OnLoad( ) override;

		private:
			s32 mWindowID = -1;
	};

	// Declaration for module export
	ENJON_MODULE_DECLARE( Basic )
}



#endif

