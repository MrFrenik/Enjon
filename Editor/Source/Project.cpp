#include "Project.h"
#include "EditorApp.h"
#include "EditorLauncherWindow.h"

#include <Utils/FileUtils.h>
#include <Engine.h>

#include <fs/filesystem.hpp>

#ifdef ENJON_SYSTEM_WINDOWS 
	#define WINDOWS_LEAN_AND_MEAN
	#include <windows.h>
#endif

namespace FileSystem = ghc::filesystem; 

namespace Enjon
{
	//======================================================================

	Project::Project( const ProjectConfig& config ) 
	{ 
		mProjectName = config.mName;
		mProjectPath = config.mPath;
		mToolChainDefinition = config.mToolChain;
	}

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

	String Project::CreateBuildDirectory( )
	{
		// Grab the engine configuration
		EngineConfig engCfg = Engine::GetInstance( )->GetConfig( );
		String enginePath = Utils::FindReplaceAll( engCfg.GetRoot( ), "\\", "/" );
		
		String buildDir = mProjectPath + "Compiled/";

		// Set build directory
		mBuildDirectory = buildDir; 

		// Create subdirectory for platform
#ifdef ENJON_SYSTEM_WINDOWS
		buildDir += "Windows/";
#endif

		// Create the build directory and others if they don't already exist
		FileSystem::create_directories( buildDir );
		FileSystem::create_directories( buildDir + "Assets/" );
		FileSystem::create_directories( buildDir + "Intermediate/" );

		// Grab needed source templates from editor app that need to be compiled into project
		String mainTemplate = Utils::FindReplaceAll( mEditor->GetProjectMainTemplate( ), "#PROJECT_NAME", mProjectName );

		// Write mainTemplate to file
		Utils::WriteToFile( mainTemplate, buildDir + "Intermediate/main.cpp" );

		// Get cmake file and replace necessary tags
		String cmakeFile = Enjon::Utils::FindReplaceAll( mEditor->GetCompileProjectCMakeTemplate(), "#PROJECTNAME", mProjectName );
		cmakeFile = Utils::FindReplaceAll( cmakeFile, "#ENJON_DIRECTORY", enginePath );
		cmakeFile = Utils::FindReplaceAll( cmakeFile, "#PROJECT_BUILD_DIR", buildDir + "Intermediate/" ); 
		cmakeFile = Utils::FindReplaceAll( cmakeFile, "#PROJECT_ROOT_DIR", mProjectPath );
		cmakeFile = Utils::FindReplaceAll( cmakeFile, "#CURRENT_SOURCE_DIR", buildDir );

		// Write all necessary intermediate files to directory for project build
		Enjon::Utils::WriteToFile( cmakeFile, buildDir + "Intermediate/CMakeLists.txt" ); 
		Enjon::Utils::WriteToFile( mEditor->GetBuildAndRunCompileTemplate(), buildDir + "Intermediate/" + "Build.bat" ); 
		Enjon::Utils::WriteToFile( mEditor->GetProjectEnjonDefinesTemplate(), buildDir + "Intermediate/" + "Defines.h" ); 

		// Copy all Enjon assets into build directory as well
		String engineAssetsPath = enginePath + "Assets/";

		// Remove all previous content from directories
		FileSystem::remove_all( buildDir + "Assets/Shaders" );
		FileSystem::remove_all( buildDir + "Assets/Cache" );
		FileSystem::remove_all( buildDir + "Assets/Fonts" );

		// Copy engine assets
		FileSystem::copy( engineAssetsPath + "Shaders/", buildDir + "Assets/Shaders", FileSystem::copy_options::recursive );
		FileSystem::copy( engineAssetsPath + "Cache/", buildDir + "Assets/Cache", FileSystem::copy_options::recursive );
		FileSystem::copy( engineAssetsPath + "Fonts/", buildDir + "Assets/Fonts", FileSystem::copy_options::recursive );

		// Copy project assets
		FileSystem::copy( mProjectPath + "Assets/", buildDir + "Assets/Cache", FileSystem::copy_options::recursive );

		return buildDir;

#ifdef ENJON_SYSTEM_WINDOWS 
		// Run the bat file to build and run the solution
		String vsPath = mEditor->GetVisualStudioDirectoryPath( );
		s32 code = system( String( "call " + buildDir + "Intermediate/" + "Build.bat" + " " + mProjectName + " " + buildDir + "Intermediate/" + " " +  vsPath ).c_str() ); 

		// Copy the executable to the main project build directory
		if ( FileSystem::exists( buildDir + "Intermediate/Release/" + mProjectName + ".exe" ) )
		{
			// Remove previous instance
			if ( FileSystem::exists( buildDir + mProjectName + ".exe" ) )
			{
				FileSystem::remove( buildDir + mProjectName + ".exe" );
			}
			
			// Remove previous SDL2 .dll
			if ( FileSystem::exists( buildDir + "SDL2.dll" ) )
			{
				FileSystem::remove( buildDir + "SDL2.dll" );
			}
			// Remove previous glew32 .dlls
			if ( FileSystem::exists( buildDir + "glew32.dll" ) )
			{
				FileSystem::remove( buildDir + "glew32.dll" );
			}
			// Remove previous freetype6 .dlls
			if ( FileSystem::exists( buildDir + "freetype6.dll" ) )
			{
				FileSystem::remove( buildDir + "freetype6.dll" );
			}

			// Copy executable
			FileSystem::copy( buildDir + "Intermediate/Release/" + mProjectName + ".exe", buildDir + mProjectName + ".exe" );
			// Copy dlls
			FileSystem::copy( enginePath + "Build/Release/" + "SDL2.dll", buildDir + "SDL2.dll" );
			FileSystem::copy( enginePath + "Build/Release/" + "freetype6.dll", buildDir + "freetype6.dll" );
			FileSystem::copy( enginePath + "Build/Release/" + "glew32.dll", buildDir + "glew32.dll" ); 

			// Remove intermediate directory after compilation
			//FileSystem::remove_all( buildDir + "Intermediate" );
		}
#endif
	}

