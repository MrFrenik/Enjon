#HEADERFILEBEGIN
#pragma once
#ifndef ENJON_#PROJECTNAME_H
#define ENJON_#PROJECTNAME_H

// Enjon includes
#include <Enjon.h>

ENJON_CLASS( Construct )
class #PROJECTNAME : public Enjon::Application
{
	ENJON_CLASS_BODY( )

	public:
		/**
		* @brief Public default constructor
		*/
		#PROJECTNAME();

		/**
		* @brief Public default destructor
		*/
		~#PROJECTNAME();

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
};

// Definition for module export
ENJON_MODULE_DEFINE( #PROJECTNAME )

#endif

#HEADERFILEEND

#SOURCEFILEBEGIN
// Project include
#include "#PROJECTNAME.h"

// Module implementation for this project
ENJON_MODULE_IMPL( #PROJECTNAME )

using namespace Enjon;

//==================================================================

#PROJECTNAME::#PROJECTNAME()
{
}

//==================================================================

#PROJECTNAME::~#PROJECTNAME()
{
}

//==================================================================

Result #PROJECTNAME::Initialize()
{
	return Result::SUCCESS;
}

//==================================================================

Result #PROJECTNAME::Update( f32 dt )
{
	return Result::PROCESS_RUNNING;
}

//==================================================================

Result #PROJECTNAME::Shutdown()
{
	return Result::SUCCESS;
}

//==================================================================

#SOURCEFILEEND


