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
		buildDir += "/Windows/";
#endif

		// Create the build directory if it doesn't exist
		if ( !FileSystem::exists( "" ) )
		{ 
			FileSystem::create_directory( buildDir );
			FileSystem::create_directory( buildDir + "Assets/" );
		} 

		//String includeFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::ParseFromTo( "#HEADERFILEBEGIN", "#HEADERFILEEND", mProjectSourceTemplate, false ), "#PROJECTNAME", projectName );
		//String sourceFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::ParseFromTo( "#SOURCEFILEBEGIN", "#SOURCEFILEEND", mProjectSourceTemplate, false ), "#PROJECTNAME", projectName ); 
		//String cmakeFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::FindReplaceAll( mProjectCMakeTemplate, "#PROJECTNAME", projectName ), "#ENJONDIRECTORY", Enjon::Utils::FindReplaceAll( Engine::GetInstance( )->GetConfig( ).GetRoot( ), "\\", "/" ) );
		//String delBatFile = mProjectDelBatTemplate;
		//String buildAndRunFIle = mProjectBuildAndRunTemplate;

		//// Write to file
		//Enjon::Utils::WriteToFile( includeFile, projectDir + "Source/" + projectName + ".h" );
		//Enjon::Utils::WriteToFile( sourceFile, projectDir + "Source/" + projectName + ".cpp" ); 
		//Enjon::Utils::WriteToFile( cmakeFile, projectDir + "CMakeLists.txt" ); 
		//Enjon::Utils::WriteToFile( delBatFile, projectDir + "Proc/" + "DelPDB.bat" ); 
		//Enjon::Utils::WriteToFile( buildAndRunFIle, projectDir + "Proc/" + "BuildAndRun.bat" ); 
		//Enjon::Utils::WriteToFile( "", projectDir + "Build/Generator/Linked/" + projectName + "_Generated.cpp" ); 
		//Enjon::Utils::WriteToFile( projectDir, projectDir + projectName + ".eproj" );
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
}