	//====================================================================== 

	Result Project::CompileProject( )
	{
		// Need to figure this part out, unfortunately... I need to be able to compile from the editor.
		//

		if ( !mEditor )
		{
			std::cout << "EditorApp is null! Cannot continue!\n";
			return Result::FAILURE;
		}

		// Grab the engine configuration
		EngineConfig engCfg = Engine::GetInstance( )->GetConfig( ); 

		String buildConfig = mEditor->GetBuildConfig( );
		String visualStudioPath = mEditor->GetVisualStudioDirectoryPath( );

		// Run the build bat for project
#ifdef ENJON_SYSTEM_WINDOWS 

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof( si ) );
		si.cb = sizeof( si );
		ZeroMemory( &pi, sizeof( pi ) );

		String path = Utils::FindReplaceAll( mProjectPath + "Proc\\\\CompileProject_Debug.bat", "/", "\\\\" ).c_str(); 
		String args = const_cast<LPSTR>( ( mProjectPath + " " + mProjectName + " " + buildConfig + " " + visualStudioPath ).c_str( ) ); 
		String cmdLineStr = ( "cmd.exe /c " + path + " " + args ); 

		char* cmdLineStrBuffer = new char[ cmdLineStr.size( ) + 1 ];
		strncpy( cmdLineStrBuffer, cmdLineStr.c_str( ), cmdLineStr.size( ) ); 
		cmdLineStrBuffer[ cmdLineStr.size( ) ] = '\0'; 

