#include "EditorView.h"
#include "EditorApp.h"
 
#include <Engine.h>
#include <IO/InputManager.h>
#include <SubsystemCatalog.h>
#include <ImGui/ImGuiManager.h>
#include <Graphics/Window.h>
#include <Utils/FileUtils.h>

namespace Enjon
{
	//=====================================================================

	EditorView::EditorView( EditorApp* app, Window* window, const String& name, const u32& flags )
		: mApp( app ), mName( name ), mViewFlags( flags ), mWindow( window )
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		// Grab gui context from window
		GUIContext* ctx = window->GetGUIContext( );
		assert( ctx != nullptr ); 

		ctx->RegisterMenuOption( "View", mName, [ & ] ( )
		{
			ImGui::MenuItem( Utils::format( "%s##options%zu", mName.c_str(), (u32)(usize)this ).c_str( ), NULL, &mViewEnabled );
		});

		// Register individual window with docking system
		ctx->RegisterWindow( mName, [ & ] ( )
		{
			// Docking windows
			if ( ImGui::BeginDock( GetViewName().c_str(), &mViewEnabled, GetViewFlags() ) )
			{
				Update( );
			}
			ImGui::EndDock( ); 
		}); 

		// Initialize the window
		Initialize( );
	}

	//=====================================================================

	void EditorView::Initialize( )
	{ 
	}

	//=====================================================================

	bool EditorView::IsFocused( ) const
	{	
		return mIsFocused;
	}

	//=====================================================================

	Window* EditorView::GetWindow( )
	{
		return mWindow;
	}

	//=====================================================================

	bool EditorView::IsHovered( ) const
	{
		return mIsHovered;
	}

	//=====================================================================

	bool EditorView::GetEnabled( )
	{
		return mViewEnabled;
	}

	//=====================================================================

	void EditorView::Update( )
	{
		// Set all state in here
		CaptureState( );

		// Call internal update
		this->UpdateView( );
 
		// Call process input if focused view
		//EditorWidgetManager* wm = mApp->GetEditorWidgetManager( ); 
		//if ( wm->GetFocused( this ) )
		if ( mIsFocused )
		{
			ProcessInput( ); 
		} 
	}

	//===================================================================== 

	void EditorView::ProcessInput( )
	{ 
		this->ProcessViewInput( );
	}

	//===================================================================== 

	void EditorView::CaptureState( )
	{ 
		Input* input = EngineSubsystem( Input );
		//EditorWidgetManager* wm = mApp->GetEditorWidgetManager( );

		// Get position and dimensions
		ImVec2 wp = ImGui::GetWindowPos( );
		ImVec2 ws = ImGui::GetWindowSize( );

		// Capture hovered state
		mIsHovered = ImGui::IsMouseHoveringRect( wp, ImVec2( wp.x + ws.x, wp.y + ws.y ) );
		//wm->SetHovered( this, isHovered );

		// Capture focused state
		bool isFocused = ( mIsHovered &&
			( input->IsKeyPressed( KeyCode::LeftMouseButton ) ||
				input->IsKeyPressed( KeyCode::RightMouseButton ) ||
				input->IsKeyDown( KeyCode::LeftMouseButton ) ||
				input->IsKeyDown( KeyCode::RightMouseButton ) ) );
		//wm->SetFocused( this, isFocused );
	}

	//===================================================================== 

	String EditorView::GetViewName( )
	{
		return mName;
	}

	//===================================================================== 

	u32 EditorView::GetViewFlags( )
	{
		return mViewFlags;
	}

	//===================================================================== 

	void EditorView::Enable( const b32& enabled )
	{
		mViewEnabled = enabled;
	}

	//===================================================================== 
}







