#pragma once
#ifndef ENJON_PROJECT_H
#define ENJON_PROJECT_H

#include <System/Types.h>
#include <Defines.h>
 
namespace Enjon
{
	class Application;

	class Project
	{
		public:
			Project( ) = default;
			~Project( ) = default; 

			void SetApplication( Enjon::Application* app ); 
			Enjon::Application* GetApplication( );

			void SetProjectPath( const String& path );
			String GetProjectPath( ) const;

			void SetProjectName( const String& name );
			String GetProjectName( ) const;
			
		private:
			String mProjectName = "";
			Enjon::Application* mApp = nullptr; 
			String mProjectPath = "";
	};
}

#endif

