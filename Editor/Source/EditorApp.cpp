#include "EditorApp.h"

#include <Engine.h>
#include <Asset/AssetManager.h>
#include <SubsystemCatalog.h>
#include <IO/InputManager.h>
#include <ImGui/ImGuiManager.h>
#include <Graphics/GraphicsSubsystem.h> 
#include <Graphics/Window.h>
#include <Entity/EntityManager.h>
#include <Entity/Components/GraphicsComponent.h>

#include <windows.h>
#include <fmt/format.h>

typedef int( *funcAdd )( const int&, const int& );
typedef int( *funcSubtract )( const int&, const int& );
typedef void( *funcEntityRotate )( const Enjon::EntityHandle&, const Enjon::f32&, bool );
typedef void( *funcSetEngineInstance )( Enjon::Engine* instance );

// TODO(): Make sure to abstract this for platform independence
HINSTANCE dllHandle = nullptr;
funcSubtract subFunc = nullptr;
funcAdd addFunc = nullptr; 
funcEntityRotate entityRotateFunc = nullptr;
funcSetEngineInstance setEngineFunc = nullptr;

namespace fs = std::experimental::filesystem; 

Enjon::String copyDir = "E:/Development/C++DLLTest/Build/Debug/TestDLLIntermediate/";

void CopyLibraryContents( )
{
	Enjon::String rootDir = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( );

	fs::path dllPath = rootDir + "Build/Debug/TestDLL.dll";
	if ( fs::exists( dllPath ) )
	{
		fs::remove( dllPath );
	}

	fs::path pdbPath = rootDir + "Build/Debug/TestDLL.pdb";
	if ( fs::exists( pdbPath ) )
	{
		fs::remove( pdbPath );
	}

	// Now copy over contents from intermediate build to executable dir
	dllPath = copyDir;
	if ( fs::exists( dllPath ) )
	{
		//fs::copy( fs::path( dllPath.string( ) + "TestDLL.dll" ), rootDir + "Build/Debug/TestDLL.dll" );
		fs::copy( dllPath, rootDir + "Build/Debug/", fs::copy_options::recursive );
	}

	fs::path expPath = rootDir + "Build/Debug/TestDLL.exp";
	if ( fs::exists( expPath ) )
	{
		fs::remove( expPath );
	}

	fs::path libPath = rootDir + "Build/Debug/TestDLL.lib";
	if ( fs::exists( libPath ) )
	{
		fs::remove( libPath );
	}

	fs::path ilkPath = rootDir + "Build/Debug/TestDLL.ilk";
	if ( fs::exists( ilkPath ) )
	{
		fs::remove( ilkPath );
	}
}


void SceneView( bool* viewBool )
{
	const Enjon::GraphicsSubsystem* gfx = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( );
	u32 currentTextureId = gfx->GetCurrentRenderTextureId( ); 

	// Render game in window
	ImVec2 cursorPos = ImGui::GetCursorScreenPos( );

	ImTextureID img = ( ImTextureID )currentTextureId;
	ImGui::Image( img, ImVec2( ImGui::GetWindowWidth( ), ImGui::GetWindowHeight( ) ),
		ImVec2( 0, 1 ), ImVec2( 1, 0 ), ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 0 ) );

	ImVec2 min = ImVec2( cursorPos.x + ImGui::GetContentRegionAvailWidth( ) - 100.0f, cursorPos.y + 10.0f );
	ImVec2 max = ImVec2( min.x + 50.0f, min.y + 10.0f );

	ImGui::SetCursorScreenPos( min );
	auto drawlist = ImGui::GetWindowDrawList( );
	f32 fps = ImGui::GetIO( ).Framerate;

	// Update camera aspect ratio
	gfx->GetSceneCamera( )->ConstCast< Enjon::Camera >( )->SetAspectRatio( ImGui::GetWindowWidth( ) / ImGui::GetWindowHeight( ) );
}

void CameraOptions( bool* enable )
{
	const Enjon::GraphicsSubsystem* gfx = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( ); 
	const Enjon::Camera* cam = gfx->GetSceneCamera( );

	if ( ImGui::TreeNode( "Camera" ) )
	{
		Enjon::ImGuiManager::DebugDumpObject( cam ); 
		ImGui::TreePop( );
	}
}

