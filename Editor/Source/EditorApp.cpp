#include "EditorApp.h"
#include "EditorSceneView.h"

#include <Engine.h>
#include <Asset/AssetManager.h>
#include <Serialize/ObjectArchiver.h>
#include <Serialize/EntityArchiver.h>
#include <SubsystemCatalog.h>
#include <IO/InputManager.h>
#include <ImGui/ImGuiManager.h>
#include <Graphics/GraphicsSubsystem.h> 
#include <Graphics/Window.h>
#include <Entity/EntityManager.h>
#include <Physics/PhysicsSubsystem.h>
#include <Scene/SceneManager.h>
#include <Entity/Components/GraphicsComponent.h>
#include <Entity/Components/RigidBodyComponent.h>
#include <Entity/Components/PointLightComponent.h>
#include <Entity/Components/DirectionalLightComponent.h>
#include <Utils/FileUtils.h>
#include <Utils/Tokenizer.h>

#include <Bullet/btBulletDynamicsCommon.h> 

#include <windows.h>
#include <fmt/format.h>
#include <chrono>
#include <ctime>

typedef void( *funcSetEngineInstance )( Enjon::Engine* instance );
typedef Enjon::Application*( *funcCreateApp )( Enjon::Engine* );
typedef void( *funcDeleteApp )( Enjon::Application* ); 

// TODO(): Make sure to abstract this for platform independence
HINSTANCE dllHandleTemp = nullptr;
HINSTANCE dllHandle = nullptr;
funcSetEngineInstance setEngineFunc = nullptr;
funcCreateApp createAppFunc = nullptr;
funcDeleteApp deleteAppFunc = nullptr;

namespace fs = std::experimental::filesystem; 

Enjon::String projectName = "TestProject";
Enjon::String projectDLLName = projectName + ".dll";
Enjon::String copyDir = ""; 
Enjon::String mProjectsDir = "E:/Development/EnjonProjects/";
Enjon::String mVisualStudioDir = "\"E:\\Programs\\MicrosoftVisualStudio14.0\\\"";
//Enjon::String mProjectsDir = "W:/Projects/";
//Enjon::String mVisualStudioDir = "\"C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\\"";

Enjon::String configuration = "Release";
//Enjon::String configuration = "RelWithDebInfo";
//Enjon::String configuration = "Debug";

namespace Enjon
{
	void CopyLibraryContents( const String& projectName, const String& projectDir )
	{
		Enjon::String rootDir = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( );
		Enjon::String dllName =  projectName + ".dll";

		fs::path dllPath = rootDir + "Build/" + configuration + "/" + dllName;
		if ( fs::exists( dllPath ) )
		{
			fs::remove( dllPath );
		}

		// Now copy over contents from intermediate build to executable dir
		dllPath = projectDir;
		if ( fs::exists( dllPath ) )
		{
			if ( fs::exists( dllPath.string( ) + "Build/" + configuration + "/" + dllName ) )
			{
				fs::copy( fs::path( dllPath.string( ) + "Build/" + configuration + "/" + dllName ), rootDir + "Build/" + configuration + "/" + dllName );
			}
		}
	}

	void EditorApp::LoadProjectView( )
	{ 
		if ( !mPlaying )
		{ 
			const char* popupName = "Load Project##Modal";
			if ( !ImGui::IsPopupOpen( popupName ) )
			{
				ImGui::OpenPopup( popupName ); 
			}
			ImGui::SetNextWindowSize( ImVec2( 600.0f, 150.0f ) );
			if( ImGui::BeginPopupModal( popupName ) )
			{
					String defaultText = mProject.GetApplication( ) == nullptr ? "Existing Projects..." : mProject.GetProjectName( );
					if ( ImGui::BeginCombo( "##LOADPROJECTLIST", defaultText.c_str() ) )
					{
						for ( auto& p : mProjectsOnDisk )
						{
							if ( ImGui::Selectable( p.GetProjectName( ).c_str( ) ) )
							{ 
								// Load the project
								LoadProject( p );
								mLoadProjectPopupDialogue = false;
								ImGui::CloseCurrentPopup( );
							}
						}

						ImGui::EndCombo( );
					} 

					if ( ImGui::Button( "Cancel" ) )
					{
						mLoadProjectPopupDialogue = false;
						ImGui::CloseCurrentPopup( );
					}

				ImGui::EndPopup( );
			} 
		} 
		else
		{
			mLoadProjectPopupDialogue = false;
		}
	}

	void EditorApp::AddComponentPopupView( )
	{ 
		ImGui::SetNextWindowSize( ImVec2( 500.0f, 120.0f ) );

		if ( ImGui::BeginPopupModal( "Add C++ Component##NewComponent", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
		{ 
			// Get component list
			EntityManager* entities = EngineSubsystem( EntityManager );
			auto compMetaClsList = entities->GetComponentMetaClassList( );

			static String componentErrorMessage = "";
			static String componentName;
			bool closePopup = false;

			// Use to check if the component already exists
			auto alreadyExists = [ & ] ( const String& name ) -> bool
			{
				for ( auto& c : compMetaClsList )
				{
					if ( c->GetName( ).compare( name ) == 0 )
					{
						return true;
					} 
				}

				return false;
			};

			char buffer[ 256 ];
			strncpy( buffer, componentName.c_str( ), 256 );
			if ( ImGui::InputText( "Component Name", buffer, 256 ) )
			{
				// Reset component name
				componentName = String(buffer);
			} 

			bool compExists = true;
			bool isValidCPPClassName = true;

			if ( ImGui::Button( "Create" ) )
			{
				compExists = alreadyExists( componentName );
				isValidCPPClassName = Utils::IsValidCPPClassName( componentName );

				if ( !compExists && isValidCPPClassName )
				{
					// Get total count of entities
					UUID curUUID = mSelectedEntity.Get( )->GetUUID( );

					std::cout << "Creating component!\n";
					closePopup = true;
					CreateComponent( componentName ); 

					// Force add all the entities ( NOTE(): HATE THIS )
					EntityManager* em = EngineSubsystem( EntityManager );
					em->ForceAddEntities( );

					// Reselect entity by uuid
					SelectEntity( em->GetEntityByUUID( curUUID ) );

					// After creating new component, need to attach to entity
					if ( mSelectedEntity )
					{
						// Add using meta class 
						mSelectedEntity.Get( )->AddComponent( Object::GetClass( componentName ) );
					}
				}
				else if ( compExists )
				{ 
					componentErrorMessage = "Component already exists!";
				}
				else
				{
					componentErrorMessage = "Not a valid C++ class name!";
				}
			}

			ImGui::SameLine( );

			if ( ImGui::Button( "Cancel" ) )
			{
				closePopup = true;
			}

			if ( closePopup )
			{
				ImGui::CloseCurrentPopup( );
				mNewComponentPopupDialogue = false; 
				componentErrorMessage = "";
				componentName = "";
			}

			// If not empty string
			if ( componentErrorMessage.compare( "" ) != 0 )
			{
				ImGui::Text( componentErrorMessage.c_str() );
			}

			ImGui::EndPopup( );
		}
	}

	//==================================================================================================================

	void EditorApp::CreateComponent( const String& componentName )
	{ 
		// Replace meta tags with component name
		String includeFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::ParseFromTo( "#HEADERFILEBEGIN", "#HEADERFILEEND", mComponentSourceTemplate, false ), "#COMPONENTNAME", componentName );
		String sourceFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::ParseFromTo( "#SOURCEFILEBEGIN", "#SOURCEFILEEND", mComponentSourceTemplate, false ), "#COMPONENTNAME", componentName ); 

		// Need to copy the include files into the source directory for the project
		Utils::WriteToFile( includeFile, mProject.GetProjectPath( ) + "Source/" + componentName + ".h" );
		Utils::WriteToFile( sourceFile, mProject.GetProjectPath( ) + "Source/" + componentName + ".cpp" );

		String cmakePath = mProject.GetProjectPath( ) + "CMakeLists.txt";
		if ( fs::exists( cmakePath ) )
		{
			// Grab the content
			String cmakeContent = Utils::read_file_sstream( cmakePath.c_str() ); 
			// Resave it 
			Utils::WriteToFile( cmakeContent, cmakePath );
		} 

		// Compile the project
		mProject.CompileProject( );

		// Save scene
		SceneManager* sm = EngineSubsystem( SceneManager );
		if ( sm->GetScene() )
		{
			sm->GetScene()->Save( );
		}

		ReloadDLL( );
	}

