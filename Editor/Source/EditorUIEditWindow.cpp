#include "EditorUIEditWindow.h"

#include <Graphics/GraphicsSubsystem.h>
#include <Utils/FileUtils.h>
#include <IO/InputManager.h>

namespace Enjon
{
	//=================================================================================

	void EditorUIEditWindow::Init( const WindowParams& params )
	{ 
		// Construct scene in world
		if ( !mUI )
		{
			// Initialize new world 
			mWorld = new World( );
			// Register contexts with world
			mWorld->RegisterContext< GraphicsSubsystemContext >( );
			// Set asset from data 
			mUI = ( UI* )( params.mData );
			// Construct Scene 
			ConstructScene( );
		} 
	}

	//=================================================================================

	ImVec2 EditorUIEditWindow::HandleMousePan()
	{ 
		static ImVec2 onClickPos = ImVec2( 0.f, 0.f );
		static bool justClicked = false;
		bool isHovered = ImGui::IsMouseHoveringWindow();
		bool isMouseDown = ImGui::IsMouseDown( 0 );
		bool isMouseClicked = ImGui::IsMouseClicked( 0 );
		bool isMouseReleased = ImGui::IsMouseReleased( 0 );
		ImVec2 mp = ImGui::GetMousePos();

		ImVec2 delta = ImVec2( 0.f, 0.f );

		// If not dragging an element. This is where it gets shitty.
		if ( isHovered && &justClicked && isMouseClicked )
		{
			onClickPos = mp;
			justClicked = true;
		} 

		// Drag
		if ( justClicked && isMouseDown && !mSelectedElement )
		{
			delta = ImVec2( mp.x - onClickPos.x, mp.y - onClickPos.y ); 
			onClickPos = mp;
		}

		//Utils::TempBuffer buffer = Utils::TransientBuffer( "Delta: <%.2f, %.2f>, MP: <%.2f, %.2f>, OCP: <%.2f, %.2f>", 
		//										delta.x, delta.y, mp.x, mp.y, onClickPos.x, onClickPos.y );
		//printf( "%s\n", buffer );

		// Reset
		if ( isMouseReleased )
		{ 
			onClickPos = ImVec2( 0.f, 0.f );
			delta = ImVec2( 0.f, 0.f );
			justClicked = false;
		} 

		return delta;
	}

	//===============================================================================================

	void EditorUIEditWindow::SelectElement( UIElement* element )
	{
		mSelectedElement = element;
	}

	//===============================================================================================

	void EditorUIEditWindow::DeselectElement()
	{ 
		mSelectedElement = nullptr;
	}

	//===============================================================================================

	void EditorUIEditWindow::DrawUI()
	{ 
		auto dl = ImGui::GetWindowDrawList();

		// Handle interacting with widget
		static ImVec2 onClickPos = ImVec2( 0.f, 0.f );
		static bool justClicked = false;

		// Do root canvas
		Vec2 pos = mUI->mRoot.mPosition;
		Transform lt = Transform( Vec3( pos, 1.f ), Quaternion(), Vec3( mUI->mRoot.mSize, 1.f ) );
		Transform wt = lt * mCamTransform; 
		ImVec2 a = ImVec2( wt.GetPosition().x, wt.GetPosition().y );
		ImVec2 b = ImVec2( a.x + wt.GetScale().x, a.y + wt.GetScale().y );

		bool hovered = ImGui::IsMouseHoveringRect( a, b ); 
		bool active = hovered && ImGui::IsMouseDown( 0 );
		bool selected = ( mSelectedElement == &mUI->mRoot );
		bool mouseClicked = ImGui::IsMouseClicked( 0 );
		bool mouseDown = ImGui::IsMouseDown( 0 );
		ImVec2 mp = ImGui::GetMousePos();

		if ( mouseClicked && !hovered && selected )
		{
			DeselectElement();
			selected = false;
		} 

		if ( hovered && mouseClicked )
		{
			DeselectElement();
			SelectElement( &mUI->mRoot );
			onClickPos = ImGui::GetMousePos();
		} 
		
		ImColor activeColor = ImColor( 0.9f, 0.4f, 0.1f, 1.f );
		ImColor hoveredColor = ImColor( 0.8f, 0.2f, 0.f, 1.f );
		ImColor color = ImColor( 1.f, 1.f, 1.f, 1.f );

		// Move element if selected
		ImVec2 delta = ImVec2( 0.f, 0.f );
		if ( mouseDown && selected )
		{ 
			delta = ImVec2( mp.x - onClickPos.x, mp.y - onClickPos.y ); 
			onClickPos = mp;
		} 

		dl->AddRect( a, b, hovered ? hoveredColor : active || selected ? activeColor : color ); 

		mUI->mRoot.mPosition += Vec2( delta.x / mCamTransform.GetScale().x, delta.y / mCamTransform.GetScale().y );
	}

