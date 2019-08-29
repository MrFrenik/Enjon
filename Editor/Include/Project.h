#pragma once
#ifndef ENJON_PROJECT_H
#define ENJON_PROJECT_H

#include <System/Types.h>
#include <Base/Object.h>
#include <Defines.h>
 
namespace Enjon
{
	class Application;
	class EditorApp;

	/*
		What are the various build systems we could have? 

		Visual Studio: 
			- VSDir
			- MSBuild
		G++: 
			- G++Dir
		Clang: 
			- ClangDir

		// Need to write a JSON serializer for objects
		JSONObjectArchiver

		// Will load various build system types? Not sure if I like that

		class BuildSystemSettings
		{
			String mName;
			String mCompilerDirectory;
			String mGenerationCommands;
		};

		class BuildSystem : public Object
		{
			String mName;
			String mCompilerDirectory;
			String mGenerationCommands;
			String mAdditionalInforamtion;
		};

		The editor instance will load in an instance of this serialized build system object and then use that for project creation/compiling/etc.
		For now this will just be a binary serialized object, editor_cfg.esys 
		Honestly, I'd like for the config settings to be all held within here for the editor...

		
	*/

	class Project
	{
		public:
			Project( ) = default;
			~Project( ) = default; 

			void SetEditor( EditorApp* app );
			void SetApplication( Enjon::Application* app ); 
			Enjon::Application* GetApplication( );

			void SetProjectPath( const String& path );
			String GetProjectPath( ) const;

			void SetProjectName( const String& name );
			String GetProjectName( ) const;

			Result CompileProject( );

			Result BuildProject( );

			void Simluate( );

			bool IsLoaded( ) const;

		private:

			String CreateBuildDirectory( );
			
		private:
			String mProjectName = "";
			Enjon::Application* mApp = nullptr; 
			String mProjectPath = "";
			String mBuildDirectory = "";
			EditorApp* mEditor = nullptr;
	};
}

#endif