	//==================================================================================================================

	void EditorApp::InspectorView( bool* enabled )
	{
		if ( mSelectedEntity )
		{
			// Debug dump the entity ( Probably shouldn't do this and should tailor it more... )
			Entity* ent = mSelectedEntity.Get( ); 

			// New component dialogue
			ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_20" ) );
			if ( ImGui::BeginCombo( "##ADDCOMPONENT", "+ Add Component..." ) )
			{
				// Label for scripting type of component class
				ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_10" ) );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
				ImGui::Text( "Scripting" );
				ImGui::PopStyleColor( );
				ImGui::PopFont( );

				// Add new component pop up
				ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_16" ) );
				if ( ImGui::Selectable( "\tCreate New Component..." ) )
				{
					mNewComponentPopupDialogue = true;
				}
				ImGui::PopFont( );

				// Separator line
				ImGui::Separator( );

				// Get component list
				EntityManager* entities = EngineSubsystem( EntityManager );
				auto compMetaClsList = entities->GetComponentMetaClassList( );

				// Label for scripting type of component class
				ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_10" ) );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
				ImGui::Text( "Custom" );
				ImGui::PopStyleColor( );
				ImGui::PopFont( );

				ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_16" ) );
				for ( auto& cls : compMetaClsList )
				{
					if ( !ent->HasComponent( cls ) )
					{
						// Add component to mEntity
						if ( ImGui::Selectable( fmt::format( "\t{}", cls->GetName( ) ).c_str() ) )
						{
							ent->AddComponent( cls );
						} 
					}
				} 
				ImGui::PopFont( );

				ImGui::EndCombo( );
			}
			ImGui::PopFont( ); 

			// Add new component dialogue window
			if ( mNewComponentPopupDialogue )
			{
				ImGui::OpenPopup( "Add C++ Component##NewComponent" );
				AddComponentPopupView( );
			}

			// Transform information
			if ( ImGui::CollapsingHeader( "Transform" ) )
			{
				ImGuiManager::DebugDumpProperty( ent, ent->Class( )->GetPropertyByName( "mLocalTransform" ) ); 
			}


			for ( auto& c : ent->GetComponents( ) )
			{
				if ( ImGui::CollapsingHeader( c->Class( )->GetName( ).c_str( ) ) )
				{
					MetaClass* cls = const_cast< MetaClass * > ( c->Class( ) );
					s32 shapeType = -1;
					if ( cls->InstanceOf<RigidBodyComponent>( ) )
					{
						shapeType = (s32)c->Cast<RigidBodyComponent>( )->GetShapeType( );
					}

					ImGuiManager::DebugDumpObject( c ); 

					// Shape type changed
					// TODO(): Be able to hook in specific delegates for property changes through reflection system
					if ( shapeType != -1 && shapeType != (s32)c->Cast< RigidBodyComponent >()->GetShapeType() )
					{
						c->ConstCast< RigidBodyComponent >( )->SetShape( c->Cast< RigidBodyComponent >( )->GetShapeType( ) );
					} 

					if ( ImGui::Button( fmt::format( "Remove##{}", (u32)c ).c_str() ) )
					{
						ent->RemoveComponent( c->Class( ) );
					}
				}
			}
		}
	}

	//===========================================================================================

	EditorWidgetManager* EditorApp::GetEditorWidgetManager( )
	{
		return &mEditorWidgetManager;
	}

	//===========================================================================================

	void EditorApp::SceneView( bool* viewBool )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		u32 currentTextureId = gfx->GetCurrentRenderTextureId( ); 

		// Render game in window
		ImVec2 cursorPos = ImGui::GetCursorScreenPos( );

		// Cache off cursor position for scene view
		mSceneViewWindowPosition = Vec2( cursorPos.x, cursorPos.y );
		mSceneViewWindowSize = Vec2( ImGui::GetWindowWidth( ), ImGui::GetWindowHeight( ) );

