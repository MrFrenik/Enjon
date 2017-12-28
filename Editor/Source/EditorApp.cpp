#include "EditorApp.h"

#include <Engine.h>
#include <Asset/AssetManager.h>
#include <SubsystemCatalog.h>
#include <IO/InputManager.h>
#include <ImGui/ImGuiManager.h>
#include <Graphics/GraphicsSubsystem.h> 
#include <Graphics/Window.h>

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

	// Register docking layouts
	Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Scene", nullptr, ImGui::DockSlotType::Slot_Top, 1.0f ) );
	Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Camera", "Scene", ImGui::DockSlotType::Slot_Right, 0.2f ) );
	Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Load Resource", "Camera", ImGui::DockSlotType::Slot_Bottom, 0.3f ) );

	return Enjon::Result::SUCCESS;
}

Enjon::Result EnjonEditor::Update( f32 dt )
{ 
	return Enjon::Result::PROCESS_RUNNING;
}

Enjon::Result EnjonEditor::ProcessInput( f32 dt )
{ 
	const Enjon::GraphicsSubsystem* mGfx = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem>( );
	const Enjon::Input* mInput = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::Input >( );
	Enjon::Camera* camera = mGfx->GetSceneCamera( )->ConstCast< Enjon::Camera >();

 
	if ( mInput->IsKeyPressed( Enjon::KeyCode::Escape ) )
	{
		return Enjon::Result::SUCCESS;
	} 

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

		const Enjon::f32 mouseSensitivity = 10.0f;

		// Grab window from graphics subsystem
		Enjon::Window* window = const_cast< Enjon::Window* >( mGfx->GetWindow( ) );

		// Set cursor to not visible
		window->ShowMouseCursor( false );

		// Reset the mouse coords after having gotten the mouse coordinates
		SDL_WarpMouseInWindow( window->GetWindowContext( ), ( float )viewPort.x / 2.0f, ( float )viewPort.y / 2.0f );

		// Offset camera orientation
		f32 xOffset = Enjon::ToRadians( ( f32 )viewPort.x / 2.0f - mouseCoords.x ) * dt * mouseSensitivity;
		f32 yOffset = Enjon::ToRadians( ( f32 )viewPort.y / 2.0f - mouseCoords.y ) * dt * mouseSensitivity;
		camera->OffsetOrientation( xOffset, yOffset ); 
	}

	return Enjon::Result::PROCESS_RUNNING;
}

Enjon::Result EnjonEditor::Shutdown( )
{ 
	return Enjon::Result::SUCCESS;
}

