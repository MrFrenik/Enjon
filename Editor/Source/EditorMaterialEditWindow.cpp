
#include "EditorMaterialEditWindow.h"
#include "EditorWorldOutlinerView.h"
#include "EditorAssetBrowserView.h"
#include "EditorApp.h"

#include <Graphics/GraphicsSubsystem.h>
#include <Scene/SceneManager.h>
#include <Asset/AssetManager.h>
#include <Entity/Archetype.h>
#include <Graphics/StaticMeshRenderable.h>
#include <Entity/Components/StaticMeshComponent.h>
#include <Entity/Components/SkeletalMeshComponent.h>
#include <Serialize/ObjectArchiver.h>
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

		EditorAssetBrowserView* abv = mApp->GetEditorAssetBrowserView( );
		if ( abv->GetGrabbedAsset( ) && mWindow->IsMouseInWindow( ) && Window::NumberOfHoveredWindows() == 1 )
		{
			mWindow->SetFocus( );
		} 

		if ( abv->GetGrabbedAsset( ) )
		{ 
			Vec2 mp = GetSceneViewProjectedCursorPosition( );

			if ( mWindow->IsMouseInWindow( ) && mWindow->IsFocused( ) )
			{ 
				String label = fmt::format( "Asset: {}", abv->GetGrabbedAsset()->GetName( ) ).c_str( );
				ImVec2 txtSize = ImGui::CalcTextSize( label.c_str( ) );
				ImGui::SetNextWindowPos( ImVec2( ImGui::GetMousePos( ).x + 15.0f, ImGui::GetMousePos().y + 5.0f ) );
				ImGui::SetNextWindowSize( ImVec2( txtSize.x + 20.0f, txtSize.y ) );
				ImGui::Begin( "##grabbed_asset_window", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar );
				{
					ImGui::Text( label.c_str( ) );
				}
				ImGui::End( ); 

				if ( ImGui::IsMouseReleased( 0 ) && IsHovered( ) )
				{
					HandleAssetDrop( );
				} 
			} 
		}

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

	//===============================================================================================

	void EditorViewport::SetViewportCallback( ViewportCallbackType type, const AssetCallback& callback )
	{
		mViewportCallbacks[ type ] = callback;
	}

	//===============================================================================================

	Vec2 EditorViewport::GetSceneViewProjectedCursorPosition( )
	{
		// Need to get percentage of screen and things and stuff from mouse position
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem ); 
		Input* input = EngineSubsystem( Input );
		iVec2 viewport = gfx->GetViewport( );
		Vec2 mp = input->GetMouseCoords( );
		
		// X screen percentage
		f32 pX = f32( mp.x - mSceneViewWindowPosition.x ) / f32( mSceneViewWindowSize.x );
		f32 pY = f32( mp.y - mSceneViewWindowPosition.y ) / f32( mSceneViewWindowSize.y ); 

		return Vec2( (s32)( pX * viewport.x ), (s32)( pY * viewport.y ) );
	}

	bool EditorViewport::HasViewportCallback( const ViewportCallbackType& type )
	{
		return ( mViewportCallbacks.find( type ) != mViewportCallbacks.end( ) );
	}

	void EditorViewport::HandleAssetDrop( )
	{
		// Get projected mouse position
		Vec2 mp = GetSceneViewProjectedCursorPosition( );

		const Asset* grabbedAsset = mApp->GetEditorAssetBrowserView( )->GetGrabbedAsset( );

		// If material, then set material of overlapped object with this asset
		if ( grabbedAsset->Class( )->InstanceOf< Material >( ) )
		{
			// Check against graphics system for object
			GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem ); 

			PickResult pickRes = gfx->GetPickedObjectResult( mp, mWindow->GetWorld( )->GetContext< GraphicsSubsystemContext >( ) );

			u32 subMeshIdx = pickRes.mSubmeshIndex;

			Entity* ent = pickRes.mEntity.Get( );

			if ( ent )
			{
				// If static mesh component
				if ( ent->HasComponent< StaticMeshComponent >( ) )
				{
					StaticMeshComponent* smc = ent->GetComponent< StaticMeshComponent >( );
					smc->SetMaterial( grabbedAsset );
				}
				else if ( ent->HasComponent< SkeletalMeshComponent >( ) )
				{
					SkeletalMeshComponent* smc = ent->GetComponent< SkeletalMeshComponent >( );
					smc->SetMaterial( grabbedAsset );
				}
			}
		} 
		else if ( grabbedAsset->Class( )->InstanceOf< Archetype >( ) )
		{
			if ( HasViewportCallback( ViewportCallbackType::AssetDropArchetype ) )
			{
				mViewportCallbacks[ ViewportCallbackType::AssetDropArchetype ]( grabbedAsset );
			}

			//Archetype* archType = grabbedAsset->ConstCast< Archetype >( );
			//if ( archType )
			//{
			//	// Instantiate the archetype right in front of the camera for now
			//	GraphicsSubsystemContext* gfxCtx = GetWindow( )->GetWorld( )->GetContext< GraphicsSubsystemContext >( );
			//	Camera* cam = gfxCtx->GetGraphicsScene( )->GetActiveCamera( );
			//	Vec3 position = cam->GetPosition() + cam->Forward( ) * 5.0f; 
			//	Vec3 scale = archType->GetRootEntity( ).Get( )->GetLocalScale( );
			//	EntityHandle handle = archType->Instantiate( Transform( position, Quaternion( ), scale ), GetWindow()->GetWorld() );
			//}
		}
		else if ( grabbedAsset->Class( )->InstanceOf< SkeletalMesh >( ) )
		{
			// Do things...
		}
		else if ( grabbedAsset->Class( )->InstanceOf< Mesh >( ) )
		{
			// Construct new entity in front of camera
			Mesh* mesh = grabbedAsset->ConstCast< Mesh >( );
			if ( mesh )
			{
				// Instantiate the archetype right in front of the camera for now
				GraphicsSubsystemContext* gfxCtx = GetWindow( )->GetWorld( )->GetContext< GraphicsSubsystemContext >( );
				Camera* cam = gfxCtx->GetGraphicsScene( )->GetActiveCamera( );
				Vec3 position = cam->GetPosition() + cam->Forward( ) * 5.0f; 
				EntityHandle handle = EngineSubsystem( EntityManager )->Allocate( mWindow->GetWorld( ) );
				if ( handle )
				{
					Entity* newEnt = handle.Get( );
					StaticMeshComponent* smc = newEnt->AddComponent< StaticMeshComponent >( );
					smc->SetMesh( mesh );
					newEnt->SetLocalPosition( position );
					newEnt->SetName( mesh->GetName( ) );
				}
			}
		}
		
	}

	//===============================================================================================

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


	s32 EditorMaterialEditWindow::Init( const String& windowName, const s32& screenWidth, const s32& screenHeight, WindowFlagsMask currentFlags )
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

	void EditorArchetypeEditWindow::ExplicitDestroy( ) 
	{
		if ( mViewport )
		{
			delete ( mViewport );
			mViewport = nullptr;
		}

		if ( mRootEntity )
		{
			mRootEntity.Get( )->MoveToWorld( EngineSubsystem( EntityManager )->GetArchetypeWorld( ) );
		}

		if ( mWorldOutlinerView )
		{
			delete ( mWorldOutlinerView );
			mWorldOutlinerView = nullptr;
		}

		if ( mInspectorView )
		{
			delete ( mInspectorView );
			mInspectorView = nullptr;
		}
	}

	//======================================================================================================================

	s32 EditorArchetypeEditWindow::Init( const String& windowName, const s32& screenWidth, const s32& screenHeight, WindowFlagsMask currentFlags )
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
		guiContext->RegisterDockingLayout( GUIDockingLayout( "World Outliner", "Viewport", GUIDockSlotType::Slot_Right, 0.25f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Inspector", "World Outliner", GUIDockSlotType::Slot_Top, 0.50f ) );

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

			// Force reload of archType
			if ( archType->GetRootEntity( ) )
			{
				archType->Reload( ); 
			}

			Transform t;
			t.SetPosition( cam->GetPosition( ) + cam->Forward( ) * 3.0f );
			t.SetScale( 2.0f );
			//mRootEntity = archType->CopyRootEntity( t, GetWorld( ) );
			mRootEntity = archType->GetRootEntity( );
			mRootEntity.Get( )->MoveToWorld( GetWorld( ) );
			mRootEntity.Get( )->SetLocalPosition( t.GetPosition( ) );
			cam->SetPosition( cam->GetPosition( ) + Vec3( 0.0f, 2.0f, 0.0f ) );
			cam->LookAt( mRootEntity.Get( )->GetWorldPosition( ) );
		}

		// Register callbacks for whenever project is reloaded ( dll reload )
		app->RegisterReloadDLLCallback( [ & ] ( )
		{
			// What do we need to do on reload? Make sure that the entity is reset? 
		} );

		auto createViewOption = [&]()
		{
			// Construct new empty entity
			if ( ImGui::MenuItem( "Empty##options", NULL ) )
			{
				EntityManager* em = EngineSubsystem( EntityManager );
				EntityHandle empty = em->Allocate( GetWorld( ) );
				empty.Get( )->SetName( "Empty" );

				// Parent to root entity
				mRootEntity.Get( )->AddChild( empty );

				// Set to selected entity
				mWorldOutlinerView->SelectEntity( empty ); 
			}
		}; 

		auto saveArchetypeOption = [ & ] ( )
		{ 
			if ( ImGui::MenuItem( "Save##archetype_options", NULL ) )
			{ 
				// Save after constructing
				mArchetype.Save( );

				for ( auto& e : mRootEntity.Get()->GetInstancedEntities() )
				{
					Archetype::RecursivelyMergeEntities( mRootEntity, e, MergeType::AcceptMerge );
				}
			}
		};

		// Register menu options
		guiContext->RegisterMenuOption( "Create", "Create", createViewOption );
		guiContext->RegisterMenuOption("File", "Save##archetype_options", saveArchetypeOption); 

		// Register archetype drop callback with viewport
		mViewport->SetViewportCallback( ViewportCallbackType::AssetDropArchetype, [ & ] ( const Asset* asset )
		{
			if ( asset )
			{
				// Cannot construct entity if asset is our archetype ( no recursive addition of archetypes )
				if ( asset == mArchetype.Get( ) )
				{
					std::cout << "Cannot recursively include self in archetype definition: " << asset->GetName( ) << "\n";
					return;
				}

				// Check the asset and look for any instance of this archetype in it
				if ( asset->ConstCast< Archetype >( )->ExistsInHierachy( mArchetype ) )
				{ 
					std::cout << "Cannot recursively include self in archetype definition: " << asset->GetName( ) << "\n";
					return;
				}

				std::cout << "Constructing: " << asset->GetName( ) << "\n"; 

				Archetype* archType = asset->ConstCast< Archetype >( );
				if ( archType )
				{
					// Instantiate the archetype right in front of the camera for now
					GraphicsSubsystemContext* gfxCtx = GetWorld( )->GetContext< GraphicsSubsystemContext >( );
					Camera* cam = gfxCtx->GetGraphicsScene( )->GetActiveCamera( );
					Vec3 position = cam->GetPosition() + cam->Forward( ) * 5.0f; 
					Vec3 scale = archType->GetRootEntity( ).Get( )->GetLocalScale( );
					EntityHandle handle = archType->Instantiate( Transform( position, Quaternion( ), scale ), GetWorld() );

					// Add to root entity
					mRootEntity.Get( )->AddChild( handle );
				} 
			}
		}); 
	}

	//======================================================================================================================
}