	//===============================================================================================

	void AddDashedLine( const ImVec2& a, const ImVec2& b, const ImColor& color )
	{
		auto dl = ImGui::GetWindowDrawList();

		Vec2 diff = ( Vec2( b.x, b.y ) - Vec2( a.x, a.y ) );
		Vec2 norm = Vec2::Normalize( diff );
		f32 length = diff.Length();
		u32 numSteps = 20;
		f32 sa = length / (f32)numSteps;

		Vec2 start = Vec2( a.x, a.y );
		ImVec2 la = a;
		for ( f32 s = 0.f; s <= length; s += sa )
		{
			Vec2 la = start + norm * s;
			Vec2 lb = start + norm * ( s + sa * 0.5f );
			if ( lb.x > b.x || lb.y > b.y ) lb = Vec2( b.x, b.y );
			dl->AddLine( ImVec2( la.x, la.y ), ImVec2( lb.x, lb.y ), color );
		}
	}

	void AddDashedLineRect( const ImVec2& a, const ImVec2& b, const ImColor& color )
	{
		auto dl = ImGui::GetWindowDrawList();
		ImVec2 tl = a;
		ImVec2 tr = ImVec2( b.x, a.y );
		ImVec2 bl = ImVec2( a.x, b.y );
		ImVec2 br = b;
 
		AddDashedLine( a, tr, color );
		AddDashedLine( tr, br, color );
		AddDashedLine( bl, br, color );
		AddDashedLine( a, bl, color ); 

		dl->AddLine( a, ImVec2( a.x + 10.f, a.y ), color );
		dl->AddLine( a, ImVec2( a.x, a.y + 10.f ), color );
		dl->AddLine( bl, ImVec2( bl.x, bl.y - 10.f ), color );
		dl->AddLine( bl, ImVec2( bl.x + 10.f, bl.y ), color );
		dl->AddLine( tr, ImVec2( tr.x - 10.f, tr.y ), color );
		dl->AddLine( tr, ImVec2( tr.x, tr.y + 10.f ), color );
		dl->AddLine( br, ImVec2( br.x - 10.f, br.y ), color );
		dl->AddLine( br, ImVec2( br.x, br.y - 10.f ), color );
	}

