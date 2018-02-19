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
			* @brief Overrides component initialization method. Gets called on startup of application before Start().
			*/
			virtual void Initialize( ) override;

			/**
			* @brief Overrides component start method. Gets called on startup of application after Initialize().
			*/
			virtual void Start( ) override;

			/**
			* @brief Overrides component update method. Gets called once per frame.
			*/
			virtual void Update( ) override;

			/**
			* @brief Overrides component shutdown method. Gets called when application is stopped.
			*/
			virtual void Shutdown( ) override;
	};
}

#endif

#HEADERFILEEND

#SOURCEFILEBEGIN
// Project includes
#include "#COMPONENTNAME.h"

using namespace Enjon;

//==================================================================

void #COMPONENTNAME::Initialize( )
{
}

//==================================================================

void #COMPONENTNAME::Start( )
{
}

//==================================================================

void #COMPONENTNAME::Update( )
{
}

//==================================================================

void #COMPONENTNAME::Shutdown( )
{
}

//==================================================================

#SOURCEFILEEND