		ImTextureID img = ( ImTextureID )currentTextureId;
		ImGui::Image( img, ImVec2( ImGui::GetWindowWidth( ), ImGui::GetWindowHeight( ) ),
			ImVec2( 0, 1 ), ImVec2( 1, 0 ), ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 0 ) );

		ImVec2 min = ImVec2( cursorPos.x + ImGui::GetContentRegionAvailWidth( ) - 100.0f, cursorPos.y + 10.0f );
		ImVec2 max = ImVec2( min.x + 50.0f, min.y + 10.0f );

		ImGui::SetCursorScreenPos( min );
		auto drawlist = ImGui::GetWindowDrawList( );
		f32 fps = ImGui::GetIO( ).Framerate;

		// Update camera aspect ratio
		gfx->GetGraphicsSceneCamera( )->ConstCast< Enjon::Camera >( )->SetAspectRatio( ImGui::GetWindowWidth( ) / ImGui::GetWindowHeight( ) );
	}

	Vec2 EditorApp::GetSceneViewProjectedCursorPosition( )
	{
		return mEditorSceneView->GetSceneViewProjectedCursorPosition( );
	}

	void EditorApp::CameraOptions( bool* enable )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		const Enjon::Camera* cam = gfx->GetGraphicsSceneCamera( );

		if ( ImGui::TreeNode( "Camera" ) )
		{
			Enjon::ImGuiManager::DebugDumpObject( cam ); 
			ImGui::TreePop( );
		}

		ImGui::DragFloat( "Camera Speed", &mCameraSpeed, 0.01f, 0.01f, 100.0f ); 
		ImGui::DragFloat( "Mouse Sensitivity", &mMouseSensitivity, 0.1f, 0.1f, 10.0f ); 
		f32 col[ 4 ] = { mRectColor.x, mRectColor.y, mRectColor.z, mRectColor.w };
		ImGui::DragFloat4( "Rect Color", col, 0.01f, 0.0f, 1.0f );
		mRectColor = Vec4( col[ 0 ], col[ 1 ], col[ 2 ], col[ 3 ] );

		if ( ImGui::TreeNode( "Application" ) )
		{
			if ( mProject.GetApplication() )
			{
				Enjon::ImGuiManager::DebugDumpObject( mProject.GetApplication() ); 
			}
			ImGui::TreePop( );
		} 

		if ( ImGui::TreeNode( "World Time" ) )
		{
			WorldTime wt = Engine::GetInstance( )->GetWorldTime( ); 
			if ( ImGui::SliderFloat( "Time Scale", &wt.mTimeScale, 0.001, 1.0f ) )
			{
				Engine::GetInstance( )->SetWorldTimeScale( wt.mTimeScale );
			}
			ImGui::TreePop( );
		}
	}

	void EditorApp::PlayOptions( )
	{
		if ( mPlaying )
		{
			if ( ImGui::Button( "Stop" ) )
			{ 
				mPlaying = false; 
				mMoveCamera = false;

				// Call shut down function for game
				if ( mProject.GetApplication() )
				{
					// Need to pass in previous scene to restore...
					ShutdownProjectApp( nullptr );
				}

				GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
				auto cam = gfx->GetGraphicsSceneCamera( )->ConstCast< Camera >();
				cam->SetPosition( mPreviousCameraTransform.GetPosition() );
				cam->SetRotation( mPreviousCameraTransform.GetRotation() ); 
			}

			static bool isPaused = false;

			if ( !isPaused )
			{
				ImGui::SameLine( );
				if ( ImGui::Button( "Pause" ) )
				{
					isPaused = true;
					PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );
					physx->PauseSystem( true );
				} 
			} 
			else
			{
				ImGui::SameLine( );
				if ( ImGui::Button( "Resume" ) )
				{
					isPaused = false;
					PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );
					physx->PauseSystem( false );
				} 
			}
		}
		else
		{
			if ( ImGui::Button( "Play" ) )
			{ 
				mPlaying = true;
				mMoveCamera = true;

				GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
				auto cam = gfx->GetGraphicsSceneCamera( );
				mPreviousCameraTransform = Enjon::Transform( cam->GetPosition(), cam->GetRotation(), Enjon::Vec3( cam->GetOrthographicScale() ) ); 

				// Call start up function for game
				if ( mProject.GetApplication() )
				{
					InitializeProjectApp( );
				}
				else
				{ 
					std::cout << "Cannot play without game loaded!\n";
					mPlaying = false;
					mMoveCamera = false;
				}
			}

			ImGui::SameLine( );
			if ( ImGui::Button( "Reload" ) )
			{ 
				// Reload the dll
				ReloadDLL( );
			}

			// Compile the project if available
			if ( mProject.GetApplication( ) )
			{
				ImGui::SameLine( ); 
				if ( ImGui::Button( "Compile" ) )
				{
					Result res = mProject.CompileProject( );

					AssetHandle< Scene > scene = EngineSubsystem( SceneManager )->GetScene( );

					// Save the scene
					if ( scene )
					{
						scene->Save( );
					}

					// Force reload the project after successful compilation
					if ( res == Result::SUCCESS )
					{
						ReloadDLL( );
					}
				} 

				ImGui::SameLine( );
				if ( ImGui::Button( "Build" ) )
				{
					Result res = mProject.BuildProject( );
				}

				ImGui::SameLine( );
				if ( ImGui::Button( "Simulate" ) )
				{
					Result res = mProject.BuildProject( );
					if ( res == Result::SUCCESS )
					{
						mProject.Simluate( );
					} 
				}
			}
		} 
	}

	void EditorApp::SelectEntity( const EntityHandle& handle )
	{
		mSelectedEntity = handle;

		if ( mSelectedEntity )
		{
			// Enable transform widget
			mTransformWidget.Enable( true );

			// Set transform to selected entity
			mTransformWidget.SetPosition( mSelectedEntity.Get( )->GetWorldPosition( ) ); 
			mTransformWidget.SetRotation( mSelectedEntity.Get( )->GetWorldRotation( ) ); 
		} 
	}

	void EditorApp::DeselectEntity( )
	{
		// Set to invalid entity handle
		mSelectedEntity = EntityHandle::Invalid();

		// Deactivate transform widget
		mTransformWidget.Enable( false );
	}

	void EditorApp::WorldOutlinerView( )
	{ 
		AssetHandle< Scene > scene = EngineSubsystem( SceneManager )->GetScene( );

		if ( !scene )
		{
			return;
		}

		EntityManager* entities = EngineSubsystem( EntityManager );

		// Print out scene name
		ImGui::Text( ( "Scene: " + scene->GetName( ) ).c_str( ) );

		ImGui::Separator( );

		// List out active entities
		for ( auto& e : entities->GetActiveEntities( ) )
		{
			if ( ImGui::Selectable( fmt::format( "{}", e->GetID() ).c_str( ) ) )
			{
				SelectEntity( e );
			} 
		}
	}

	void EditorApp::LoadResourceFromFile( )
	{
		// Load file
		if ( ImGui::CollapsingHeader( "Load Resource" ) )
		{
			char buffer[ 256 ];
			std::strncpy( buffer, mResourceFilePathToLoad.c_str( ), 256 );
			if ( ImGui::InputText( "File Path", buffer, 256 ) )
			{
				mResourceFilePathToLoad = Enjon::String( buffer );
			}

			if ( ImGui::Button( "Load File" ) )
			{
				Enjon::AssetManager* am = EngineSubsystem( AssetManager );
				am->AddToDatabase( mResourceFilePathToLoad );
			}
		}
	}

	//================================================================================================================================

	void EditorApp::SelectSceneView( )
	{
		SceneManager* sm = EngineSubsystem( SceneManager );
		AssetHandle< Scene > scene = sm->GetScene( ); 
		String defaultText = scene.Get( ) == nullptr ? "Available Scenes..." : scene->GetName( );
		if ( ImGui::BeginCombo( "##Available Scenes", defaultText.c_str() ) )
		{ 
			// If there's a valid application set
			if ( mProject.GetApplication( ) )
			{
				// Get component list
				AssetManager* am = EngineSubsystem( AssetManager );
				const HashMap<String, AssetRecordInfo>* scenes = am->GetAssets<Scene>( );

				for ( auto& record : *scenes )
				{ 
					// Add component to mEntity
					if ( ImGui::Selectable( record.second.GetAssetName( ).c_str( ) ) )
					{ 
						// Get uuid of asset
						UUID uuid = record.second.GetAssetUUID( );

						// Load the scene
						sm->LoadScene( uuid ); 

						// Deselect entity
						DeselectEntity( );
					} 
				} 
			}
			else
			{
				ImGui::Text( "No project loaded." );
			}

			ImGui::EndCombo( ); 
		}

		if ( sm->GetScene() )
		{
			if ( ImGui::Button( "Save Scene" ) )
			{
				sm->GetScene()->Save( );
			}
		}
	}

	//================================================================================================================================
	 
	void EditorApp::CreateNewProject( const String& projectName )
	{ 
		// Just output the source files for now... This is already going to get ugly, so need to split this all up pretty quickly
		String projectDir = mProjectsDir + projectName + "/";
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

		String includeFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::ParseFromTo( "#HEADERFILEBEGIN", "#HEADERFILEEND", mProjectSourceTemplate, false ), "#PROJECTNAME", projectName );
		String sourceFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::ParseFromTo( "#SOURCEFILEBEGIN", "#SOURCEFILEEND", mProjectSourceTemplate, false ), "#PROJECTNAME", projectName ); 
		String cmakeFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::FindReplaceAll( mProjectCMakeTemplate, "#PROJECTNAME", projectName ), "#ENJONDIRECTORY", Enjon::Utils::FindReplaceAll( Engine::GetInstance( )->GetConfig( ).GetRoot( ), "\\", "/" ) );
		String delBatFile = mProjectDelBatTemplate;
		String buildAndRunFIle = mProjectBuildAndRunTemplate;

		// Write to file
		Enjon::Utils::WriteToFile( includeFile, projectDir + "Source/" + projectName + ".h" );
		Enjon::Utils::WriteToFile( sourceFile, projectDir + "Source/" + projectName + ".cpp" ); 
		Enjon::Utils::WriteToFile( cmakeFile, projectDir + "CMakeLists.txt" ); 
		Enjon::Utils::WriteToFile( delBatFile, projectDir + "Proc/" + "DelPDB.bat" ); 
		Enjon::Utils::WriteToFile( buildAndRunFIle, projectDir + "Proc/" + "BuildAndRun.bat" ); 
		Enjon::Utils::WriteToFile( mProjectBuildBatTemplate, projectDir + "Proc/" + "Build.bat" ); 
		Enjon::Utils::WriteToFile( "", projectDir + "Build/Generator/Linked/" + projectName + "_Generated.cpp" ); 
		Enjon::Utils::WriteToFile( projectDir, projectDir + projectName + ".eproj" );
		Enjon::Utils::WriteToFile( mCompileProjectBatTemplate, projectDir + "Proc/" + "CompileProject.bat" );


		// Now call BuildAndRun.bat
