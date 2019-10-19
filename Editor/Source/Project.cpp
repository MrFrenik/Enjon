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

namespace fs = ghc::filesystem; 

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
		fs::create_directories( buildDir );
		fs::create_directories( buildDir + "Assets/" );
		fs::create_directories( buildDir + "Intermediate/" );

		const ProjectSourceFileTemplates& templates = mEditor->GetProjectSourceFileTemplates();

		// Grab needed source templates from editor app that need to be compiled into project
		String mainTemplate = Utils::FindReplaceAll( templates.mProjectMainTemplate, "#PROJECT_NAME", mProjectName );

		// Write mainTemplate to file
		Utils::WriteToFile( mainTemplate, buildDir + "Intermediate/main.cpp" );

		// Get cmake file and replace necessary tags
		String cmakeFile = Enjon::Utils::FindReplaceAll( templates.mCompileProjectCMakeTemplate, "#PROJECTNAME", mProjectName );
		cmakeFile = Utils::FindReplaceAll( cmakeFile, "#ENJON_DIRECTORY", enginePath );
		cmakeFile = Utils::FindReplaceAll( cmakeFile, "#PROJECT_BUILD_DIR", buildDir + "Intermediate/" ); 
		cmakeFile = Utils::FindReplaceAll( cmakeFile, "#PROJECT_ROOT_DIR", mProjectPath );
		cmakeFile = Utils::FindReplaceAll( cmakeFile, "#CURRENT_SOURCE_DIR", buildDir );

		// Write all necessary intermediate files to directory for project build
		Enjon::Utils::WriteToFile( cmakeFile, buildDir + "Intermediate/CMakeLists.txt" ); 
		Enjon::Utils::WriteToFile( templates.mProjectBuildAndRunCompileTemplate, buildDir + "Intermediate/" + "Build.bat" ); 
		Enjon::Utils::WriteToFile( templates.mProjectEnjonDefinesTemplate, buildDir + "Intermediate/" + "Defines.h" ); 

		// Copy all Enjon assets into build directory as well
		String engineAssetsPath = enginePath + "Assets/";

		// Remove all previous content from directories
		fs::remove_all( buildDir + "Assets/Shaders" );
		fs::remove_all( buildDir + "Assets/Cache" );
		fs::remove_all( buildDir + "Assets/Fonts" );

		// Copy engine assets
		fs::copy( engineAssetsPath + "Shaders/", buildDir + "Assets/Shaders", fs::copy_options::recursive );
		fs::copy( engineAssetsPath + "Cache/", buildDir + "Assets/Cache", fs::copy_options::recursive );
		fs::copy( engineAssetsPath + "Fonts/", buildDir + "Assets/Fonts", fs::copy_options::recursive );

		// Copy project assets
		fs::copy( mProjectPath + "Assets/", buildDir + "Assets/Cache", fs::copy_options::recursive );

		return buildDir;

#ifdef ENJON_SYSTEM_WINDOWS 
		// Run the bat file to build and run the solution
		s32 code = system( String( "call \"" + buildDir + "Intermediate/" + "Build.bat" + "\" " + mProjectName + " \"" + buildDir + "Intermediate/\"" ).c_str() ); 

		// Copy the executable to the main project build directory
		if ( fs::exists( buildDir + "Intermediate/Release/" + mProjectName + ".exe" ) )
		{
			// Remove previous instance
			if ( fs::exists( buildDir + mProjectName + ".exe" ) )
			{
				fs::remove( buildDir + mProjectName + ".exe" );
			}
			
			// Remove previous SDL2 .dll
			if ( fs::exists( buildDir + "SDL2.dll" ) )
			{
				fs::remove( buildDir + "SDL2.dll" );
			}
			// Remove previous glew32 .dlls
			if ( fs::exists( buildDir + "glew32.dll" ) )
			{
				fs::remove( buildDir + "glew32.dll" );
			}
			// Remove previous freetype6 .dlls
			if ( fs::exists( buildDir + "freetype6.dll" ) )
			{
				fs::remove( buildDir + "freetype6.dll" );
			}

			// Copy executable
			fs::copy( buildDir + "Intermediate/Release/" + mProjectName + ".exe", buildDir + mProjectName + ".exe" );
			// Copy dlls
			fs::copy( enginePath + "Build/Release/" + "SDL2.dll", buildDir + "SDL2.dll" );
			fs::copy( enginePath + "Build/Release/" + "freetype6.dll", buildDir + "freetype6.dll" );
			fs::copy( enginePath + "Build/Release/" + "glew32.dll", buildDir + "glew32.dll" ); 

			// Remove intermediate directory after compilation
			//fs::remove_all( buildDir + "Intermediate" );
		}
