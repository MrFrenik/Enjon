
#include "EditorMaterialEditWindow.h"
#include "EditorWorldOutlinerView.h"
#include "EditorApp.h"

#include <Graphics/GraphicsSubsystem.h>
#include <Scene/SceneManager.h>
#include <Asset/AssetManager.h>
#include <Entity/Archetype.h>
#include <Graphics/StaticMeshRenderable.h>
#include <Entity/Components/StaticMeshComponent.h>
#include <IO/InputManager.h>
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

		// Rotate camera over time
		Camera* cam = gfxCtx->GetGraphicsScene( )->GetActiveCamera( );

		// Render game in window
		ImVec2 cursorPos = ImGui::GetCursorScreenPos( );

		// Cache off cursor position for scene view
		Vec2 padding( 20.0f, 8.0f );
		f32 width = ImGui::GetWindowWidth( ) - padding.x;
		f32 height = ImGui::GetWindowSize( ).y - ImGui::GetCursorPosY( ) - padding.y;
		mSceneViewWindowPosition = Vec2( cursorPos.x, cursorPos.y );
		mSceneViewWindowSize = Vec2( width, height );

		ImTextureID img = ( ImTextureID )currentTextureId;
		ImGui::Image( img, ImVec2( width, height ), ImVec2( 0, 1 ), ImVec2( 1, 0 ), ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 0 ) );

		// Update camera aspect ratio
		gfxCtx->GetGraphicsScene( )->GetActiveCamera( )->SetAspectRatio( ImGui::GetWindowWidth( ) / ImGui::GetWindowHeight( ) );

		// Draw border around image
		ImDrawList* dl = ImGui::GetWindowDrawList( );
		ImVec2 a( mSceneViewWindowPosition.x, mSceneViewWindowPosition.y );
		ImVec2 b( mSceneViewWindowPosition.x + mSceneViewWindowSize.x, mSceneViewWindowPosition.y + mSceneViewWindowSize.y ); 
		dl->AddRect( a, b, ImColor( 0.0f, 0.64f, 1.0f, 0.48f ), 1.0f, 15, 1.5f ); 

		if ( IsFocused( ) )
		{
			UpdateCamera( );
		}
		else
		{
			if ( mFocusSet )
			{
				// Reset to showing mouse cursor ( push and pop this, maybe? )
				mWindow->ShowMouseCursor( true ); 
			}

			mStartedFocusing = false;
			mFocusSet = false;
		}
	} 

	void EditorViewport::CaptureState( )
	{
		Input* input = EngineSubsystem( Input );

		// Capture hovered state
		mIsHovered = ImGui::IsWindowHovered( ); 

		// Capture focused state
		mIsFocused = ( mIsHovered && ( input->IsKeyDown( KeyCode::RightMouseButton ) ) );
	}

	void EditorViewport::UpdateCamera( )
	{
		if ( IsFocused( ) && !mFocusSet )
		{
			if ( !mStartedFocusing )
			{
				mStartedFocusing = true;
				mFocusSet = true;
			}
		}

		Input* input = EngineSubsystem( Input ); 
 
		Enjon::Vec3 velDir( 0, 0, 0 ); 

		Window* window = this->GetWindow( );

		// Can't operate without a window
		assert( window != nullptr );

		World* world = window->GetWorld( );

		// Can't operate without a world
		assert( world != nullptr );

		GraphicsSubsystemContext* gsc = world->GetContext< GraphicsSubsystemContext >( );

		// Can't operate without a graphics subsystem context
		assert( gsc != nullptr );

		// Get viewport of window
		iVec2 viewPort = window->GetViewport( );

		if ( mStartedFocusing )
		{
			Vec2 mc = input->GetMouseCoords( );
			Vec2 center = GetCenterOfViewport( );
			mMouseCoordsDelta = Vec2( (f32)(viewPort.x) / 2.0f - mc.x, (f32)(viewPort.y) / 2.0f - mc.y ); 
			mStartedFocusing = false;
		}

		Camera* camera = gsc->GetGraphicsScene( )->GetActiveCamera( );

		// Set camera speed 
		Vec2 mw = input->GetMouseWheel( ).y;
		f32 mult = mw.y == 1.0f ? 1.5f : mw.y == -1.0f ? 0.75f : 1.0f;
		mCameraSpeed = Math::Clamp(mCameraSpeed * mult, 0.25f, 128.0f);

		if ( input->IsKeyDown( Enjon::KeyCode::W ) )
		{
			Enjon::Vec3 F = camera->Forward( );
			velDir += F;
		}
		if ( input->IsKeyDown( Enjon::KeyCode::S ) )
		{
			Enjon::Vec3 B = camera->Backward( );
			velDir += B;
		}
		if ( input->IsKeyDown( Enjon::KeyCode::A ) )
		{
			velDir += camera->Left( );
		}
		if ( input->IsKeyDown( Enjon::KeyCode::D ) )
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
		Enjon::Vec2 mouseCoords = input->GetMouseCoords( ); 

		// Set cursor to not visible
		window->ShowMouseCursor( false );

		// Reset the mouse coords after having gotten the mouse coordinates
		Vec2 center = GetCenterOfViewport( );
		//SDL_WarpMouseInWindow( window->GetWindowContext( ), (s32)center.x, (s32)center.y );
		SDL_WarpMouseInWindow( window->GetWindowContext( ), ( f32 )viewPort.x / 2.0f - mMouseCoordsDelta.x, ( f32 )viewPort.y / 2.0f - mMouseCoordsDelta.y );

		// Offset camera orientation
		f32 xOffset = Enjon::Math::ToRadians( ( f32 )viewPort.x / 2.0f - mouseCoords.x - mMouseCoordsDelta.x ) * mMouseSensitivity / 100.0f;
		f32 yOffset = Enjon::Math::ToRadians( ( f32 )viewPort.y / 2.0f - mouseCoords.y - mMouseCoordsDelta.y ) * mMouseSensitivity / 100.0f;
		camera->OffsetOrientation( xOffset, yOffset ); 
	}

	Vec2 EditorViewport::GetCenterOfViewport( )
	{
		return Vec2( mSceneViewWindowPosition.x + mSceneViewWindowSize.x / 2.0f, mSceneViewWindowPosition.y + mSceneViewWindowSize.y / 2.0f );
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

		guiContext->RegisterDockingLayout( GUIDockingLayout( mViewport->GetViewName().c_str(), nullptr, GUIDockSlotType::Slot_Tab, 1.0f ) ); 

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

	//======================================================================================================================

	EditorArchetypeEditWindow::EditorArchetypeEditWindow( const AssetHandle< Archetype >& archetype )
		: mInitialized( false ), mArchetype( archetype )
	{ 
	}

	//======================================================================================================================

	int EditorArchetypeEditWindow::Init( std::string windowName, int screenWidth, int screenHeight, WindowFlagsMask currentFlags )
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
			mWorld->RegisterContext< EntitySubsystemContext >( );

			ConstructScene( );

			mInitialized = true;
		} 

		return 1; 
	}

	//======================================================================================================================

	void EditorArchetypeEditWindow::ConstructScene( )
	{ 
		GUIContext* guiContext = GetGUIContext( );

		EditorApp* app = Engine::GetInstance( )->GetApplication( )->ConstCast< EditorApp >( );

		// Create viewport
		mViewport = new EditorViewport( app, this );

		// Create world outliner
		mWorldOutlinerView = new EditorWorldOutlinerView( app, this ); 

		// Create inspector view
		mInspectorView = new EditorInspectorView( app, this );

		// Register callback for entity selection
		mWorldOutlinerView->RegisterEntitySelectionCallback( [ & ] ( const EntityHandle& handle )
		{
			mInspectorView->SetInspetedObject( handle.Get( ) );
		});

		// Register callback for entity deselection
		mWorldOutlinerView->RegisterEntityDeselectionCallback( [ & ] ( )
		{
			mInspectorView->DeselectInspectedObject( );
		});
 
		// NOTE(John): For some reason, grabbing the view name of the docking window does not work with this function call... who knows?
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Viewport", nullptr, GUIDockSlotType::Slot_Tab, 1.0f ) ); 
		guiContext->RegisterDockingLayout( GUIDockingLayout( "World Outliner", "Viewport", GUIDockSlotType::Slot_Right, 0.45f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Inspector", "World Outliner", GUIDockSlotType::Slot_Top, 0.45f ) );

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

		// Construct entity from archetype
		if ( mArchetype )
		{
			Archetype* archType = mArchetype.Get( )->ConstCast< Archetype >( );
			Transform t;
			t.SetPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );
			t.SetScale( 2.0f );
			EntityHandle entity = archType->Instantiate( t, GetWorld( ) );
		}

		// Register callbacks for whenever project is reloaded ( dll reload )
		app->RegisterReloadDLLCallback( [ & ] ( )
		{
			// What do we need to do on reload? Make sure that the entity is reset? 
		} ); 
	}

	//======================================================================================================================
}
