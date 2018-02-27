#include "Project.h"
#include "EditorApp.h"

#include <Utils/FileUtils.h>
#include <Engine.h>

#include <filesystem>

namespace FileSystem = std::experimental::filesystem; 

namespace Enjon
{
	//======================================================================

	void Project::SetEditor( EditorApp* app )
	{
		mEditor = app;
	}

	//======================================================================

	void Project::SetApplication( Application* app )
	{
		mApp = app;
	}

	//======================================================================

	Enjon::Application* Project::GetApplication( )
	{
		return mApp;
	}

	//======================================================================

	void Project::SetProjectPath( const String& path )
	{
		mProjectPath = path;
	}

	//======================================================================

	String Project::GetProjectPath( ) const
	{
		return mProjectPath;
	}

	//======================================================================

	void Project::SetProjectName( const String& name )
	{
		mProjectName = name;
	}

	//======================================================================

	String Project::GetProjectName( ) const
	{
		return mProjectName;
	}

	//======================================================================

	void Project::CreateBuildDirectory( )
	{
		// Grab the engine configuration
		EngineConfig engCfg = Engine::GetInstance( )->GetConfig( );
		
		String buildDir = mProjectPath + "Compiled/";

		// Set build directory
		mBuildDirectory = buildDir;

		// Create subdirectory for platform
#ifdef ENJON_SYSTEM_WINDOWS
		buildDir += "Windows/";
#endif

		// Create the build directory and others if they don't already exist
		FileSystem::create_directory( buildDir );
		FileSystem::create_directory( buildDir + "Assets/" );
		FileSystem::create_directory( buildDir + "Intermediate/" );

		// Grab needed source templates from editor app that need to be compiled into project
		String mainTemplate = Utils::FindReplaceAll( mEditor->GetProjectMainTemplate( ), "#PROJECT_NAME", mProjectName );

		// Write mainTemplate to file
		Utils::WriteToFile( mainTemplate, buildDir + "Intermediate/main.cpp" );

		// Get cmake file and replace necessary tags
		String cmakeFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::FindReplaceAll( mEditor->GetCompileProjectCMakeTemplate(), "#PROJECTNAME", mProjectName ), "#ENJONDIRECTORY", Enjon::Utils::FindReplaceAll( Engine::GetInstance( )->GetConfig( ).GetRoot( ), "\\", "/" ) );
		cmakeFile = Utils::FindReplaceAll( cmakeFile, "#PROJECT_BUILD_DIR", buildDir );
		//String buildAndRunFIle = mProjectBuildAndRunTemplate;

		// Write to file
		Enjon::Utils::WriteToFile( cmakeFile, buildDir + "Intermediate/CMakeLists.txt" ); 
		//Enjon::Utils::WriteToFile( buildAndRunFIle, projectDir + "Proc/" + "BuildAndRun.bat" ); 
	}

	//====================================================================== 

	Result Project::CompileProject( )
	{
		// Grab the engine configuration
		EngineConfig engCfg = Engine::GetInstance( )->GetConfig( ); 

		String buildConfig = mEditor->GetBuildConfig( );
		String visualStudioPath = mEditor->GetVisualStudioDirectoryPath( );

		// Run the build bat for project
#ifdef ENJON_SYSTEM_WINDOWS 
		// TODO(): Spawn up a separate thread to call this so we know when it's finished
		s32 code = system( String( "call " + mProjectPath + "Proc/" + "CompileProject.bat" + " " + Enjon::Utils::FindReplaceAll( mProjectPath, "/", "\\" ) + " " + mProjectName + " " + buildConfig + " " + visualStudioPath ).c_str() ); 
		if ( code == 0 )
		{
			// Success...
		}

#endif
		return Result::SUCCESS; 
	} 

	//======================================================================

	Result Project::BuildProject( )
	{
		// Create build directory if not already made
		CreateBuildDirectory( );

		return Result::SUCCESS;
	}

	//======================================================================
}