#ifdef ENJON_SYSTEM_WINDOWS 
		// Start the projection solution
		s32 code = system( String( "call " + projectDir + "Proc/" + "Build.bat" + " " + Enjon::Utils::FindReplaceAll( projectDir, "/", "\\" ) + " " + projectName ).c_str() ); 
		if ( code == 0 )
		{
			// Unload previous project
			UnloadDLL( ); 

			// Create new project
			Project proj; 
			proj.SetProjectPath( projectDir );
			proj.SetProjectName( projectName );
			proj.SetEditor( this );

			// Compile the project
			proj.CompileProject( ); 

			// Add project to list
			mProjectsOnDisk.push_back( proj ); 

			// Load the new project
			LoadProject( proj ); 

			// Load the solution for the project
			LoadProjectSolution( );
		}
		else
		{
			std::cout << "Could not build project.\n";
		}
#endif


	}

	//================================================================================================================================

	void EditorApp::LoadProject( const Project& project )
	{ 
		SceneManager* sm = EngineSubsystem( SceneManager );

		sm->UnloadScene( );

		// Unload previous dll
		UnloadDLL( );

		// Set project
		mProject = project;

		// Load project dll
		LoadDLL( );

		// Reinitialize asset manager
		AssetManager* am = EngineSubsystem( AssetManager );
		am->Reinitialize( mProject.GetProjectPath( ) + "Assets/" ); 
	}

	//================================================================================================================================

	void EditorApp::LoadProjectSolution( )
	{
		// Now call BuildAndRun.bat
#ifdef ENJON_SYSTEM_WINDOWS 
		// TODO(): Error check the fuck out of this call
		// Is it possible to know whether or not this succeeded?
		s32 code = system( String( "start " + mProject.GetProjectPath() + "Build/" + mProject.GetProjectName() + ".sln" ).c_str() ); 
#endif
	}

	//================================================================================================================================

	void EditorApp::CreateProjectView( )
	{
		char buffer[ 256 ];
		strncpy( buffer, mNewProjectName.c_str( ), 256 );
		if ( ImGui::InputText( "Project Name", buffer, 256 ) )
		{
			mNewProjectName = String( buffer );
		}

		if ( ImGui::Button( "Create New Project" ) && mNewProjectName.compare( "" ) != 0 )
		{
			// If project is able to be made, then make it
			String projectPath = mProjectsDir + "/" + mNewProjectName + "/";
			if ( !fs::exists( projectPath ) )
			{
				CreateNewProject( mNewProjectName );
			}
			else
			{
				std::cout << "Project already exists!\n";
			} 
		}

		if ( !mPlaying )
		{ 
			// Load visual studio project 
			if ( ImGui::Button( "Load Project Solution" ) )
			{
				if ( fs::exists( mProject.GetProjectPath( ) + "Build/" + mProject.GetProjectName( ) + ".sln" ) )
				{ 
					LoadProjectSolution( );
				}
			}
		}

	}

	//================================================================================================================================

	void EditorApp::CleanupScene( )
	{ 
		// Force the scene to clean up ahead of frame
		EntityManager* entities = EngineSubsystem( EntityManager );
		entities->ForceCleanup( );
	}

	//================================================================================================================================

	bool EditorApp::UnloadDLL( ByteBuffer* buffer )
	{
		bool needsReload = false;

		if ( dllHandle )
		{
			if ( mPlaying && buffer )
			{
				Application* app = mProject.GetApplication( );
				if ( app )
				{
					Enjon::ObjectArchiver::Serialize( app, buffer );
					needsReload = true;

					// Shutdown project app
					ShutdownProjectApp( buffer );
				}
			}

			// Free application memory
			if ( deleteAppFunc )
			{
				deleteAppFunc( mProject.GetApplication( ) );
				mProject.SetApplication( nullptr );
			}

			// Free library if in use
			FreeLibrary( dllHandle );
			dllHandle = nullptr;
			createAppFunc = nullptr;
			deleteAppFunc = nullptr;
		}

		return needsReload;
	}

	//================================================================================================================================

	void EditorApp::ReloadDLL( )
	{
		// Save the current scene and store uuid
		UUID uuid;
		SceneManager* sm = EngineSubsystem( SceneManager );
		if ( sm->GetScene() )
		{
			uuid = sm->GetScene()->GetUUID( );
			sm->UnloadScene( );
		} 

		// ReloadDLL without release scene asset
		LoadDLL( false ); 

		// Set current scene using previous id if valid
		if ( uuid )
		{
			sm->LoadScene( uuid );
		}
	}

	//================================================================================================================================

	void EditorApp::LoadDLL( bool releaseSceneAsset )
	{
		Enjon::ByteBuffer buffer;

		// Unload previous scene
		EngineSubsystem( SceneManager )->UnloadScene( );
		// Unload previous scene 
		//UnloadScene( releaseSceneAsset ); 

		// Actual code starts here...
		bool needsReload = UnloadDLL( &buffer );

		// Copy files to directory
		CopyLibraryContents( mProject.GetProjectName(), mProject.GetProjectPath() );

		// Try to load library
		dllHandle = LoadLibrary( ( mProject.GetProjectName() + ".dll" ).c_str() );

		// If valid, then set address of procedures to be called
		if ( dllHandle )
		{
			// Set functions from handle
			createAppFunc = ( funcCreateApp )GetProcAddress( dllHandle, "CreateApplication" );
			deleteAppFunc = ( funcDeleteApp )GetProcAddress( dllHandle, "DeleteApplication" );

			// Create application
			if ( createAppFunc )
			{
				mProject.SetApplication( createAppFunc( Enjon::Engine::GetInstance( ) ) );
			} 

			// Reload the entity from the buffer
			if ( buffer.GetSize( ) )
			{
				// Deserialize scene entity
				mSceneEntity = Enjon::EntityArchiver::Deserialize( &buffer ); 

				// Push back scene entity into mSceneEntities
				mSceneEntities.push_back( mSceneEntity.Get( ) );
			}

			if ( needsReload )
			{
				Application* app = mProject.GetApplication( );
				if ( app )
				{
					std::cout << "Buffer size: " << buffer.GetSize( ) << "\n";
					Enjon::ObjectArchiver::Deserialize( &buffer, app ); 

					for ( u32 i = 0; i < 30; ++i )
					{
						// Write this out to file for shiggles
						buffer.WriteToFile( mProject.GetProjectPath( ) + "/testWriteBuffer" + fmt::format("{}", i ) ); 
					}
				}
			}
		}
		else
		{
			// Reload the entity from the buffer
			if ( buffer.GetSize( ) )
			{
				// Deserialize scene entity
				mSceneEntity = Enjon::EntityArchiver::Deserialize( &buffer );

				// Push back scene entity into mSceneEntities
				mSceneEntities.push_back( mSceneEntity.Get( ) );
			}

			std::cout << "Could not load library\n";
		}

		// Reload current scene
		SceneManager* sm = EngineSubsystem( SceneManager );
		sm->ReloadScene( );
	}

	//================================================================================================================================

	String EditorApp::GetBuildConfig( ) const
	{
		return configuration;
	}

	//================================================================================================================================

	String EditorApp::GetVisualStudioDirectoryPath( ) const
	{
		return mVisualStudioDir;
	}

	//================================================================================================================================

	String EditorApp::GetCompileProjectCMakeTemplate( ) const
	{
		return mCompileProjectCMakeTemplate;
	}

	//================================================================================================================================

	String EditorApp::GetBuildAndRunCompileTemplate( ) const
	{
		return mProjectBuildAndRunCompileTemplate;
	}

	//================================================================================================================================

	String EditorApp::GetProjectEnjonDefinesTemplate( ) const
	{
		return mProjectEnjonDefinesTemplate;
	}

	//================================================================================================================================

	String EditorApp::GetProjectMainTemplate( ) const
	{
		return mProjectMainTemplate;
	} 

	//================================================================================================================================

	void EditorApp::CollectAllProjectsOnDisk( )
	{ 
		for ( auto& p : fs::recursive_directory_iterator( mProjectsDir ) )
		{
			if ( Enjon::Utils::HasFileExtension( p.path( ).string( ), "eproj" ) )
			{ 
				Project proj;
				String path = "";
				Vector<String> split = Utils::SplitString( Utils::FindReplaceAll( Utils::SplitString( p.path( ).string( ), "." ).front( ), "\\", "/" ), "/" );
				split.pop_back( );
				for ( auto& p : split )
				{
					path += p + "/";
				}

				proj.SetProjectPath( path );
				proj.SetProjectName( Enjon::Utils::SplitString( Enjon::Utils::SplitString( p.path( ).string( ), "\\" ).back(), "." ).front() );
				proj.SetEditor( this );
				mProjectsOnDisk.push_back( proj );
			}
		} 
	}

	//================================================================================================================================

	void EditorApp::InitializeProjectApp( )
	{
		// Get project application
		Application* app = mProject.GetApplication( );

		// Get current loaded scene
		AssetHandle< Scene > scene = EngineSubsystem( SceneManager )->GetScene( );

		// If both are loaded then can initialize application
		if ( app && scene )
		{
			// Set application state to running
			SetApplicationState( ApplicationState::Running );

			// Cache off all entity handles in scene before app starts
			EntityManager* em = EngineSubsystem( EntityManager );
			mSceneEntities = em->GetActiveEntities( );

			// NOTE(): Don't really like this at all...
			// Save current scene before starting so we can reload on stop 
			scene.Save( );

			// Initialize the app
			app->Initialize( );

			// Turn on the physics simulation
			PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );
			physx->PauseSystem( false ); 

			// Turn off the selection widget
			mTransformWidget.Enable( false );
		}
	}

	//================================================================================================================================

	void EditorApp::ShutdownProjectApp( ByteBuffer* buffer )
	{
		Application* app = mProject.GetApplication( );
		if ( app )
		{
			// Shutdown application state
			SetApplicationState( ApplicationState::Stopped );

			EntityManager* em = EngineSubsystem( EntityManager );
			SceneManager* sm = EngineSubsystem( SceneManager );
			Vector<Entity*> entities = em->GetActiveEntities( ); 

			// Shutodwn the application
			app->Shutdown( ); 

			// Unload current scene and catch its uuid
			UUID uuid = sm->UnloadScene( );

			// Reload scene with previous uuid
			sm->LoadScene( uuid );

			// Clean up physics subsystem from contact events as well
			PhysicsSubsystem* phys = EngineSubsystem( PhysicsSubsystem );
			phys->Reset( );

			// Pause the physics simulation
			phys->PauseSystem( true ); 
		}

		// Set the active camera in graphics scene to editor camera
		if ( mEditorCamera.GetGraphicsScene( ) )
		{
			mEditorCamera.GetGraphicsScene( )->SetActiveCamera( &mEditorCamera );
		}
	}
	 
	Enjon::Result EditorApp::Initialize( )
	{ 
		mApplicationName = "EditorApp"; 

		// Set application state to stopped by default
		SetApplicationState( ApplicationState::Stopped );

		Enjon::String mAssetsDirectoryPath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + "Editor/Assets/";

		// Get asset manager and set its properties ( I don't like this )
		AssetManager* mAssetManager = EngineSubsystem( AssetManager );
		GraphicsSubsystem* mGfx = EngineSubsystem( GraphicsSubsystem );
		PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem ); 

		// Set up camera and then add to graphics scene
		mEditorCamera = Camera( mGfx->GetViewport() );
		mEditorCamera.SetNearFar( 0.01f, 1000.0f );
		mEditorCamera.SetProjection(ProjectionType::Perspective);
		mEditorCamera.SetPosition(Vec3(0, 5, 10)); 
		mGfx->GetGraphicsScene( )->AddCamera( &mEditorCamera );
		mGfx->GetGraphicsScene()->SetActiveCamera( &mEditorCamera );

		// Pause the physics simulation
		physx->PauseSystem( true ); 

		// Register project template files
		mProjectSourceTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectSourceTemplate.cpp" ).c_str() ); 
		mProjectCMakeTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectCMakeTemplate.txt" ).c_str( ) );
		mProjectDelBatTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/DelPDB.bat" ).c_str( ) );
		mProjectBuildAndRunTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/BuildAndRun.bat" ).c_str( ) ); 
		mProjectBuildBatTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/Build.bat" ).c_str( ) ); 
		mComponentSourceTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ComponentSourceTemplate.cpp" ).c_str( ) ); 
		mCompileProjectBatTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/CompileProject.bat" ).c_str( ) ); 
		mCompileProjectCMakeTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectCompileCMakeTemplate.txt" ).c_str( ) ); 
		mProjectMainTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectAppMain.cpp" ).c_str( ) ); 
		mProjectBuildAndRunCompileTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/BuildAndRunCompile.bat" ).c_str( ) ); 
		mProjectEnjonDefinesTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectEnjonDefines.h" ).c_str( ) ); 

		// Set up copy directory for project dll
		copyDir = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( ) + projectName + "/";

		// Grab all .eproj files and store them for loading later
		CollectAllProjectsOnDisk( ); 

		LoadResources( );

		// Add all necessary views into editor widget manager
		mEditorWidgetManager.AddView( new EditorSceneView( this ) );

		// Initialize transform widget
		mTransformWidget.Initialize( this ); 

		// Register individual windows
		Enjon::ImGuiManager::RegisterWindow( [ & ] ( )
		{
			// Docking windows
			if ( ImGui::BeginDock( "Create Project", &mShowCreateProjectView, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
			{
				// Print docking information
				CreateProjectView( );
			}
			ImGui::EndDock( );
		} );

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "Camera", &mShowCameraOptions ) )
			{
				CameraOptions( &mShowCameraOptions );
			}
			ImGui::EndDock( );
		});

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "Load Resource", &mShowLoadResourceOption ) )
			{
				LoadResourceFromFile( );
			}
			ImGui::EndDock( );
		});

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "World Outliner", nullptr ) )
			{
				WorldOutlinerView( );
			}
			ImGui::EndDock( );
		});

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "Play Options", nullptr ) )
			{
				PlayOptions( );
			}
			ImGui::EndDock( );
		} );

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "Inspector View", nullptr ) )
			{
				InspectorView( nullptr );
			}
			ImGui::EndDock( );
		} );

		static bool sceneSelectionViewOpen = true;
		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "Scene Selection View", &sceneSelectionViewOpen ) )
			{
				SelectSceneView( );
				CheckForPopups( );
			}
			ImGui::EndDock( );
		} ); 

		auto createViewOption = [&]()
		{
			if ( mProject.GetApplication() && !mPlaying )
			{
				if ( ImGui::MenuItem( "Empty##options", NULL ) )
				{
					std::cout << "Creating empty entity...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					EntityHandle empty = em->Allocate( );

					// Set to selected entity
					SelectEntity( empty );
				}

				if ( ImGui::MenuItem( "Cube##options", NULL ) )
				{
					std::cout << "Creating cube...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					AssetManager* am = EngineSubsystem( AssetManager );
					GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
					EntityHandle cube = em->Allocate( );
					if ( cube )
					{
						Entity* ent = cube.Get( );
						GraphicsComponent* gfx = ent->AddComponent<GraphicsComponent>( );
						gfx->SetMesh( am->GetAsset< Mesh >( "models.unit_cube" ) );
						gfx->SetMaterial( am->GetDefaultAsset<Material>( ) );

						RigidBodyComponent* rbc = ent->AddComponent<RigidBodyComponent>( );
						rbc->SetShape( CollisionShapeType::Box ); 

						const Camera* cam = gs->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );
					}

					// Select entity
					SelectEntity( cube );
				}

				if ( ImGui::MenuItem( "Sphere##options", NULL ) )
				{
					std::cout << "Creating sphere...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					AssetManager* am = EngineSubsystem( AssetManager );
					GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
					EntityHandle sphere = em->Allocate( );
					if ( sphere )
					{
						Entity* ent = sphere.Get( );
						GraphicsComponent* gfx = ent->AddComponent<GraphicsComponent>( );
						gfx->SetMesh( am->GetAsset< Mesh >( "models.unit_sphere" ) );
						gfx->SetMaterial( am->GetDefaultAsset<Material>( ) );

						RigidBodyComponent* rbc = ent->AddComponent< RigidBodyComponent >( );
						rbc->SetShape( CollisionShapeType::Sphere );

						const Camera* cam = gs->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );
					}

					// Select entity
					SelectEntity( sphere );
				}

				if ( ImGui::MenuItem( "Point Light##options", NULL ) )
				{
					std::cout << "Creating point light...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
					EntityHandle pointLight = em->Allocate( );
					if ( pointLight )
					{
						Entity* ent = pointLight.Get( ); 
						PointLightComponent* plc = ent->AddComponent<PointLightComponent>( );

						const Camera* cam = gs->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );
					}

					// Select entity
					SelectEntity( pointLight );
				}

				if ( ImGui::MenuItem( "Directional Light##options", NULL ) )
				{
					std::cout << "Creating directional light...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
					EntityHandle directionalLight = em->Allocate( );
					if ( directionalLight )
					{
						Entity* ent = directionalLight.Get( ); 
						ent->AddComponent<DirectionalLightComponent>( );

						const Camera* cam = gs->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );
					}

					// Select entity
					SelectEntity( directionalLight );
				}

				if ( ImGui::MenuItem( "Scene##options", NULL ) )
				{
					// Construct scene and save it
					AssetManager* am = EngineSubsystem( AssetManager );
					AssetHandle< Scene > scene = am->ConstructAsset< Scene >( );

					// Unload scene after saving it
					// NOTE(): Shouldn't have to do this
					scene.Unload( ); 
				} 

				if ( ImGui::MenuItem( "Material##Options", NULL ) )
				{
					// Construct material asset and save it
					AssetManager* am = EngineSubsystem( AssetManager );

					// Don't name it for now
					AssetHandle< Material > mat = am->ConstructAsset< Material >( ); 
				}
			}
		}; 

		auto loadProjectMenuOption = [&]()
		{
			ImGui::MenuItem( "Load Project...##options", NULL, &mLoadProjectPopupDialogue );
		};
		ImGuiManager::RegisterMenuOption("File", loadProjectMenuOption);
 
		// Register menu options
		ImGuiManager::RegisterMenuOption( "Create", createViewOption );

		// Register docking layouts 
		ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Scene View", nullptr, ImGui::DockSlotType::Slot_Top, 1.0f ) );
	    ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Graphics", nullptr, ImGui::DockSlotType::Slot_Bottom, 0.2f));
		ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Camera", nullptr, ImGui::DockSlotType::Slot_Right, 0.2f ) );
		ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Load Resource", "Graphics", ImGui::DockSlotType::Slot_Tab, 0.15f ) );
		ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "World Outliner", "Camera", ImGui::DockSlotType::Slot_Top, 0.7f ) );
		ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Play Options", "Scene View", ImGui::DockSlotType::Slot_Top, 0.1f ) );
		ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Create Project", "Scene View", ImGui::DockSlotType::Slot_Bottom, 0.1f ) );
		ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Inspector View", "World Outliner", ImGui::DockSlotType::Slot_Bottom, 0.5f ) );
		ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Scene Selection View", "Create Project", ImGui::DockSlotType::Slot_Tab, 0.5f ) );

		return Enjon::Result::SUCCESS;
	}

	//=================================================================================================

	void EditorApp::SetEditorSceneView( EditorSceneView* view )
	{
		mEditorSceneView = view;
	}

	//=================================================================================================

	void EditorApp::CheckForPopups( )
	{
		if ( mLoadProjectPopupDialogue )
		{
			LoadProjectView( );
		}
	}

	Enjon::Result EditorApp::Update( f32 dt )
	{ 
		// Update transform widget
		mTransformWidget.Update( );

		// Simulate game tick scenario if playing
		if ( mPlaying )
		{
			Application* app = mProject.GetApplication( );
			if ( app )
			{
				// Process application input
				app->ProcessInput( dt );

				// Update application ( ^ Could be called in the same tick )
				app->Update( dt );
			}
		} 

		return Enjon::Result::PROCESS_RUNNING;
	}

	enum class TransformMode
	{
		Translate,
		Scale
	};

	Enjon::Result EditorApp::ProcessInput( f32 dt )
	{
		static bool mInteractingWithTransformWidget = false;
		static TransformMode mMode = TransformMode::Translate;
		static Vec3 mIntersectionStartPosition;
		static Vec3 mRootStartPosition;
		static Vec3 mRootStartScale;
		static TransformWidgetRenderableType mType;
		static GraphicsSubsystem* mGfx = EngineSubsystem( GraphicsSubsystem );
		static Vec2 mMouseCoordsDelta = Vec2( 0.0f );
		Input* mInput = EngineSubsystem( Input );
		Camera* camera = mGfx->GetGraphicsSceneCamera( )->ConstCast< Enjon::Camera >( );
		Enjon::iVec2 viewPort = mGfx->GetViewport( ); 
		Enjon::Window* window = mGfx->GetWindow( )->ConstCast< Enjon::Window >( );

		// Can move camera if scene view has focus
		bool previousCamMove = mMoveCamera;
		mMoveCamera = mEditorWidgetManager.GetFocused( mEditorSceneView );
		if ( mMoveCamera && ( mMoveCamera != previousCamMove ) )
		{ 
			Vec2 mc = mInput->GetMouseCoords( );
			Vec2 center = mEditorSceneView->GetCenterOfViewport( );
			mMouseCoordsDelta = Vec2( (f32)(viewPort.x) / 2.0f - mc.x, (f32)(viewPort.y) / 2.0f - mc.y );
		}

		if ( !mPlaying )
		{
			if ( !mMoveCamera )
			{
				if ( mInput->IsKeyPressed( KeyCode::Delete ) )
				{
					if ( mSelectedEntity )
					{
						mSelectedEntity.Get( )->Destroy( );
						DeselectEntity( );
					}
				}

				if ( mSelectedEntity )
				{
					if ( mInput->IsKeyPressed( KeyCode::W ) )
					{
						mTransformWidget.SetTransformationMode( TransformationMode::Translation );
					}
					if ( mInput->IsKeyPressed( KeyCode::E ) )
					{
						mTransformWidget.SetTransformationMode( TransformationMode::Rotation );
					}
					if ( mInput->IsKeyPressed( KeyCode::R ) )
					{
						mTransformWidget.SetTransformationMode( TransformationMode::Scale );
					} 
					if ( mInput->IsKeyPressed( KeyCode::L ) )
					{
						switch ( mTransformWidget.GetTransformSpace( ) )
						{
							case TransformSpace::Local:
							{
								mTransformWidget.SetTransformSpace( TransformSpace::World ); 
							} break;
							case TransformSpace::World:
							{
								mTransformWidget.SetTransformSpace( TransformSpace::Local ); 
							} break;
						}
					}

					// Move entity to editor camera position and set rotation
					if ( mInput->IsKeyDown( KeyCode::LeftShift ) && mInput->IsKeyDown( KeyCode::LeftCtrl ) )
					{
						if ( mInput->IsKeyPressed( KeyCode::F ) )
						{
							mSelectedEntity.Get( )->SetLocalPosition( mEditorCamera.GetPosition( ) );
							mSelectedEntity.Get( )->SetLocalRotation( mEditorCamera.GetRotation( ).NegativeAngleAxis().Normalize() ); 

							//Vec3 eulerAngles = mEditorCamera.GetRotation( ).NegativeAngleAxis().EulerAngles( );
							//Quaternion newRot = Quaternion::FromEulerAngles( Vec3( eulerAngles.x, eulerAngles.y, -eulerAngles.z ) );
							//mSelectedEntity.Get()->SetLocalRotation( newRot.Normalize() );
						}
					}

					// Copy entity
					if ( mInput->IsKeyDown( KeyCode::LeftCtrl ) && mInput->IsKeyPressed( KeyCode::D ) )
					{
						EntityManager* em = EngineSubsystem( EntityManager );
						EntityHandle newEnt = em->CopyEntity( mSelectedEntity );
						if ( newEnt )
						{
							SelectEntity( newEnt );
						}
					}
				}

				if ( mInput->IsKeyDown( KeyCode::LeftMouseButton ) )
				{
					if ( mTransformWidget.IsInteractingWithWidget( ) )
					{
						mTransformWidget.InteractWithWidget( );
						Vec3 delta = mTransformWidget.GetDelta( );

						TransformationMode transformationMode = mTransformWidget.GetTransformationMode( );

						switch ( transformationMode )
						{
							case TransformationMode::Translation:
							{
								switch ( mTransformWidget.GetInteractedWidgetType( ) )
								{
									case TransformWidgetRenderableType::TranslationXYAxes:
									case TransformWidgetRenderableType::TranslationXZAxes:
									case TransformWidgetRenderableType::TranslationYZAxes:
									case TransformWidgetRenderableType::TranslationRoot:
									case TransformWidgetRenderableType::TranslationUpAxis:
									case TransformWidgetRenderableType::TranslationForwardAxis:
									case TransformWidgetRenderableType::TranslationRightAxis:
									{
										Entity* ent = mSelectedEntity.Get( );
										if ( ent )
										{
											Vec3 lp = ent->GetLocalPosition( ) + delta;
											ent->SetLocalPosition( lp );
										}
									} break;
								}

							} break;

							case TransformationMode::Scale:
							{
								switch ( mTransformWidget.GetInteractedWidgetType( ) )
								{
									case TransformWidgetRenderableType::ScaleYZAxes:
									case TransformWidgetRenderableType::ScaleXZAxes:
									case TransformWidgetRenderableType::ScaleXYAxes:
									case TransformWidgetRenderableType::ScaleForwardAxis:
									case TransformWidgetRenderableType::ScaleUpAxis:
									case TransformWidgetRenderableType::ScaleRightAxis:
									{
										Entity* ent = mSelectedEntity.Get( );
										if ( ent )
										{
											Vec3 ls = ent->GetLocalScale( ) + delta;
											ent->SetLocalScale( ls );
										}
									} break;
									case TransformWidgetRenderableType::ScaleRoot:
									{
										Entity* ent = mSelectedEntity.Get( );
										if ( ent )
										{
											Vec3 ls = ent->GetLocalScale( ) + delta * ent->GetLocalScale( );
											ent->SetLocalScale( ls );
										}

									} break;
								}

							} break;

							case TransformationMode::Rotation:
							{ 
								Entity* ent = mSelectedEntity.Get( );
								if ( ent )
								{
									ent->SetLocalRotation( ent->GetLocalRotation() * mTransformWidget.GetDeltaRotation() );
								}
							} break;
						}

						if ( mSelectedEntity.Get( ) )
						{
							Entity* ent = mSelectedEntity.Get( );
							// Set position and rotation to that of entity
							mTransformWidget.SetPosition( ent->GetWorldPosition( ) ); 
							mTransformWidget.SetRotation( -ent->GetWorldRotation( ) ); 
						}
					} 
				}
				else
				{
					mTransformWidget.EndInteraction( );
				}

			}

			if ( mMoveCamera )
			{ 
				Enjon::Vec3 velDir( 0, 0, 0 ); 

				// Set camera speed 
				Vec2 mw = mInput->GetMouseWheel( ).y;
				f32 mult = mw.y == 1.0f ? 1.5f : mw.y == -1.0f ? 0.75f : 1.0f;
				mCameraSpeed = Clamp(mCameraSpeed * mult, 0.25f, 128.0f);

				if ( mInput->IsKeyDown( Enjon::KeyCode::W ) )
				{
					Enjon::Vec3 F = camera->Forward( );
					velDir += F;
				}
				if ( mInput->IsKeyDown( Enjon::KeyCode::S ) )
				{
					Enjon::Vec3 B = camera->Backward( );
					velDir += B;
				}
				if ( mInput->IsKeyDown( Enjon::KeyCode::A ) )
				{
					velDir += camera->Left( );
				}
				if ( mInput->IsKeyDown( Enjon::KeyCode::D ) )
				{
					velDir += camera->Right( );
				}

				// Normalize velocity
				velDir = Enjon::Vec3::Normalize( velDir );

				f32 avgDT = Engine::GetInstance( )->GetWorldTime( ).GetDeltaTime( );

				// Set camera position
				camera->SetPosition( camera->GetPosition() + ( mCameraSpeed * avgDT * velDir ) );

				// Set camera rotation
				// Get mouse input and change orientation of camera
				Enjon::Vec2 mouseCoords = mInput->GetMouseCoords( );

				//Vec2 mouseCoords = mEditorSceneView->GetSceneViewProjectedCursorPosition( );

				// Set cursor to not visible
				window->ShowMouseCursor( false );

				// Reset the mouse coords after having gotten the mouse coordinates
				Vec2 center = mEditorSceneView->GetCenterOfViewport( );
				//SDL_WarpMouseInWindow( window->GetWindowContext( ), (s32)center.x, (s32)center.y );
				SDL_WarpMouseInWindow( window->GetWindowContext( ), ( f32 )viewPort.x / 2.0f - mMouseCoordsDelta.x, ( f32 )viewPort.y / 2.0f - mMouseCoordsDelta.y );

				// Offset camera orientation
				f32 xOffset = Enjon::ToRadians( ( f32 )viewPort.x / 2.0f - mouseCoords.x - mMouseCoordsDelta.x ) * mMouseSensitivity / 100.0f;
				f32 yOffset = Enjon::ToRadians( ( f32 )viewPort.y / 2.0f - mouseCoords.y - mMouseCoordsDelta.y ) * mMouseSensitivity / 100.0f;
				camera->OffsetOrientation( xOffset, yOffset );

				// Adjust rotator
				mCameraRotator.x += xOffset;
				mCameraRotator.y += yOffset;
			}

			// Mouse cursor on, interact with world
			else
			{
				mGfx->GetWindow( )->ConstCast< Enjon::Window >( )->ShowMouseCursor( true );
 
				if ( mEditorWidgetManager.GetHovered( mEditorSceneView ) )
				{
					if ( mInput->IsKeyPressed( KeyCode::LeftMouseButton ) )
					{
						auto viewport = mGfx->GetViewport( );
						auto mp = GetSceneViewProjectedCursorPosition( );

						iVec2 dispSize = mGfx->GetViewport( );
						PickResult pr = mGfx->GetPickedObjectResult( GetSceneViewProjectedCursorPosition( ) );
						if ( pr.mEntity.Get( ) )
						{
							// Set selected entity
							SelectEntity( pr.mEntity );
						}
						// Translation widget interaction
						else if ( EditorTransformWidget::IsValidID( pr.mId ) )
						{
							// Begin widget interaction
							mTransformWidget.BeginWidgetInteraction( TransformWidgetRenderableType( pr.mId - MAX_ENTITIES ) );
						}
						else
						{
							// Deselect entity if click in scene view and not anything valid
							DeselectEntity( ); 
						}
					} 
				}
			} 


			if ( !mTransformWidget.IsInteractingWithWidget( ) && mSelectedEntity )
			{
				mTransformWidget.SetPosition( mSelectedEntity.Get( )->GetWorldPosition( ) ); 
				mTransformWidget.SetRotation( mSelectedEntity.Get( )->GetWorldRotation( ) );
			}
		}

		// Starting /Stopping game instance
		if ( mInput->IsKeyPressed( Enjon::KeyCode::Escape ) )
		{
			if ( mPlaying )
			{
				mPlaying = false; 
				mMoveCamera = false;

				// Call shut down function for game
				Application* app = mProject.GetApplication( );
				if ( app )
				{
					// Shutown the application
					ShutdownProjectApp( nullptr );
				}

				camera->SetPosition( mPreviousCameraTransform.GetPosition() );
				camera->SetRotation( mPreviousCameraTransform.GetRotation() );
			}
		} 

		return Enjon::Result::PROCESS_RUNNING;
	}

	Enjon::Result EditorApp::Shutdown( )
	{ 
		return Enjon::Result::SUCCESS;
	} 

	void EditorApp::LoadProjectResources( )
	{
		Enjon::String paintPeelingAlbedoPath = Enjon::String( "Materials/PaintPeeling/Albedo.png" );
		Enjon::String paintPeelingNormalPath = Enjon::String( "Materials/PaintPeeling/Normal.png" );
		Enjon::String paintPeelingRoughnessPath = Enjon::String( "Materials/PaintPeeling/Roughness.png" );
		Enjon::String paintPeelingMetallicPath = Enjon::String( "Materials/PaintPeeling/Metallic.png" );
		Enjon::String paintPeelingAOPath = Enjon::String( "Materials/PaintPeeling/ao.png" );

		AssetManager* am = EngineSubsystem( AssetManager );

		am->AddToDatabase( paintPeelingAlbedoPath );
		am->AddToDatabase( paintPeelingNormalPath );
		am->AddToDatabase( paintPeelingMetallicPath );
		am->AddToDatabase( paintPeelingRoughnessPath );
		am->AddToDatabase( paintPeelingAOPath );

		AssetHandle< Material > pp = am->ConstructAsset< Material >( );
		AssetHandle< ShaderGraph > sg = am->GetAsset< ShaderGraph >( "shaders.shadergraphs.defaultstaticgeom" ); 
		pp->SetShaderGraph( sg );
		pp.Get()->ConstCast< Material >()->SetUniform( "albedoMap", am->GetAsset< Texture >( "materials.paintpeeling.albedo" ) );
		pp.Get()->ConstCast< Material >()->SetUniform( "normalMap", am->GetAsset< Texture >( "materials.paintpeeling.normal" ) );
		pp.Get()->ConstCast< Material >()->SetUniform( "metallicMap", am->GetAsset< Texture >( "materials.paintpeeling.metallic" ) );
		pp.Get()->ConstCast< Material >()->SetUniform( "roughMap", am->GetAsset< Texture >( "materials.paintpeeling.roughness" ) );
		pp.Get()->ConstCast< Material >()->SetUniform( "aoMap", am->GetAsset< Texture >( "materials.paintpeeling.ao" ) );
		pp.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", am->GetAsset< Texture >( "textures.black" ) );
		pp->Save( );
	}

	void EditorApp::LoadResources( )
	{
		// Paths to resources
		Enjon::String greenPath				= Enjon::String("Textures/green.png"); 
		Enjon::String redPath				= Enjon::String("Textures/red.png"); 
		Enjon::String bluePath				= Enjon::String("Textures/blue.png"); 
		Enjon::String blackPath				= Enjon::String("Textures/black.png"); 
		Enjon::String midGreyPath			= Enjon::String("Textures/grey.png"); 
		Enjon::String lightGreyPath			= Enjon::String("Textures/light_grey.png"); 
		Enjon::String whitePath				= Enjon::String("Textures/white.png"); 
		Enjon::String yellowPath			= Enjon::String("Textures/yellow.png"); 
		Enjon::String axisBoxDiffusePath	= Enjon::String("Textures/axisBoxDiffuse.png"); 
		Enjon::String hdrPath				= Enjon::String("Textures/HDR/03-ueno-shrine_3k.hdr"); 
		Enjon::String cubePath				= Enjon::String("Models/unit_cube.obj"); 
		Enjon::String spherePath			= Enjon::String("Models/unit_sphere.obj"); 
		Enjon::String conePath				= Enjon::String("Models/unit_cone.obj"); 
		Enjon::String cylinderPath			= Enjon::String("Models/unit_cylinder.obj"); 
		Enjon::String ringPath				= Enjon::String("Models/unit_ring.obj"); 
		Enjon::String axisBoxPath			= Enjon::String("Models/axisBox.obj"); 
		Enjon::String shaderGraphPath		= Enjon::String("Shaders/ShaderGraphs/DefaultStaticGeom.sg"); 

		AssetManager* mAssetManager = EngineSubsystem( AssetManager );
		
		// Add to asset database( will serialize the asset if not loaded from disk, otherwise will load the asset )
		mAssetManager->AddToDatabase( axisBoxDiffusePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( greenPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( redPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( midGreyPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( lightGreyPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( bluePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( blackPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( whitePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( yellowPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( hdrPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( cubePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( spherePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( conePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( cylinderPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( ringPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( axisBoxPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( shaderGraphPath, true, true, AssetLocationType::EngineAsset );

		// Create materials
		AssetHandle< Material > redMat = mAssetManager->ConstructAsset< Material >( "RedMaterial" );
		AssetHandle< Material > greenMat = mAssetManager->ConstructAsset< Material >( "GreenMaterial" );
		AssetHandle< Material > blueMat = mAssetManager->ConstructAsset< Material >( "BlueMaterial" );
		AssetHandle< Material > yellowMat = mAssetManager->ConstructAsset< Material >( "YellowMaterial" );
		AssetHandle< Material > axisBoxMat = mAssetManager->ConstructAsset< Material >( "AxisBoxMat" );
		AssetHandle< ShaderGraph > sg = mAssetManager->GetAsset< ShaderGraph >( "shaders.shadergraphs.defaultstaticgeom" );

		axisBoxMat->SetShaderGraph( sg );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "albedoMap", mAssetManager->GetAsset< Texture >( "textures.axisboxdiffuse" ) );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "normalMap", mAssetManager->GetAsset< Texture >( "textures.front_normal" ) );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "metallicMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "roughMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "aoMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );

		redMat->SetShaderGraph( sg );
		redMat.Get()->ConstCast< Material >()->SetUniform( "albedoMap", mAssetManager->GetAsset< Texture >( "textures.red" ) );
		redMat.Get()->ConstCast< Material >()->SetUniform( "normalMap", mAssetManager->GetAsset< Texture >( "textures.front_normal" ) );
		redMat.Get()->ConstCast< Material >()->SetUniform( "metallicMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );
		redMat.Get()->ConstCast< Material >()->SetUniform( "roughMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		redMat.Get()->ConstCast< Material >()->SetUniform( "aoMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		redMat.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );

		greenMat->SetShaderGraph( sg );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "albedoMap", mAssetManager->GetAsset< Texture >( "textures.green" ) );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "normalMap", mAssetManager->GetAsset< Texture >( "textures.front_normal" ) );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "metallicMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "roughMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "aoMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );

		blueMat->SetShaderGraph( sg );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "albedoMap", mAssetManager->GetAsset< Texture >( "textures.blue" ) );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "normalMap", mAssetManager->GetAsset< Texture >( "textures.front_normal" ) );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "metallicMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "roughMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "aoMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );

		yellowMat->SetShaderGraph( sg );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "albedoMap", mAssetManager->GetAsset< Texture >( "textures.yellow" ) );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "normalMap", mAssetManager->GetAsset< Texture >( "textures.front_normal" ) );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "metallicMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "roughMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "aoMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );

		axisBoxMat->Save( );
		redMat->Save( );
		greenMat->Save( );
		blueMat->Save( );
		yellowMat->Save( ); 
	}
}


