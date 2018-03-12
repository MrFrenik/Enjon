#include "EditorView.h"
#include "EditorApp.h"
 
#include <Engine.h>
#include <IO/InputManager.h>
#include <SubsystemCatalog.h>
#include <ImGui/ImGuiManager.h>

#include <fmt/format.h>

namespace Enjon
{
	//=====================================================================

	EditorView::EditorView( EditorApp* app, const String& name, const u32& flags )
		: mApp( app ), mName( name ), mViewFlags( flags )
	{
		ImGuiManager::RegisterMenuOption( "View", [ & ] ( )
		{
			ImGui::MenuItem( fmt::format( "{}##options", mName ).c_str( ), NULL, &mViewEnabled );
		});

		// Register individual window with docking system
		Enjon::ImGuiManager::RegisterWindow( [ & ] ( )
		{
			// Docking windows
			if ( ImGui::BeginDock( GetViewName().c_str(), &mViewEnabled, GetViewFlags() ) )
			{
				Update( );
			}
			ImGui::EndDock( ); 
		}); 
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
		EditorWidgetManager* wm = mApp->GetEditorWidgetManager( );
		if ( wm->GetFocused( this ) )
		{
			ProcessInput( ); 
		}

		//bool isHovered = wm->GetHovered( this );
		//bool isFocused = wm->GetFocused( this );

		//std::cout << fmt::format( "{} - focused: {}, hovered: {}\n", mName, isFocused, isHovered );
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
		EditorWidgetManager* wm = mApp->GetEditorWidgetManager( );

		// Capture hovered state
		bool isHovered = ImGui::IsMouseHoveringWindow( ); 
		wm->SetHovered( this, isHovered );

		// Capture focused state
		bool isFocused = ( isHovered &&
			( input->IsKeyPressed( KeyCode::LeftMouseButton ) ||
				input->IsKeyPressed( KeyCode::RightMouseButton ) ||
				input->IsKeyDown( KeyCode::LeftMouseButton ) ||
				input->IsKeyDown( KeyCode::RightMouseButton ) ) );
		wm->SetFocused( this, isFocused );
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
}