		// Start the child process. 
		//if ( !CreateProcess( path,   // No module name (use command line)
		if ( !CreateProcess( NULL,   // No module name (use command line)
			cmdLineStrBuffer,			// Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			CREATE_UNICODE_ENVIRONMENT,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi )           // Pointer to PROCESS_INFORMATION structure
			)
		{
			delete cmdLineStrBuffer;
			printf( "CreateProcess failed (%d).\n", GetLastError( ) );
			return Result::FAILURE;
		}

		// Wait for process to finish
		WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		delete cmdLineStrBuffer;
#endif
		
		return Result::SUCCESS; 
	} 

	//======================================================================

	void Project::Simluate( )
	{
#ifdef ENJON_SYSTEM_WINDOWS 
		// TODO(): Spawn up a separate thread to call this so we know when it's finished
		String cd = "pushd \"E:/Development/EnjonProjects/PongProject/Compiled/Windows/\"";
		s32 code = system( String( cd + "\ncall " + mProjectName + ".exe\n" + "popd\nexit 0" ).c_str() ); 
		//s32 code = system( String( cd ).c_str() ); 
		if ( code == 0 )
		{
			// Success...
		}

#endif

	}

	Result Project::BuildProject( )
	{
		if ( !mEditor )
		{
			std::cout << "EditorApp is null! Cannot continue!\n";
			return Result::FAILURE;
		}

		// Create build directory if not already made
		String buildDir = CreateBuildDirectory( );
		String enginePath = Utils::FindReplaceAll( Engine::GetInstance()->GetConfig().GetRoot( ), "\\", "/" );

#ifdef ENJON_SYSTEM_WINDOWS 
		// Run the bat file to build and run the solution
		String vsPath = mEditor->GetVisualStudioDirectoryPath( ); 

		s32 code = system( String( "call " + buildDir + "Intermediate/" + "Build.bat" + " " + mProjectName + " " + buildDir + "Intermediate/" + " " +  vsPath ).c_str() ); 

		// Copy the executable to the main project build directory
		if ( FileSystem::exists( buildDir + "Intermediate/Release/" + mProjectName + ".exe" ) )
		{
			// Remove previous instance
			if ( FileSystem::exists( buildDir + mProjectName + ".exe" ) )
			{
				FileSystem::remove( buildDir + mProjectName + ".exe" );
			}
			
			// Remove previous SDL2 .dll
			if ( FileSystem::exists( buildDir + "SDL2.dll" ) )
			{
				FileSystem::remove( buildDir + "SDL2.dll" );
			}
			// Remove previous glew32 .dlls
			if ( FileSystem::exists( buildDir + "glew32.dll" ) )
			{
				FileSystem::remove( buildDir + "glew32.dll" );
			}
			// Remove previous freetype6 .dlls
			if ( FileSystem::exists( buildDir + "freetype6.dll" ) )
			{
				FileSystem::remove( buildDir + "freetype6.dll" );
			}

			// Copy executable
			FileSystem::copy( buildDir + "Intermediate/Release/" + mProjectName + ".exe", buildDir + mProjectName + ".exe" );
			// Copy dlls
			FileSystem::copy( enginePath + "Build/Release/" + "SDL2.dll", buildDir + "SDL2.dll" );
			FileSystem::copy( enginePath + "Build/Release/" + "freetype6.dll", buildDir + "freetype6.dll" );
			FileSystem::copy( enginePath + "Build/Release/" + "glew32.dll", buildDir + "glew32.dll" ); 

			// Remove intermediate directory after compilation
			//FileSystem::remove_all( buildDir + "Intermediate" );
		}
#endif

		return Result::SUCCESS;
	}

	//======================================================================

	bool Project::IsLoaded( ) const
	{
		return mApp != nullptr;
	}

	//======================================================================

	void Project::LaunchApplication()
	{ 
#ifdef ENJON_SYSTEM_WINDOWS 
		String engineDir = Engine::GetInstance()->GetConfig().GetRoot();
		s32 code = system( String( "start " + engineDir + "/Build/Debug/Sandbox.exe " + mProjectPath ).c_str() ); 
#endif
	}

	//======================================================================
}







