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
	class EditorLauncherWindow;

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

	ENJON_ENUM( )
	enum class ConfigurationType
	{
		Release,
		Debug,
		Count
	};

	// Want to be able to serialize/deserialize .json files... eventually 
	ENJON_CLASS( Construct )
	class ToolChainDefinition : public Object 
	{
		ENJON_CLASS_BODY( ToolChainDefinition )

		public:
	
		ENJON_PROPERTY( DisplayName = "label" )
		String mLabel;

		ENJON_PROPERTY( DisplayName = "cmake_generator" )
		String mCMakeGenerator;

		ENJON_PROPERTY( DisplayName = "command" )
		String mCommand;

		ENJON_PROPERTY( DisplayName = "args" )
		String mArgs[ (u32)ConfigurationType::Count ];

		ENJON_PROPERTY( DisplayName = "after_build_evt" )
		String mAfterBuildEvent;

		ENJON_PROPERTY( DisplayName = "output_dir" )
		String mOutputDir;

		ENJON_PROPERTY( DisplayName = "include" )
		Vector< String > mIncludeDirectories;

		ENJON_PROPERTY( DisplayName = "source" )
		Vector< String > mSources;

		ENJON_PROPERTY( DisplayName = "library_directories" )
		Vector< String > mLibraryDirectories; 

		ENJON_PROPERTY( DisplayName = "libraries" )
		Vector< String > mLibraries; 
	};

	struct ProjectConfig
	{
		String mName = "";
		String mPath = "";
		ToolChainDefinition mToolChain;
	};

	ENJON_CLASS( Construct )
	class Project : public Object
	{
		friend EditorApp;
		friend EditorLauncherWindow;

		ENJON_CLASS_BODY( Project )

		public:

			Project( const ProjectConfig& config );

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
			
		protected:
			ENJON_PROPERTY( )
			String mProjectName = "";

			ENJON_PROPERTY( )
			String mProjectPath = "";

			ENJON_PROPERTY( )
			String mBuildDirectory = "";

			ENJON_PROPERTY( )
			ToolChainDefinition mToolChainDefinition;

			Enjon::Application* mApp = nullptr; 
			EditorApp* mEditor = nullptr; 
	};
}

#endif