void EnjonEditor::WorldOutlinerView( )
{
	const Enjon::EntityManager* entities = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::EntityManager >( );
	ImGui::Text( fmt::format( "Entities: {}", entities->GetActiveEntities().size() ).c_str() ); 

	for ( auto& e : entities->GetActiveEntities( ) )
	{
		if ( ImGui::TreeNode( fmt::format( "{}", e->GetID( ) ).c_str( ) ) )
		{
			Enjon::ImGuiManager::DebugDumpObject( e );
			ImGui::TreePop( );
		}
	}
}

void EnjonEditor::LoadResourceFromFile( )
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
			Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( )->ConstCast< Enjon::AssetManager >( );
			am->AddToDatabase( mResourceFilePathToLoad );
		}
	}
}
 
Enjon::Result EnjonEditor::Initialize( )
{ 
	mApplicationName = "EnjonEditor";

	Enjon::String mAssetsDirectoryPath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + "Editor/Assets/";
	Enjon::String cacheDirectoryPath = mAssetsDirectoryPath + "/Cache/";

	// Get asset manager and set its properties ( I don't like this )
	Enjon::AssetManager* mAssetManager = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get<Enjon::AssetManager>( )->ConstCast< Enjon::AssetManager >( );
	Enjon::GraphicsSubsystem* mGfx = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( );

	// This also needs to be done through a config file or cmake
	mAssetManager->SetAssetsDirectoryPath( mAssetsDirectoryPath );
	mAssetManager->SetCachedAssetsDirectoryPath( cacheDirectoryPath );
	mAssetManager->SetDatabaseName( GetApplicationName( ) );
	mAssetManager->Initialize( ); 

	// Register individual windows
	Enjon::ImGuiManager::RegisterWindow( [ & ] ( )
	{
		// Docking windows
		if ( ImGui::BeginDock( "Scene", &mViewBool, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
		{
			// Print docking information
			SceneView( &mViewBool );
		}
		ImGui::EndDock( ); 
	}); 

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

	// Register docking layouts
	Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Scene", nullptr, ImGui::DockSlotType::Slot_Top, 1.0f ) );
	Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Camera", "Scene", ImGui::DockSlotType::Slot_Right, 0.2f ) );
	Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Load Resource", "Camera", ImGui::DockSlotType::Slot_Bottom, 0.3f ) );
	Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "World Outliner", "Camera", ImGui::DockSlotType::Slot_Top, 0.7f ) );

	// Create an entity to manipulate 
	Enjon::EntityManager* entities = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::EntityManager >( )->ConstCast< Enjon::EntityManager >( );
	mEntity = entities->Allocate( );

	Enjon::GraphicsComponent* gfxCmp = mEntity.Get( )->AddComponent< Enjon::GraphicsComponent >( );
	gfxCmp->SetMesh( mAssetManager->GetAsset< Enjon::Mesh >( "models.unit_cube" ) );
	gfxCmp->SetMaterial( mAssetManager->GetAsset< Enjon::Material >( "NewMaterial" ).Get( ) );

	mGfx->GetScene( )->AddRenderable( gfxCmp->GetRenderable( ) );

	return Enjon::Result::SUCCESS;
}

Enjon::Result EnjonEditor::Update( f32 dt )
{ 
	static float t = 0.0f;
	t += dt;

	// Try to rotate entity if dll is loaded
	if ( entityRotateFunc )
	{ 
		entityRotateFunc( mEntity, t, mEntitySwitch );
	}

	return Enjon::Result::PROCESS_RUNNING;
}

