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

		f32 os = mCamera.GetOrthographicScale();
		return ImVec2( delta.x * os, delta.y * os );
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
	
	bool PointInCircle( const ImVec2& c, f32 r, const ImVec2& p )
	{ 
		return ( ( Vec2( p.x, p.y ) - Vec2( c.x, c.y ) ).Length() <= r );
	}

	void EditorUIEditWindow::DrawRectHandle( ImVec2* center )
	{
		auto dl = ImGui::GetWindowDrawList( ); 
		f32 r = 10.f / mCamera.GetOrthographicScale( );
		bool hovered = PointInCircle( *center, r, ImGui::GetMousePos() );
		dl->AddCircleFilled( *center, r, hovered ? ImColor( 0.9f, 0.2f, 0.1f, 1.f ) : ImColor( 1.f, 1.f, 1.f, 0.1f ) );
	}

	void EditorUIEditWindow::DrawUI()
	{ 
		auto dl = ImGui::GetWindowDrawList();

		// Handle interacting with widget
		static ImVec2 onClickPos = ImVec2( 0.f, 0.f );
		static bool justClicked = false;

		// Do root canvas
		Vec2 pos = mUI->mRoot.mPosition;
		Transform lt = Transform( Vec3( pos, 1.f ), Quaternion(), Vec3( mUI->mRoot.mSize, 1.f ) );

		Vec3 ra = mCamera.TransformPoint( Vec3( mUI->mRoot.mPosition, 0.f ) );
		Vec3 rb = mCamera.TransformPoint( Vec3( mUI->mRoot.mPosition, 0.f ) + Vec3( mUI->mRoot.mSize, 0.f ) ); 
		ImVec2 a = ImVec2( ra.x, ra.y );
		ImVec2 b = ImVec2( rb.x, rb.y );

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
			onClickPos = mp;
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

		// Transform position of stuff
		Vec3 fa = ra;
		fa += Vec3( delta.x, delta.y, 0.f );
		Vec3 fp = Mat4x4::Inverse( mCamera.GetViewProjectionMatrix( ) ) * fa; 
		mUI->mRoot.mPosition = Vec2( fp.x, fp.y );

		// Want to scale as well
		DrawRectHandle( &a );
		DrawRectHandle( &b );
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

	// TODO: when reaching zoom limits, stop camera movement as well
	// This is broken as shit...
	void ZoomOrthoCamera( Camera* cam, const Vec3& zoomTowards, const f32& amount )
	{
		// Calculate how much we will have to move towards the zoomTowards position
		f32 os = cam->GetOrthographicScale();
		float multiplier = (1.0f / os * amount);

		// Move camera
		Vec3 pos = cam->GetPosition();
		pos = (zoomTowards - pos) * multiplier + pos;

		// Zoom camera
		os -= amount;

		// Limit zoom
		f32 minZoom = 0.6f;
		f32 maxZoom = 5.f;
		os = Math::Clamp( os, minZoom, maxZoom ); 

		if ( os - cam->GetOrthographicScale( ) != 0.f )
		{
			cam->SetPosition( pos );
			cam->SetOrthographicScale( os ); 
		}
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
				mCanvasInitialized = true;
				mCamera = Camera( (u32)ImGui::GetWindowWidth(), (u32)ImGui::GetWindowHeight() );
				//mCamera.SetPosition( Vec3( co.x, co.y, 0.f ) );
				mCamera.SetPosition( Vec3( 0.f ) );
				mCamera.SetProjectionType( ProjectionType::Orthographic );
				mCamera.SetOrthographicScale( 1.f );
			} 

			// Just want to draw a bunch of lines for a canvas
			// Do da canvas stuff 
			// Want to add lines for the 'origin' of the viewport, but want to be able to pan the canvas around 
			// Perhaps just draw the viewport in relation to the camera's transform?   

			bool win_hov = ImGui::IsMouseHoveringWindow();

			Input* input = EngineSubsystem( Input );

			auto dl = ImGui::GetWindowDrawList(); 

			{
				Vec3 mp = Vec3( ImGui::GetMousePos().x, ImGui::GetMousePos().y, 0.f ); 
				Vec3 mpt = mCamera.TransformPoint( mp ); 
			}

			ImVec2 delta = HandleMousePan(); 
			//mCamTransform.SetPosition( mCamTransform.GetPosition() + Vec3( delta.x, delta.y, 0.f ) );
			mCamera.SetPosition( mCamera.GetPosition() - Vec3( delta.x, delta.y, 0.f ) );
			if ( win_hov && input->GetMouseWheel().y )
			{ 
				Vec3 to = Vec3( ImGui::GetMousePos().x, ImGui::GetMousePos().y, 0.f );
				ZoomOrthoCamera( &mCamera, to, input->GetMouseWheel().y / 10.f );
			} 

			Vec3 la_p = mCamera.TransformPoint( mViewportTransform.GetPosition() );
			Vec3 lb_p = mCamera.TransformPoint( mViewportTransform.GetPosition() + mViewportTransform.GetScale() ); 

			ImVec2 la = ImVec2( la_p.x, la_p.y );
			ImVec2 lb = ImVec2( lb_p.x, lb_p.y );		// Should I know about the resolution of the desired image? I think so...

			// What about some grid lines... 
			Vec3 cp = mViewportTransform.GetPosition();
			dl->AddLine( ImVec2( la.x, la.y - 1000.f ), ImVec2( la.x, la.y + 1000.f ), ImColor( 0.1f, 0.8f, 0.2f, 0.3f ) );
			dl->AddLine( ImVec2( la.x - 1000.f, la.y ), ImVec2( la.x + 1000.f, la.y ), ImColor( 0.1f, 0.8f, 0.2f, 0.3f ) ); 

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

						f32 ortho = mCamera.GetOrthographicScale( );
						ImGui::DragFloat( "Ortho", &ortho );
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
