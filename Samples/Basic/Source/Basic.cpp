
// Project includes
#include "Basic.h"

using namespace Enjon;

// Module implementation definition
ENJON_MODULE_DEFINE( Basic )

//==================================================================

void Basic::OnLoad( )
{ 
	// Need to bring this window to the front...
	// Here we'll launch a "tutorial" window to explain to the folks at home what's going on
	std::cout << "Loading basic tutorial...\n";

	WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );
	WindowParams params;
	params.mWindowClass = Object::GetClass< Window >( );
	params.mWidth = 1200;
	params.mHeight = 500;
	params.mName = "Sample: Basic Tutorial";
	//params.mFlags = WindowFlags::BORDERLESS;
	mWindowID = ws->AddNewWindow( params );
	ws->ForceInitWindows( ); 
	Window* window = ws->GetWindow( mWindowID ); 

	GUIContext* guiCtx = window->GetGUIContext( ); 

	// View all projects loaded currently. This list should be held in a project manifest for the editor.
	auto createProjectView = [ & ] ( )
	{
		Window* win = EngineSubsystem( WindowSubsystem )->GetWindow( mWindowID );
		win->GetGUIContext()->RootDock( ); 
		if ( ImGui::BeginDock( "Basic Tutorial Overview", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize ) )
		{
			ImGui::Text( "Welcome! This will eventually give you a basic overview of how to get started using the Enjon Editor." );
		}
		ImGui::EndDock( );
	}; 

	guiCtx->RegisterWindow( "Tutorial", createProjectView );
	guiCtx->RegisterDockingLayout( GUIDockingLayout( "Basic Tutorial Overview", nullptr, GUIDockSlotType::Slot_Top, 1.0f ) );
	guiCtx->SetActiveDock( "Basic Tutorial Overview" );
	guiCtx->Finalize( ); 

	// Focus on this window
	window->SetFocus( );
}

Result Basic::Initialize()
{
	// Show you how to do basic things in the editor/engine
	return Result::SUCCESS; 
}

//==================================================================

Result Basic::Update( f32 dt )
{
	return Result::PROCESS_RUNNING;
}

//==================================================================

Result Basic::Shutdown()
{
	return Result::SUCCESS;
}

//==================================================================