#endif
	}

	//====================================================================== 

#include <exception>

	b32 Project::RegenerateProjectBuild()
	{ 
		const String& projectDir = mProjectPath;
		const String& projectName = mProjectName; 
		const ToolChainDefinition& tcDef = mToolChainDefinition; 
		const ProjectSourceFileTemplates& templates = mEditor->GetProjectSourceFileTemplates(); 

#ifdef ENJON_SYSTEM_WINDOWS 
		// Start the projection solution
		system( String( "call rmdir \"" + projectDir + "Proc/\" /s /q" ).c_str() ); 
		system( String( "call rmdir \"" + projectDir + "Build/\" /s /q" ).c_str() ); 
#endif 

		fs::create_directory( projectDir + "Proc/" );
		fs::create_directory( projectDir + "Build/" );
		fs::create_directory( projectDir + "Build/Generator/" );
		fs::create_directory( projectDir + "Build/Generator/Intermediate/" );
		fs::create_directory( projectDir + "Build/Generator/Linked/" ); 

		String cmakeFlags = tcDef.mCMakeGenerator;
		String releaseArgs = Utils::FindReplaceAll( 
									Utils::FindReplaceAll( tcDef.mArgs[ (u32)ConfigurationType::Release ], "${PROJ_OUTPUT_DIR}", projectDir + "Build/" ), 
									"${PROJ_NAME}", projectName ); 
		String debugArgs = Utils::FindReplaceAll( 
									Utils::FindReplaceAll( tcDef.mArgs[ (u32)ConfigurationType::Debug ], "${PROJ_OUTPUT_DIR}", projectDir + "Build/" ), 
									"${PROJ_NAME}", projectName );

		String afterBuildEvt = Utils::FindReplaceAll( tcDef.mAfterBuildEvent, "${PROJ_NAME}", projectName );

		String includeFile = Utils::FindReplaceAll( Utils::ParseFromTo( "#HEADERFILEBEGIN", "#HEADERFILEEND", templates.mProjectSourceTemplate, false ), "#PROJECTNAME", projectName );
		String sourceFile = Utils::FindReplaceAll( Utils::ParseFromTo( "#SOURCEFILEBEGIN", "#SOURCEFILEEND", templates.mProjectSourceTemplate, false ), "#PROJECTNAME", projectName ); 
		String delBatFile = templates.mProjectDelBatTemplate;
		String buildFile = Utils::FindReplaceAll(templates.mProjectBuildBatTemplate, "#CMAKE_FLAGS", cmakeFlags);
		String buildAndRunFIle = Utils::FindReplaceAll( Utils::FindReplaceAll(templates.mProjectBuildAndRunTemplate, "#CMAKE_FLAGS", cmakeFlags), "#AFTER_BUILD_EVT", afterBuildEvt );
		String compileReleaseFile = Utils::FindReplaceAll( 
										Utils::FindReplaceAll( 
										Utils::FindReplaceAll( templates.mCompileProjectBatTemplate, "#PROJ_NAME", projectName ), 
												"#PROJ_COMPILE_CMD", tcDef.mCommand + " " + releaseArgs), 
												"${PROJ_COMPILER_PATH}", tcDef.mCompilerPath);
		String compileDebugFile = Utils::FindReplaceAll( 
										Utils::FindReplaceAll( 
										Utils::FindReplaceAll( templates.mCompileProjectBatTemplate, "#PROJ_NAME", projectName ), 
												"#PROJ_COMPILE_CMD", tcDef.mCommand + " " + debugArgs ),
										"${PROJ_COMPILER_PATH}", tcDef.mCompilerPath );

		String cmakeFile = Utils::FindReplaceAll( Utils::FindReplaceAll( templates.mProjectCMakeTemplate, "#PROJECTNAME", projectName ), "#ENJONDIRECTORY", Utils::FindReplaceAll( Engine::GetInstance( )->GetConfig( ).GetRoot( ), "\\", "/" ) );
		String includes = "";
		for (auto& id : tcDef.mIncludeDirectories) {
			includes += id;
			includes += "\n";
		}
		cmakeFile = Utils::FindReplaceAll( cmakeFile, "#PROJ_INCLUDES", includes );
		//cmakeFile = Utils::FindReplaceAll( cmakeFile, "#PROJ_INC_DIR", projectConfig.mToolChain.mIncludeDirectories );

		// Write to file
		//Enjon::Utils::WriteToFile( includeFile, projectDir + "Source/" + projectName + ".h" );
		//Enjon::Utils::WriteToFile( sourceFile, projectDir + "Source/" + projectName + ".cpp" ); 
		Enjon::Utils::WriteToFile( cmakeFile, projectDir + "CMakeLists.txt" ); 
		Enjon::Utils::WriteToFile( delBatFile, projectDir + "Proc/" + "DelPDB.bat" ); 
		Enjon::Utils::WriteToFile( buildAndRunFIle, projectDir + "Proc/" + "BuildAndRun.bat" ); 
		Enjon::Utils::WriteToFile( buildFile, projectDir + "Proc/" + "Build.bat" ); 
		Enjon::Utils::WriteToFile( "", projectDir + "Build/Generator/Linked/" + projectName + "_Generated.cpp" ); 
		Enjon::Utils::WriteToFile( projectDir + "\n" + Engine::GetInstance()->GetConfig().GetRoot(), projectDir + projectName + ".eproj" );
		Enjon::Utils::WriteToFile( templates.mCompileProjectBatTemplate, projectDir + "Proc/" + "CompileProject.bat" ); 
		Enjon::Utils::WriteToFile( compileReleaseFile, projectDir + "Proc/" + "CompileProject_Release.bat" ); 
		Enjon::Utils::WriteToFile( compileDebugFile, projectDir + "Proc/" + "CompileProject_Debug.bat" ); 

		// Now call BuildAndRun.bat
#ifdef ENJON_SYSTEM_WINDOWS 
		// Start the projection solution
		s32 code = system( String( "call \"" + projectDir + "Proc/" + "BuildAndRun.bat\"" + " \"" + Enjon::Utils::FindReplaceAll( projectDir, "/", "\\" ) + "\" " + projectName ).c_str() ); 
		if ( code == 0 )
		{ 
			return true; 
		}
		else
		{
			std::cout << "Could not build project.\n";
			return false;
		}
#endif 

		
	}

	//====================================================================== 

	b32 Project::CreateNewProject( const ProjectConfig& projectConfig, const ProjectSourceFileTemplates& templates )
	{ 
		// Just output the source files for now... This is already going to get ugly, so need to split this all up pretty quickly
		const String& projectDir = projectConfig.mPath;
		const String& projectName = projectConfig.mName; 

		if ( !fs::exists( projectDir ) )
		{
			fs::create_directory( projectDir );
			fs::create_directory( projectDir + "Source/" );
			fs::create_directory( projectDir + "Assets/" );
			fs::create_directory( projectDir + "Proc/" );
			fs::create_directory( projectDir + "Build/" );
			fs::create_directory( projectDir + "Build/Generator/" );
			fs::create_directory( projectDir + "Build/Generator/Intermediate/" );
			fs::create_directory( projectDir + "Build/Generator/Linked/" );
		}

		//String cmakeFlags = mConfigSettings.mToolChains[(u32)mConfigSettings.mToolChainID]->mCMakeFlags;
		String cmakeFlags = projectConfig.mToolChain.mCMakeGenerator;
		String releaseArgs = Utils::FindReplaceAll( 
									Utils::FindReplaceAll( projectConfig.mToolChain.mArgs[ (u32)ConfigurationType::Release ], "${PROJ_OUTPUT_DIR}", projectDir + "Build/" ), 
									"${PROJ_NAME}", projectName ); 
		String debugArgs = Utils::FindReplaceAll( 
									Utils::FindReplaceAll( projectConfig.mToolChain.mArgs[ (u32)ConfigurationType::Debug ], "${PROJ_OUTPUT_DIR}", projectDir + "Build/" ), 
									"${PROJ_NAME}", projectName );

		String afterBuildEvt = Utils::FindReplaceAll( projectConfig.mToolChain.mAfterBuildEvent, "${PROJ_NAME}", projectName );

		String includeFile = Utils::FindReplaceAll( Utils::ParseFromTo( "#HEADERFILEBEGIN", "#HEADERFILEEND", templates.mProjectSourceTemplate, false ), "#PROJECTNAME", projectName );
		String sourceFile = Utils::FindReplaceAll( Utils::ParseFromTo( "#SOURCEFILEBEGIN", "#SOURCEFILEEND", templates.mProjectSourceTemplate, false ), "#PROJECTNAME", projectName ); 
		String delBatFile = templates.mProjectDelBatTemplate;
		String buildFile = Utils::FindReplaceAll(templates.mProjectBuildBatTemplate, "#CMAKE_FLAGS", cmakeFlags);
		String buildAndRunFIle = Utils::FindReplaceAll( Utils::FindReplaceAll(templates.mProjectBuildAndRunTemplate, "#CMAKE_FLAGS", cmakeFlags), "#AFTER_BUILD_EVT", afterBuildEvt );
		String compileReleaseFile = Utils::FindReplaceAll( 
										Utils::FindReplaceAll( 
										Utils::FindReplaceAll( templates.mCompileProjectBatTemplate, "#PROJ_NAME", projectName ), 
												"#PROJ_COMPILE_CMD", projectConfig.mToolChain.mCommand + " " + releaseArgs), 
												"${PROJ_COMPILER_PATH}", projectConfig.mToolChain.mCompilerPath);
		String compileDebugFile = Utils::FindReplaceAll( 
										Utils::FindReplaceAll( 
										Utils::FindReplaceAll( templates.mCompileProjectBatTemplate, "#PROJ_NAME", projectName ), 
												"#PROJ_COMPILE_CMD", projectConfig.mToolChain.mCommand + " " + debugArgs ),
										"${PROJ_COMPILER_PATH}", projectConfig.mToolChain.mCompilerPath );

		String cmakeFile = Utils::FindReplaceAll( Utils::FindReplaceAll( templates.mProjectCMakeTemplate, "#PROJECTNAME", projectName ), "#ENJONDIRECTORY", Utils::FindReplaceAll( Engine::GetInstance( )->GetConfig( ).GetRoot( ), "\\", "/" ) );
		String includes = "";
		for (auto& id : projectConfig.mToolChain.mIncludeDirectories) {
			includes += id;
			includes += "\n";
		}
		cmakeFile = Utils::FindReplaceAll( cmakeFile, "#PROJ_INCLUDES", includes );
		//cmakeFile = Utils::FindReplaceAll( cmakeFile, "#PROJ_INC_DIR", projectConfig.mToolChain.mIncludeDirectories );

		// Write to file
		Enjon::Utils::WriteToFile( includeFile, projectDir + "Source/" + projectName + ".h" );
		Enjon::Utils::WriteToFile( sourceFile, projectDir + "Source/" + projectName + ".cpp" ); 
		Enjon::Utils::WriteToFile( cmakeFile, projectDir + "CMakeLists.txt" ); 
		Enjon::Utils::WriteToFile( delBatFile, projectDir + "Proc/" + "DelPDB.bat" ); 
		Enjon::Utils::WriteToFile( buildAndRunFIle, projectDir + "Proc/" + "BuildAndRun.bat" ); 
		Enjon::Utils::WriteToFile( buildFile, projectDir + "Proc/" + "Build.bat" ); 
		Enjon::Utils::WriteToFile( "", projectDir + "Build/Generator/Linked/" + projectName + "_Generated.cpp" ); 
		Enjon::Utils::WriteToFile( projectDir + "\n" + Engine::GetInstance()->GetConfig().GetRoot(), projectDir + projectName + ".eproj" );
		Enjon::Utils::WriteToFile( templates.mCompileProjectBatTemplate, projectDir + "Proc/" + "CompileProject.bat" ); 
		Enjon::Utils::WriteToFile( compileReleaseFile, projectDir + "Proc/" + "CompileProject_Release.bat" ); 
		Enjon::Utils::WriteToFile( compileDebugFile, projectDir + "Proc/" + "CompileProject_Debug.bat" ); 

		// Now call BuildAndRun.bat
#ifdef ENJON_SYSTEM_WINDOWS 
		// Start the projection solution
		s32 code = system( String( "call \"" + projectDir + "Proc/" + "BuildAndRun.bat\"" + " \"" + Enjon::Utils::FindReplaceAll( projectDir, "/", "\\" ) + "\" " + projectName ).c_str() ); 
		if ( code == 0 )
		{ 
			return true; 
		}
		else
		{
			std::cout << "Could not build project.\n";
			return false;
		}
#else
			// Unload previous project
			UnloadDLL( ); 

			// Create new project
			Project proj( projectConfig );
			//proj.SetProjectPath( projectDir );
			//proj.SetProjectName( projectName );
			proj.SetEditor( this );

			// Compile the project
			proj.CompileProject( ); 

			// Add project to list
			mProjectsOnDisk.push_back( proj ); 

			// Load the new project
			LoadProject( proj ); 

			// Load the solution for the project
			LoadProjectSolution( );

			// Add project to project list
			mConfigSettings.mProjectList.push_back( proj );

			// Serialize editor configuration settings
			SerializeEditorConfigSettings();
#endif 
	}

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

		ConfigurationType configType = mEditor->GetConfigType();

		// Run the build bat for project