Enjon::Result EnjonEditor::ProcessInput( f32 dt )
{ 
	const Enjon::GraphicsSubsystem* mGfx = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem>( );
	const Enjon::Input* mInput = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::Input >( );
	Enjon::Camera* camera = mGfx->GetSceneCamera( )->ConstCast< Enjon::Camera >(); 

	if ( mInput->IsKeyPressed( Enjon::KeyCode::T ) )
	{
		mMoveCamera ^= 1;
	}

	if ( mMoveCamera )
	{
		Enjon::Vec3 velDir( 0, 0, 0 );

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

		// Set camera position
		camera->Transform.Position += dt * 10.0f * velDir;

		// Set camera rotation
		// Get mouse input and change orientation of camera
		Enjon::Vec2 mouseCoords = mInput->GetMouseCoords( );

		Enjon::iVec2 viewPort = mGfx->GetViewport( );

		f32 mouseSensitivity = 10.0f;

		// Grab window from graphics subsystem
		Enjon::Window* window = mGfx->GetWindow( )->ConstCast< Enjon::Window >( );

		// Set cursor to not visible
		window->ShowMouseCursor( false );

		// Reset the mouse coords after having gotten the mouse coordinates
		SDL_WarpMouseInWindow( window->GetWindowContext( ), ( f32 )viewPort.x / 2.0f, ( f32 )viewPort.y / 2.0f );

		// Offset camera orientation
		f32 xOffset = Enjon::ToRadians( ( f32 )viewPort.x / 2.0f - mouseCoords.x ) * dt * mouseSensitivity;
		f32 yOffset = Enjon::ToRadians( ( f32 )viewPort.y / 2.0f - mouseCoords.y ) * dt * mouseSensitivity;
		camera->OffsetOrientation( xOffset, yOffset );
	}
	else
	{
		mGfx->GetWindow( )->ConstCast< Enjon::Window >( )->ShowMouseCursor( true );
	}

	// DLL Loading/Unloading
	{
		if ( mInput->IsKeyPressed( Enjon::KeyCode::R ) )
		{
			if ( dllHandle )
			{
				// Free library if in use
				FreeLibrary( dllHandle );
				dllHandle = nullptr;
				addFunc = nullptr;
				subFunc = nullptr;
				entityRotateFunc = nullptr;
			}

			// Copy files to directory
			CopyLibraryContents( );

			// Try to load library
			dllHandle = LoadLibrary( "TestDLL.dll" );

			// If valid, then set address of procedures to be called
			if ( dllHandle )
			{
				addFunc = ( funcAdd )GetProcAddress( dllHandle, "Add" );
				subFunc = ( funcSubtract )GetProcAddress( dllHandle, "Subtract" );
				entityRotateFunc = ( funcEntityRotate )GetProcAddress( dllHandle, "RotateEntity" );

				// Try and set the engine instance
				setEngineFunc = ( funcSetEngineInstance )GetProcAddress( dllHandle, "SetEngineInstance" );
				if ( setEngineFunc )
				{
					setEngineFunc( Enjon::Engine::GetInstance( ) );
				}

			}
			else
			{
				std::cout << "Could not load library\n";
			}
		}

		if ( mInput->IsKeyPressed( Enjon::KeyCode::One ) )
		{
			if ( addFunc )
			{
				std::cout << addFunc( 10, 10 ) << "\n";
			}
		}

		if ( mInput->IsKeyPressed( Enjon::KeyCode::Two ) )
		{
			if ( subFunc )
			{
				std::cout << subFunc( 20, 5 ) << "\n";
			}
		}

		if ( mInput->IsKeyPressed( Enjon::KeyCode::E ) )
		{
			mEntitySwitch ^= 1;
		}

		if ( mInput->IsKeyPressed( Enjon::KeyCode::O ) )
		{
			if ( !mEntity.Get( )->HasChildren( ) )
			{ 
				Enjon::EntityManager* entities = Enjon::Engine::GetInstance()->GetSubsystemCatalog( )->Get< Enjon::EntityManager >( )->ConstCast< Enjon::EntityManager >( );
				const Enjon::AssetManager* assetManager = Enjon::Engine::GetInstance()->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );

				for ( u32 i = 0; i < 5; ++i )
				{
					// Add entity as child, for shiggles
					Enjon::EntityHandle child = entities->Allocate( );
					mEntity.Get( )->AddChild( child );

					Enjon::GraphicsComponent* childGfx = child.Get( )->AddComponent< Enjon::GraphicsComponent >( );
					childGfx->SetMesh( assetManager->GetAsset< Enjon::Mesh >( "models.unit_sphere" ) );
					childGfx->SetMaterial( assetManager->GetAsset< Enjon::Material >( "NewMaterial" ).Get( ) );

					// Set local transform of child
					child.Get( )->SetLocalTransform( Enjon::Transform( Enjon::Vec3( std::cos( ( f32 )i ), 2.0f, std::sin( ( f32 )i ) ), Enjon::Quaternion( ), Enjon::Vec3( 0.5f ) ) );

					// Add renderable to scene
					Enjon::Engine::GetInstance()->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( )->GetScene( )->AddRenderable( childGfx->GetRenderable( ) );
				}
			}
		}

		if ( mInput->IsKeyPressed( Enjon::KeyCode::P ) )
		{
			if ( mEntity.Get( )->HasChildren( ) )
			{
				for ( auto& child : mEntity.Get( )->GetChildren( ) )
				{
					child.Get( )->Destroy( );
				}
			}
		}
	}

	return Enjon::Result::PROCESS_RUNNING;
}

Enjon::Result EnjonEditor::Shutdown( )
{ 
	return Enjon::Result::SUCCESS;
}

