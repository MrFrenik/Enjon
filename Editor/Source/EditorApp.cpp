#include "EditorApp.h"

#include <Engine.h>
#include <Asset/AssetManager.h>
#include <SubsystemCatalog.h>
#include <IO/InputManager.h>
#include <ImGui/ImGuiManager.h>
#include <Graphics/GraphicsSubsystem.h> 

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
	const Enjon::Input* input = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::Input >( );

	if ( input->IsKeyPressed( Enjon::KeyCode::Escape ) )
	{
		return Enjon::Result::SUCCESS;
	}

	return Enjon::Result::PROCESS_RUNNING;
}

Enjon::Result EnjonEditor::Shutdown( )
{ 
	return Enjon::Result::SUCCESS;
}

