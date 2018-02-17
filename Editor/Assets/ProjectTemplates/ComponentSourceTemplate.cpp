#HEADERFILEBEGIN
#pragma once
#ifndef ENJON_#COMPONENTNAME_H
#define ENJON_#COMPONENTNAME_H

// Enjon includes
#include <Enjon.h>

namespace Enjon
{
	ENJON_CLASS( Construct )
	class #COMPONENTNAME : public Enjon::Component
	{
		ENJON_COMPONENT( #COMPONENTNAME )

		public:
			/**
			* @brief Overrides component initialization method. Gets called on startup of application.
			*/
			virtual Enjon::Result Initialize() override;

			/**
			* @brief Overrides component update method. Gets called once per frame.
			*/
			virtual Enjon::Result Update( Enjon::f32 dt ) override;

			/**
			* @brief Overrides component shutdown method. Gets called when application is stopped.
			*/
			virtual Enjon::Result Shutdown( ) override;
	};
}

#endif

#HEADERFILEEND

#SOURCEFILEBEGIN
// Project includes
#include "#COMPONENTNAME.h"

using namespace Enjon;

//==================================================================

Result #COMPONENTNAME::Initialize()
{
	return Result::SUCCESS;
}

//==================================================================

Result #COMPONENTNAME::Update( f32 dt )
{
	return Result::PROCESS_RUNNING;
}

//==================================================================

Result #COMPONENTNAME::Shutdown()
{
	return Result::SUCCESS;
}

//==================================================================

#SOURCEFILEEND