#ifdef ENJON_SYSTEM_WINDOWS 

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof( si ) );
		si.cb = sizeof( si );
		ZeroMemory( &pi, sizeof( pi ) );

		String configTypeStr = configType == ConfigurationType::Debug ? "Debug" : "Release"; 
		String path = Utils::FindReplaceAll( mProjectPath + "Proc\\\\CompileProject_" + configTypeStr + ".bat", "/", "\\\\" ).c_str(); 
		String args = const_cast<LPSTR>( ( mProjectPath + " " + mProjectName + " " + configTypeStr ).c_str( ) ); 
		String cmdLineStr = ( "cmd.exe /c \"" + path + "\" \"" + args + "\"" ); 

		s32 code = system( String( "call \"" + mProjectPath + "Proc/" + "CompileProject_" + configTypeStr + ".bat\"" + " \"" + Enjon::Utils::FindReplaceAll( mProjectPath, "/", "\\" ) + "\" " + mProjectName ).c_str() ); 

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
		//String cd = "pushd \"E:/Development/EnjonProjects/PongProject/Compiled/Windows/\"";
		//s32 code = system( String( cd + "\ncall " + mProjectName + ".exe\n" + "popd\nexit 0" ).c_str() ); 
		//s32 code = system( String( cd ).c_str() ); 
		//if ( code == 0 )
		//{
		//	// Success...
		//}

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
		s32 code = system( String( "call " + buildDir + "Intermediate/" + "Build.bat" + " " + mProjectName + " " + buildDir + "Intermediate/" ).c_str() ); 

		// Copy the executable to the main project build directory
		if ( fs::exists( buildDir + "Intermediate/Release/" + mProjectName + ".exe" ) )
		{
			// Remove previous instance
			if ( fs::exists( buildDir + mProjectName + ".exe" ) )
			{
				fs::remove( buildDir + mProjectName + ".exe" );
			}
			
			// Remove previous SDL2 .dll
			if ( fs::exists( buildDir + "SDL2.dll" ) )
			{
				fs::remove( buildDir + "SDL2.dll" );
			}
			// Remove previous glew32 .dlls
			if ( fs::exists( buildDir + "glew32.dll" ) )
			{
				fs::remove( buildDir + "glew32.dll" );
			}
			// Remove previous freetype6 .dlls
			if ( fs::exists( buildDir + "freetype6.dll" ) )
			{
				fs::remove( buildDir + "freetype6.dll" );
			}

			// Copy executable
			fs::copy( buildDir + "Intermediate/Release/" + mProjectName + ".exe", buildDir + mProjectName + ".exe" );
			// Copy dlls
			fs::copy( enginePath + "Build/Release/" + "SDL2.dll", buildDir + "SDL2.dll" );
			fs::copy( enginePath + "Build/Release/" + "freetype6.dll", buildDir + "freetype6.dll" );
			fs::copy( enginePath + "Build/Release/" + "glew32.dll", buildDir + "glew32.dll" ); 

			// Remove intermediate directory after compilation
			//fs::remove_all( buildDir + "Intermediate" );
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

	void Project::LaunchSandbox()
	{ 
#ifdef ENJON_SYSTEM_WINDOWS 
		String engineDir = Engine::GetInstance()->GetConfig().GetRoot();
		String editorDir = Engine::GetInstance()->GetConfig().GetRoot() + "/Editor";
		ConfigurationType configType = mEditor->GetConfigType();
		String config = configType == ConfigurationType::Debug ? "Debug" : "Release";
		String arg = "start \"\" \"" + engineDir + "/Build/" + config + "/Sandbox.exe\" \"" + mProjectPath + "\" " + mProjectName + "\"";
		s32 code = system( arg.c_str() ); 
#endif
	}

	//======================================================================

	void Project::KillSandbox()
	{
#ifdef ENJON_SYSTEM_WINDOWS 
		s32 code = system( "taskkill /IM \"Sandbox.exe\"" ); 
#endif 
	}

	//======================================================================

	b32 Project::IsSandboxRunning()
	{
#ifdef ENJON_SYSTEM_WINDOWS
		HWND hwnd;
		hwnd = FindWindow( NULL, "Sandbox.exe" );
		return ( hwnd != 0 ); 
#endif
	}

	//======================================================================

	void Project::OpenProjectFolder( )
	{
#ifdef ENJON_SYSTEM_WINDOWS
		String arg = "start \"\" \"" + mProjectPath + "\"";
		s32 code = system( arg.c_str() ); 
#endif
	}

	//======================================================================
}







