
#include "EditorArchetypeEditWindow.h" 
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

namespace Enjon 
{ 
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

	void EditorArchetypeEditWindow::Init( const WindowParams& params )
	{
		// Construct scene in world
		if ( !mInitialized )
		{ 
			// Initialize new world 
			mWorld = new World( );
			// Register contexts with world
			mWorld->RegisterContext< GraphicsSubsystemContext >( );
			mWorld->RegisterContext< EntitySubsystemContext >( );

			mArchetype = (Archetype*)params.mData;

			ConstructScene( );

			mInitialized = true;
		}
	}

	//======================================================================================================================

	void EditorArchetypeEditWindow::ConstructScene( )
	{
		GUIContext* guiContext = GetGUIContext( );

		EditorApp* app = Engine::GetInstance( )->GetApplication( )->ConstCast< EditorApp >( );

		// Add main menu options
		guiContext->RegisterMainMenu( "File" );
		guiContext->RegisterMainMenu( "Create" );
		guiContext->RegisterMainMenu( "View" );

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
		} );

		// Register callback for entity deselection
		mWorldOutlinerView->RegisterEntityDeselectionCallback( [ & ] ( )
		{
			mInspectorView->DeselectInspectedObject( );
		} );

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

		auto createViewOption = [ & ] ( )
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

				for ( auto& e : mRootEntity.Get( )->GetInstancedEntities( ) )
				{
					Archetype::RecursivelyMergeEntities( mRootEntity, e, MergeType::AcceptMerge );
				}
			}
		};

		// Register menu options
		guiContext->RegisterMenuOption( "Create", "Create", createViewOption );
		guiContext->RegisterMenuOption( "File", "Save##archetype_options", saveArchetypeOption );

		// Register archetype drop callback with viewport
		mViewport->SetViewportCallback( ViewportCallbackType::AssetDropArchetype, [ & ] ( const void* data )
		{
			const Asset* asset = ( const Asset* )data;
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
					Vec3 position = cam->GetPosition( ) + cam->Forward( ) * 5.0f;
					Vec3 scale = archType->GetRootEntity( ).Get( )->GetLocalScale( );
					EntityHandle handle = archType->Instantiate( Transform( position, Quaternion( ), scale ), GetWorld( ) );

					// Add to root entity
					mRootEntity.Get( )->AddChild( handle );
				}
			}
		} );

		//guiContext->Finalize( );
	}

	//======================================================================================================================

	void EditorTransformWidgetToolBar::UpdateView( )
	{
		ImGui::ListBoxHeader( "##TransformOptions", ImVec2( Math::Min( ImGui::GetWindowSize( ).x * 0.95f, 900.0f ), Math::Min( ImGui::GetWindowSize( ).y * 0.7f, 35.0f ) ) );
		{
			// Translation snapping
			bool tSnap = mApp->IsTransformSnappingEnabled( TransformationMode::Translation );
			if ( ImGui::Checkbox( "Translation Snap", &tSnap ) )
			{
				mApp->EnableTransformSnapping( tSnap, TransformationMode::Translation );
			}

			ImGui::SameLine( );

			f32 tSnapVal = mApp->GetTransformSnap( TransformationMode::Translation );
			ImGui::PushItemWidth( Math::Min( ImGui::GetWindowSize( ).x * 0.95f, 100.0f ) );
			if ( ImGui::SliderFloat( "##TranslationSnapAmount", &tSnapVal, 0.0f, 10.0f, "%.1f", 1.0f ) )
			{
				mApp->SetTransformSnap( TransformationMode::Translation, tSnapVal );
			}
			ImGui::PopItemWidth( );

			ImGui::SameLine( );

			// Scale snapping
			bool sSnap = mApp->IsTransformSnappingEnabled( TransformationMode::Scale );
			if ( ImGui::Checkbox( "Scale Snap", &sSnap ) )
			{
				mApp->EnableTransformSnapping( sSnap, TransformationMode::Scale );
			}

			ImGui::SameLine( );

			f32 sSnapVal = mApp->GetTransformSnap( TransformationMode::Scale );
			ImGui::PushItemWidth( Math::Min( ImGui::GetWindowSize( ).x * 0.95f, 100.0f ) );
			if ( ImGui::SliderFloat( "##ScaleSnapAmount", &sSnapVal, 0.0f, 10.0f, "%.1f", 1.0f ) )
			{
				mApp->SetTransformSnap( TransformationMode::Scale, sSnapVal );
			}
			ImGui::PopItemWidth( );

			ImGui::SameLine( );

			// Rotation snapping
			bool rSnap = mApp->IsTransformSnappingEnabled( TransformationMode::Rotation );
			if ( ImGui::Checkbox( "Rotation Snap", &rSnap ) )
			{
				mApp->EnableTransformSnapping( rSnap, TransformationMode::Rotation );
			}

			ImGui::SameLine( );

			f32 rSnapVal = mApp->GetTransformSnap( TransformationMode::Rotation );
			ImGui::PushItemWidth( Math::Min( ImGui::GetWindowSize( ).x * 0.95f, 100.0f ) );
			if ( ImGui::SliderFloat( "##RotationSnapAmount", &rSnapVal, 0.0f, 25.0f, "%.1f", 1.0f ) )
			{
				mApp->SetTransformSnap( TransformationMode::Rotation, rSnapVal );
			}
			ImGui::PopItemWidth( );

			// Display the transform's transformation type
			EditorTransformWidget* tw = mApp->GetTransformWidget( );
			TransformSpace ts = tw->GetTransformSpace( );

		}
		ImGui::ListBoxFooter( );
	}

	//====================================================================================================================== 
}