	void EditorUIEditWindow::Canvas()
	{ 
		// Don't want this to be scrollable, however...
		//ImGui::SetNextWindowPos( ImVec2( ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 46.f ) );
		//ImGui::SetNextWindowSize( ImVec2( ImGui::GetWindowWidth() - 20.f, ImGui::GetWindowHeight() - 15.f ) );
		//ImGui::Begin( "##canvas_window", nullptr,
		//	ImGuiWindowFlags_NoMove |
		//	ImGuiWindowFlags_NoResize |
		//	ImGuiWindowFlags_NoTitleBar | 
		//	ImGuiWindowFlags_NoBringToFrontOnFocus
		//);
		ImGui::ListBoxHeader( "##canvas", ImVec2( ImGui::GetWindowWidth() - 20.f, ImGui::GetWindowHeight() - 15.f ) );
		{ 
			if ( !mCanvasInitialized )
			{
				Vec2 vs = Vec2( 900.f, 506.f );		// By default, it's the size of the Sandbox's window
				mViewportTransform = Transform( Vec3( 0.f ), Quaternion(), Vec3( vs.x, vs.y, 1.f ) );
				ImVec2 co = ImVec2( ImGui::GetWindowPos().x + ( ImGui::GetWindowWidth() - vs.x ) / 2.f , ImGui::GetWindowPos().y + ( ImGui::GetWindowHeight() - vs.y ) / 2.f );
				mCamTransform = Transform( Vec3( co.x, co.y, 0.f ), Quaternion(), Vec3( 0.5f ) ); 
				mCanvasInitialized = true;
			} 

			// Just want to draw a bunch of lines for a canvas
			// Do da canvas stuff 
			// Want to add lines for the 'origin' of the viewport, but want to be able to pan the canvas around 
			// Perhaps just draw the viewport in relation to the camera's transform?   

			bool win_hov = ImGui::IsMouseHoveringWindow();

			Input* input = EngineSubsystem( Input );

			auto dl = ImGui::GetWindowDrawList(); 

			ImVec2 delta = HandleMousePan(); 
			mCamTransform.SetPosition( mCamTransform.GetPosition() + Vec3( delta.x, delta.y, 0.f ) );
			if ( win_hov && input->GetMouseWheel().y ) 
			{ 
				Transform wtb = mViewportTransform * mCamTransform; 
				ImVec2 lba = ImVec2( wtb.GetPosition().x, wtb.GetPosition().y );
				ImVec2 lbb = ImVec2( lba.x + wtb.GetScale().x, lba.y + wtb.GetScale().y );		// Should I know about the resolution of the desired image? I think so...

				f32 y = input->GetMouseWheel().y / 10.f;
				f32 oscl = mCamTransform.GetScale().x;
				f32 scl = Math::Clamp( mCamTransform.GetScale().x + y, 0.1f, 10.f );
				mCamTransform.SetScale( Math::Clamp( mCamTransform.GetScale().x + y, 0.1f, 10.f ) );

				Transform wta = mViewportTransform * mCamTransform; 
				ImVec2 laa = ImVec2( wta.GetPosition().x, wta.GetPosition().y );
				ImVec2 lab = ImVec2( laa.x + wta.GetScale().x, laa.y + wta.GetScale().y );		// Should I know about the resolution of the desired image? I think so...

				Vec2 ca = Vec2( ( laa.x + lab.x ) / 2.f, ( laa.y + lab.y ) / 2.f );
				Vec2 cb = Vec2( ( lba.x + lbb.x ) / 2.f, ( lba.y + lbb.y ) / 2.f );
				Vec2 dc = cb - ca;

				// Also want to move the camera center towards the mouse
				// The camera's center needs to move towards the mouse ( the mouse needs to be transformed into the viewport space )
				if ( (oscl - scl) != 0.f )
				{
					// Have to transform the mouse position into the viewport's space first, THEN can transform it into the camera's position
					// How to transform the mp into window space? 
					Vec2 mp = Vec2( ImGui::GetMousePos().x, ImGui::GetMousePos().y ); 
					Vec2 cp = Vec2( mCamTransform.GetPosition().x, mCamTransform.GetPosition().y ); 
					Transform mt = Transform( Vec3( mp.x, mp.y, 0.f ), Quaternion(), Vec3( 1.f ) );
					Transform wt = mt * mCamTransform; 
					//mp = Vec2( wt.GetPosition().x, wt.GetPosition().y );
					Vec3 diff = (wtb.GetPosition() - wta.GetPosition());
					//dc = Vec2( wt.GetPosition().x - ca.x, wt.GetPosition().y - ca.y ); // This fucking blows...
					//dc.x -= wt.GetPosition().x;
					//dc.y -= wt.GetPosition().y;
					//Vec2 norm = Vec2::Normalize( diff );
					//f32 length = diff.Length();
					//cp += norm * length * scl;
					Utils::TempBuffer buff = Utils::TransientBuffer( "Diff< %.2f, %.2f >\n", dc.x, dc.y );
					printf( buff.buffer ); 
					mCamTransform.SetPosition( mCamTransform.GetPosition() + Vec3( dc.x, dc.y, 0.f ) );
				}
			} 

			{
				//Vec2 cp = Vec2( mCamTransform.GetPosition().x, mCamTransform.GetPosition().y );
				//Vec2 mp = Vec2( ImGui::GetMousePos().x, ImGui::GetMousePos().y );
				//Utils::TempBuffer buff = Utils::TransientBuffer( "CP: <%.2f, %.2f>, MP: <%.2f, %.2f>\n", cp.x, cp.y, mp.x, mp.y );
				//printf( buff.buffer ); 
			}

			Transform wt = mViewportTransform * mCamTransform; 

			ImVec2 la = ImVec2( wt.GetPosition().x, wt.GetPosition().y );
			ImVec2 lb = ImVec2( la.x + wt.GetScale().x, la.y + wt.GetScale().y );		// Should I know about the resolution of the desired image? I think so...

			// What about some grid lines... 
			Vec3 cp = mCamTransform.GetPosition();
			dl->AddLine( ImVec2( la.x, la.y - 1000.f - cp.y ), ImVec2( la.x, la.y + 1000.f + cp.y ), ImColor( 0.1f, 0.8f, 0.2f, 0.3f ) );
			dl->AddLine( ImVec2( la.x - 1000.f - cp.x, la.y ), ImVec2( la.x + 1000.f + cp.x, la.y ), ImColor( 0.1f, 0.8f, 0.2f, 0.3f ) ); 

			// Viewport
			// Want to add a dotted rect, actually
			AddDashedLineRect( la, lb, ImColor( 0.4f, 0.4f, 0.4f, 1.f ) );
			//dl->AddRect( la, lb, ImColor( 0.4f, 0.4f, 0.4f, 0.4f ) ); 

			DrawUI();
		}
		ImGui::ListBoxFooter();
		//ImGui::End(); 
	}

