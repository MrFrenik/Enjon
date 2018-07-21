
#include "EditorMaterialEditWindow.h"
#include "EditorApp.h"

#include <Graphics/GraphicsSubsystem.h>
#include <Scene/SceneManager.h>
#include <Asset/AssetManager.h>
#include <Graphics/StaticMeshRenderable.h>
#include <SubsystemCatalog.h>
#include <Engine.h>
#include <fmt/format.h>

namespace Enjon
{
	/**
	* @brief Must be overriden
	*/
	void EditorViewport::UpdateView( )
	{ 
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem ); 

		// Grab graphics context from world and then get framebuffer rendertarget texture
		World* world = mWindow->GetWorld( );
		GraphicsSubsystemContext* gfxCtx = world->GetContext< GraphicsSubsystemContext >( );
		u32 currentTextureId = gfxCtx->GetFrameBuffer( )->GetTexture( ); 
		//u32 currentTextureId = gfx->GetGBufferTexture( GBufferTextureType::OBJECT_ID );

		// Rotate camera over time
		Camera* cam = gfxCtx->GetGraphicsScene( )->GetActiveCamera( );
		//cam->SetRotation( Quaternion::AngleAxis( Engine::GetInstance( )->GetWorldTime( ).GetTotalTimeElapsed( ), Vec3::YAxis( ) ) );

		// Render game in window
		ImVec2 cursorPos = ImGui::GetCursorScreenPos( );

		// Cache off cursor position for scene view
		Vec2 padding( 20.0f, 8.0f );
		//Vec2 padding( -20.0f, -40.0f );
		f32 width = ImGui::GetWindowWidth( ) - padding.x;
		f32 height = ImGui::GetWindowSize( ).y - ImGui::GetCursorPosY( ) - padding.y;
		mSceneViewWindowPosition = Vec2( cursorPos.x, cursorPos.y );
		mSceneViewWindowSize = Vec2( width, height );

		ImTextureID img = ( ImTextureID )currentTextureId;
		ImGui::Image( img, ImVec2( width, height ),
			ImVec2( 0, 1 ), ImVec2( 1, 0 ), ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 0 ) );

		// Update camera aspect ratio
		gfxCtx->GetGraphicsScene( )->GetActiveCamera( )->SetAspectRatio( ImGui::GetWindowWidth( ) / ImGui::GetWindowHeight( ) );

		// Draw border around image
		ImDrawList* dl = ImGui::GetWindowDrawList( );
		ImVec2 a( mSceneViewWindowPosition.x, mSceneViewWindowPosition.y );
		ImVec2 b( mSceneViewWindowPosition.x + mSceneViewWindowSize.x, mSceneViewWindowPosition.y + mSceneViewWindowSize.y ); 
		dl->AddRect( a, b, ImColor( 0.0f, 0.64f, 1.0f, 0.48f ), 1.0f, 15, 1.5f ); 
	} 

	EditorMaterialEditWindow::EditorMaterialEditWindow( const AssetHandle< Material >& mat )
		: mMaterial( mat ), mInitialized( false )
	{ 
	}

	int EditorMaterialEditWindow::Init( std::string windowName, int screenWidth, int screenHeight, WindowFlagsMask currentFlags )
	{ 
		// Construct scene in world
		if ( !mInitialized )
		{ 
			// Init window base
			Window::Init( windowName, screenWidth, screenHeight, currentFlags );

			// Initialize new world 
			mWorld = new World( ); 
			// Register contexts with world
			mWorld->RegisterContext< GraphicsSubsystemContext >( ); 

			ConstructScene( );

			mInitialized = true;
		} 

		return 1; 
	} 
 
	void EditorMaterialEditWindow::ConstructScene( )
	{ 
		GUIContext* guiContext = GetGUIContext( );

		// Create viewport
		mViewport = new EditorViewport( Engine::GetInstance( )->GetApplication( )->ConstCast< EditorApp >( ), this );

		guiContext->RegisterDockingLayout( GUIDockingLayout( "Viewport", nullptr, GUIDockSlotType::Slot_Tab, 1.0f ) ); 

		// NOTE(): This should be done automatically for the user in the backend
		// Add window to graphics subsystem ( totally stupid way to do this )
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		gfx->AddWindow( this );

		World* world = GetWorld( );
		GraphicsScene* scene = world->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( );

		// Need to create an external scene camera held in the viewport that can manipulate the scene view
		Camera* cam = scene->GetActiveCamera( );
		cam->SetNearFar( 0.1f, 1000.0f );
		cam->SetProjection( ProjectionType::Perspective );
		cam->SetPosition( Vec3( 0.0f, 0.0f, -3.0f ) );

		mRenderable.SetMesh( EngineSubsystem( AssetManager )->GetAsset< Mesh >( "models.unit_sphere" ) );
		mRenderable.SetPosition( cam->GetPosition() + cam->Forward() * 5.0f );
		mRenderable.SetScale( 2.0f );
		mRenderable.SetMaterial( mMaterial );
		scene->AddStaticMeshRenderable( &mRenderable ); 

		guiContext->RegisterWindow( "Properties", [ & ]
		{
			if ( ImGui::BeginDock( "Properties", &mViewportOpen ) )
			{
				if ( mMaterial )
				{
					ImGui::Text( fmt::format( "Material: {}", mMaterial.Get()->GetName( ) ).c_str( ) ); 
					ImGuiManager* igm = EngineSubsystem( ImGuiManager );
					igm->InspectObject( mMaterial.Get() );
				}

				ImGui::EndDock( );
			}
		} );

		auto saveMaterialOption = [ & ] ( )
		{ 
			if ( ImGui::MenuItem( "Save##save_mat_options", NULL ) )
			{
				if ( mMaterial )
				{
					mMaterial->Save( );
				} 
			}
		};

		// Register menu options
		guiContext->RegisterMenuOption("File", "Save##save_material_options", saveMaterialOption); 

		guiContext->RegisterDockingLayout( GUIDockingLayout( "Viewport", nullptr, GUIDockSlotType::Slot_Tab, 1.0f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Properties", "Viewport", GUIDockSlotType::Slot_Left, 0.45f ) );
	} 
}