	//=================================================================================

	void EditorUIEditWindow::ConstructScene()
	{ 
		GUIContext* guiContext = GetGUIContext( );

		// Add main menu options
		guiContext->RegisterMainMenu( "File" ); 
		guiContext->RegisterMainMenu( "Random" ); 

		World* world = GetWorld( ); 

		guiContext->RegisterWindow( "Canvas", [ & ] 
		{
			if ( ImGui::BeginDock( "Canvas" ) )
			{
				Canvas(); 
				ImGui::EndDock();
			}
		}); 

		guiContext->RegisterWindow( "Properties", [ & ]
		{
			if ( ImGui::BeginDock( "Properties" ) )
			{
				ImGui::ListBoxHeader( "##asset_props", ImVec2( ImGui::GetWindowWidth() - 20.f, ImGui::GetWindowHeight() - 15.f ) );
				{
					if ( mUI )
					{ 
						if ( ImGui::Button( "Add Button" ) )
						{
							mUI->mRoot.AddChild( new UIElementButton() ); 
						}

						World* world = GetWorld( );
						ImGui::Text( "     UI:" ); ImGui::SameLine(); ImGui::SetCursorPosX( ImGui::GetWindowWidth() * 0.4f );
						ImGui::Text( "%s", Utils::format( "%s", mUI.Get( )->GetName().c_str() ).c_str( ) );
						ImGuiManager* igm = EngineSubsystem( ImGuiManager );
						igm->InspectObject( &mUI->mRoot ); 
					} 
				}
				ImGui::ListBoxFooter();
				ImGui::EndDock( );
			}
		});

		auto saveAssetOption = [ & ] ( )
		{
			if ( ImGui::MenuItem( "Save##save_asset_option", NULL ) )
			{
				if ( mUI )
				{
					mUI->Save();
				}
			}
		};

		// Register menu options
		guiContext->RegisterMenuOption( "File", "Save##save_asset_option", saveAssetOption ); 
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Canvas", nullptr, GUIDockSlotType::Slot_Tab, 0.45f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Properties", "Canvas", GUIDockSlotType::Slot_Left, 0.25f ) );
		guiContext->SetActiveDock( "Canvas" );
		guiContext->Finalize( ); 
	}

	//=================================================================================

}
