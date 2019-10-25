

#include "EditorUIEditWindow.h"
#include "EditorMaterialEditWindow.h"
#include "EditorApp.h"

#include <Engine.h>
#include <Graphics/GraphicsSubsystem.h>
#include <Graphics/FrameBuffer.h>
#include <Utils/FileUtils.h>
#include <IO/InputManager.h>

#include <Utils/Property.h>

namespace Enjon
{ 

#define Vec2ToImVec2( v )\
	ImVec2( (v).x, (v).y )

	//=================================================================================

	UIRenderPass::UIRenderPass( const AssetHandle< UI >& ui, const Window* window )
		: mUI( ui ), mWindow( window )
	{ 
		mFrameBuffer = new FrameBuffer( 800, 800 ); 
		mGUIContext = new GUIContext( const_cast< Window* >( window ) ); 

		mWindow = new EditorUICanvasWindow();

		// Want to be able to use a custom gui context, but have to use a window. Why isn't this working? ...

		// Initialize ImGUI context and set
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		mGUIContext->SetContext( igm->GetContextByWindow( const_cast< Window* >( window ) ) );

		GUIContextParams params;
		params.mUseRootDock = false;
		mGUIContext->RegisterWindow( "UI", [ & ] () 
		{
			mUI->OnUI();
 
			ImGui::SetNextWindowPos( ImVec2( 0.f, 0.f ) );
			ImGui::Begin( "Blah" ); 
			{
				ImGui::Text( "Fucka you, dal-fehn!" );
			}
			ImGui::End();
		}); 
		mGUIContext->SetGUIContextParams( params );
		mGUIContext->Finalize();
	}

	//================================================================================= 

	void UIRenderPass::Render()
	{ 
		// Not sure what to do here, really...
		// Bind custom render target, I suppose? Then render out to that? Want a way to be able to save out the frame buffer 
		// Need to bind a camera as well
		mFrameBuffer->Bind();
		{ 
			// Clear default buffer
			//const_cast< Window* >( mWindow )->Clear( 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, RGBA32_Black() ); 

			GUIContext ctx = *const_cast< Window* >( mWindow )->GetGUIContext();
			const_cast< Window* >( mWindow )->SetGUIContext( *mGUIContext );

			ImGuiManager* igm = EngineSubsystem( ImGuiManager );
 
			// Queue up gui ( don't want to have to do this, obviously )
			igm->Render( const_cast< Window* >( mWindow ) ); 

			// I surpose...
			//mGUIContext->Render();		// This ain't durin' it right

			// Damnit, I can't do this. I just want to see the elements drawn to a fucking back buffer

			// Flush
			glViewport(0, 0, 800, 800);
			ImGui::Render(); 
			ImGui_ImplSdlGL3_RenderDrawData( ImGui::GetDrawData( ) ); 

			const_cast< Window* >( mWindow )->SetGUIContext( ctx );
		}
		mFrameBuffer->Unbind();
	}

	//=================================================================================

	u32 UIRenderPass::GetFramebufferTextureId() const
	{
		return (u32)mFrameBuffer->GetTexture();
	}

	//=================================================================================

	void EditorUIEditWindow::Init( const WindowParams& params )
	{ 
		// Construct scene in world
		if ( !mUI )
		{
			// Initialize new world 
			mWorld = new World( );

			// Register contexts with world
			GraphicsSubsystemContext* gCtx = mWorld->RegisterContext< GraphicsSubsystemContext >( );
 
			// Set asset from data 
			mUI = ( UI* )( params.mData ); 

			// Don't need to render the world with this context ( this will be abstracted away eventually into render pipelines )
			gCtx->EnableRenderWorld( false );

			// Construct Scene 
			ConstructScene( ); 
		} 
	}

	//=================================================================================

	void EditorUIEditWindow::Update()
	{ 
		Input* input = EngineSubsystem( Input );
		WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );

		for ( auto& wi : ws->GetWindows() )
		{
			if ( wi->Class()->InstanceOf< EditorUICanvasWindow >() )
			{
				f32 w = wi->GetScreenWidth();
				f32 h = wi->GetScreenHeight();

				bool refreshSize = false;
				if ( input->IsKeyPressed( KeyCode::Up ) ) {
					h += 10.f;
					refreshSize = true;
				}
				if ( input->IsKeyPressed( KeyCode::Down ) ) {
					h -= 10.f;
					refreshSize = true;
				}
				if ( input->IsKeyPressed( KeyCode::Left ) ) {
					w -= 10.f;
					refreshSize = true;
				}
				if ( input->IsKeyPressed( KeyCode::Right ) ) {
					w += 10.f;
					refreshSize = true;
				} 

				if ( refreshSize ) {
					//printf( "Size: <%.2f, %.2f>\n", w, h ); 
					wi->SetSize( iVec2( w, h ) );
				}
				break;
			}
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

	Vec4 GetRectHandle( const ImVec2& center, Camera* camera )
	{ 
		//f32 r = 5.f * camera->GetOrthographicScale( );
		f32 r = 7.f;
		ImVec2 a = ImVec2( center.x - r / 2.f, center.y - r / 2.f );
		ImVec2 b = ImVec2( a.x + r, a.y + r );
		return Vec4( a.x, a.y, b.x, b.y );
	}


	bool EditorUIEditWindow::DrawRectHandle( const ImVec2& center, const u32& id, ImVec2* delta )
	{ 
		static ImVec2 onClickPos = ImVec2( 0.f, 0.f );
		static bool justClicked = false;
		static s32 selectedID = -1;
		Vec4 rh = GetRectHandle( center, &mCamera );
		ImVec2 a = ImVec2( rh.x, rh.y );
		ImVec2 b = ImVec2( rh.z, rh.w );
		bool isHovered = ImGui::IsMouseHoveringRect( a, b );
		bool isMouseDown = ImGui::IsMouseDown( 0 );
		bool isMouseClicked = ImGui::IsMouseClicked( 0 );
		bool isMouseReleased = ImGui::IsMouseReleased( 0 );
		bool isSelected = ( id == selectedID );
		ImVec2 mp = ImGui::GetMousePos(); 
		auto dl = ImGui::GetWindowDrawList( ); 

		// If not dragging an element. This is where it gets shitty.
		if ( isHovered && !justClicked && isMouseClicked )
		{
			onClickPos = mp;
			justClicked = true;
			selectedID = id;
			isSelected = true;
		} 

		// Drag
		if ( justClicked && isMouseDown && ((u32)selectedID == id) )
		{
			*delta = ImVec2( mp.x - onClickPos.x, mp.y - onClickPos.y ); 
			onClickPos = mp;
		}

		if ( isMouseReleased )
		{ 
			onClickPos = ImVec2( 0.f, 0.f );
			*delta = ImVec2( 0.f, 0.f );
			selectedID = -1;
			justClicked = false;
			isSelected = false;
		} 

		dl->AddRectFilled( a, b, isHovered ? ImColor( 1.0f, 0.2f, 0.1f, 1.f ) : isSelected ? ImColor( 1.f, 0.3f, 0.2f, 1.f ) : ImColor( 0.6f, 0.2f, 0.1f, 1.0f ) );
		return isSelected;
	}

	Vec4 TransformElement( UIElement* element, Camera* camera )
	{ 
		Vec2 ea = element->GetCalculatedLayoutPosition();
		Vec2 eb = element->GetCalculatedLayoutPosition() + element->GetCalculatedLayoutSize(); 

		Vec3 ra = camera->TransformPoint( Vec3( ea.x, ea.y / camera->GetAspectRatio(), 0.f ) );
		Vec3 rb = camera->TransformPoint( Vec3( eb.x, eb.y / camera->GetAspectRatio() , 0.f ) ); 

		return Vec4( ra.x, ra.y, rb.x, rb.y );
	}

	Vec4 GetTotalElementAreaWithRectHandles( UIElement* element, Camera* camera )
	{
		Vec4 dims = TransformElement( element, camera );
		Vec4 a = GetRectHandle( ImVec2( dims.x, dims.y ), camera );
		Vec4 b = GetRectHandle( ImVec2( dims.z, dims.w ), camera );
		return Vec4( a.x, a.y, b.z, b.w );
	}

	bool MouseHoveringElement( UIElement* element, Camera* camera )
	{ 
		Vec4 dims = GetTotalElementAreaWithRectHandles( element, camera );
		return (ImGui::IsMouseHoveringRect( ImVec2( dims.x, dims.y ), ImVec2( dims.z, dims.w ) ) );
	}

	bool EditorUIEditWindow::CanSelectThisElement( UIElement* element, Camera* camera )
	{
		if ( element == &mUI->mRoot )
		{
			return false;
		}

		// Look to see if ANY of the elements below this one are selectable. If not, then you good, playa
		bool canSelect = MouseHoveringElement( element, camera );

		// Recursively look at children, and if ANY of them are overlapping this one and the mouse is hovering them instead, then you can't select it
		bool selectChildren = false;
		for ( auto& c : element->mChildren )
		{ 
			selectChildren |= CanSelectThisElement( c, camera );
		}

		return (canSelect && !selectChildren);
	}

	bool HoveringAnyElementRecursive( UIElement* element, Camera* camera )
	{
		bool hovered = MouseHoveringElement( element, camera );
		for ( auto& c : element->mChildren )
		{
			hovered &= HoveringAnyElementRecursive( c, camera );
		}
		return hovered; 
	}

#define __TransformElementInternalByHandle( fa, fb, element, camera )\
	do {\
		f32 length = (fb - fa).Length();\
		Vec3 fp = Mat4x4::Inverse( camera.GetViewProjectionMatrix( ) ) * Vec3( (fa), 0.f );\
		Vec3 fp2 = Mat4x4::Inverse( camera.GetViewProjectionMatrix( ) ) * Vec3( (fb), 0.f );\
		if ( (fp2.x - fp.x) > 1.f && ((fp2.y - fp.y) * camera.GetAspectRatio()) > 1.f )\
		{\
			/*element->mPosition = Vec2( fp.x, fp.y * camera.GetAspectRatio() );\*/\
			element->SetSize( Vec2( (fp2.x - fp.x), (fp2.y - fp.y) * camera.GetAspectRatio() ) );\
		}\
		usedHandle = true;\
	} while ( 0 )


	void EditorUIEditWindow::DrawUIElementsRecursive( UIElement* element )
	{
		auto dl = ImGui::GetWindowDrawList();

		// Handle interacting with widget
		static ImVec2 onClickPos = ImVec2( 0.f, 0.f );
		static bool justClicked = false;
		static bool usingHandle = false;

		// Do root canvas
		Vec2 pos = element->GetCalculatedLayoutPosition();

		Vec2 ea = element->GetCalculatedLayoutPosition();
		Vec2 eb = element->GetCalculatedLayoutPosition() + element->GetCalculatedLayoutSize(); 

		Vec4 tdims = TransformElement( element, &mCamera );
		ImVec2 a = ImVec2( tdims.x, tdims.y );
		ImVec2 b = ImVec2( tdims.z, tdims.w );

		bool hovered = ImGui::IsMouseHoveringRect( a, b ); 
		bool active = hovered && ImGui::IsMouseDown( 0 );
		bool selected = ( mSelectedElement == element );
		bool mouseClicked = ImGui::IsMouseClicked( 0 );
		bool mouseDown = ImGui::IsMouseDown( 0 );
		ImVec2 mp = ImGui::GetMousePos();

		ImVec2 delta = ImVec2( 0.f, 0.f );

		// Scale widget
		bool usedHandle = false;
		if ( selected ) { 
			// Top Left
			if ( DrawRectHandle( a, 0, &delta ) ) { 
				__TransformElementInternalByHandle( Vec2( a.x + delta.x, a.y + delta.y ), Vec2( b.x, b.y ), element, mCamera );
			}

			// Top Right
			if ( DrawRectHandle( ImVec2( b.x, a.y ), 1, &delta ) ) { 
				__TransformElementInternalByHandle( Vec2( a.x, a.y + delta.y ), Vec2( b.x + delta.x, b.y ), element, mCamera );
			}

			// Right
			if ( DrawRectHandle( ImVec2( b.x, (a.y + b.y) / 2.f ), 2, &delta ) ) {
				__TransformElementInternalByHandle( Vec2( a.x, a.y ), Vec2( b.x + delta.x, b.y ), element, mCamera ); 
			}

			// Top
			if ( DrawRectHandle( ImVec2( (a.x + b.x) / 2.f, a.y ), 3, &delta ) ) {
				__TransformElementInternalByHandle( Vec2( a.x, a.y + delta.y ), Vec2( b.x, b.y ), element, mCamera );
			}

			// Bottom Left
			if ( DrawRectHandle( ImVec2( a.x, b.y ), 4, &delta ) ) {
				__TransformElementInternalByHandle( Vec2( a.x + delta.x, a.y ), Vec2( b.x, b.y + delta.y ), element, mCamera );
			}

			// Left
			if ( DrawRectHandle( ImVec2( a.x, (a.y + b.y) / 2.f ), 5, &delta ) ) {
				__TransformElementInternalByHandle( Vec2( a.x + delta.x, a.y ), Vec2( b.x, b.y ), element, mCamera );
			}

			// Bottom 
			if ( DrawRectHandle( ImVec2( (a.x + b.x) / 2.f, b.y ), 6, &delta ) ) {
				__TransformElementInternalByHandle( Vec2( a.x, a.y ), Vec2( b.x, b.y + delta.y ), element, mCamera );
			}

			// Bottom Right
			if ( DrawRectHandle( b, 7, &delta ) ) {									// BR		
				__TransformElementInternalByHandle( Vec2( a.x, a.y ), Vec2( b.x + delta.x, b.y + delta.y ), element, mCamera );
			} 
		} 

		if ( hovered && mouseClicked && CanSelectThisElement( element, &mCamera ) )
		{
			DeselectElement();
			SelectElement( element );
			onClickPos = mp;
			selected = true;
		} 

		if ( mouseClicked && !hovered && ImGui::IsMouseHoveringWindow() && selected && !usedHandle && !HoveringAnyElementRecursive( &mUI->mRoot, &mCamera ) )
		{
			DeselectElement();
			selected = false;
			onClickPos = mp;
		} 
		
		ImColor activeColor = ImColor( 0.9f, 0.4f, 0.1f, 1.f );
		ImColor hoveredColor = ImColor( 0.8f, 0.2f, 0.f, 1.f );
		ImColor color = ImColor( 1.f, 1.f, 1.f, 0.f );

		// Move element if selected
		if ( !mouseClicked && mouseDown && selected && !usedHandle && ImGui::IsMouseHoveringWindow() )
		{ 
			delta = ImVec2( mp.x - onClickPos.x, mp.y - onClickPos.y ); 
			onClickPos = mp;
		} 

		// Transform position of stuff 
		if ( !usedHandle && selected ) {
			Vec2 fa = Vec2( a.x, a.y );
			fa += Vec2( delta.x, delta.y );
			Vec3 fp = Mat4x4::Inverse( mCamera.GetViewProjectionMatrix( ) ) * Vec3( fa, 0.f ); 
			//element->mPosition = Vec2( fp.x, fp.y * mCamera.GetAspectRatio() ); 
		} 

		// Moving element via keyboard commands
		if ( selected )
		{
			Input* input = EngineSubsystem( Input );
			if ( input->IsKeyPressed( KeyCode::Up ) )
			{
				//element->mPosition.y -= 1.f;
			}
			if ( input->IsKeyPressed( KeyCode::Down ) )
			{
				//element->mPosition.y += 1.f;
			}
			if ( input->IsKeyPressed( KeyCode::Right ) )
			{
				//element->mPosition.x += 1.f;
			}
			if ( input->IsKeyPressed( KeyCode::Left ) )
			{
				//element->mPosition.x -= 1.f;
			} 
		}

		if ( selected ) {
			AddDashedLineRect( a, b, activeColor );
		}

		// Children
		for ( auto& c : element->mChildren )
		{
			DrawUIElementsRecursive( c );
		}
	} 

	//===============================================================================================

	void EditorUIEditWindow::AddDashedLine( const ImVec2& a, const ImVec2& b, const ImColor& color )
	{
		auto dl = ImGui::GetWindowDrawList();

		Vec2 diff = ( Vec2( b.x, b.y ) - Vec2( a.x, a.y ) );
		Vec2 norm = Vec2::Normalize( diff );
		f32 length = diff.Length();
		u32 numSteps = 12;
		f32 sa = 10.f;

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

	void EditorUIEditWindow::AddDashedLineRect( const ImVec2& a, const ImVec2& b, const ImColor& color )
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
		if ( !mCanvasInitialized )
		{
			mCanvasInitialized = true;
			mCamera = Camera( (u32)500, (u32)300 );
			mCamera.SetPosition( Vec3( 0.f ) );
			mCamera.SetProjectionType( ProjectionType::Orthographic );
			mCamera.SetOrthographicScale( 2.f );
			mCamera.SetAspectRatio( 1.f );
		} 

		auto windows = EngineSubsystem( WindowSubsystem )->GetWindows();
		for ( auto& w : windows )
		{
			if ( w->Class()->InstanceOf< EditorUICanvasWindow >() )
			{
				ImGui::ListBoxHeader( "##canvas", ImVec2( ImGui::GetWindowWidth() * 0.975f, ImGui::GetWindowHeight() * 0.975f ) );
				{
					Input* input = EngineSubsystem( Input );
					u32 sw = w->GetScreenWidth();
					u32 sh = w->GetScreenHeight();
					GraphicsSubsystemContext* ctx = w->GetWorld()->GetContext< GraphicsSubsystemContext >(); 
					mCamera.SetAspectRatio( (f32 )sh / (f32 )sw ); 
					//mCamera.SetAspectRatio( 1.f); 

					bool win_hov = ImGui::IsMouseHoveringWindow(); 
					
					ImVec2 delta = HandleMousePan(); 
					
					// Move camera
					mCamera.SetPosition( mCamera.GetPosition() - Vec3( delta.x, delta.y, 0.f ) ); 
					if ( win_hov && input->GetMouseWheel().y )
					{
						Vec3 to = Vec3( ImGui::GetMousePos().x, ImGui::GetMousePos().y, 0.f );
						ZoomOrthoCamera( &mCamera, to, input->GetMouseWheel().y / 10.f );
					} 

					auto dl = ImGui::GetWindowDrawList();
					ImTextureID img = (ImTextureID)Int2VoidP(ctx->GetFrameBuffer()->GetTexture());

					// Viewport dimensions transformed to camera space
					ImVec2 va = Vec2ToImVec2( mCamera.TransformPoint( Vec3( 0.f, 0.f, 0.f ) ) );
					ImVec2 vb = Vec2ToImVec2( mCamera.TransformPoint( Vec3( sw, sh ) ) ) ; 

					// Add border around viewport to denote where it be
					AddDashedLineRect( va, vb, ImColor( 1.f, 1.f, 1.f, 0.5f ) );

					// Add image from ui buffer
					dl->AddImage( img, va, vb, ImVec2( 0, 1 ), ImVec2( 1, 0 ) );

					DrawUIElementsRecursive( &mUI->mRoot ); 
				} 
				ImGui::ListBoxFooter();

				break;
			}
		} 
	}

	//=================================================================================

	void EditorUIEditWindow::ExplicitDestructor()
	{ 
		WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );
		for ( auto& w : ws->GetWindows() )
		{
			if ( w->Class()->InstanceOf< EditorUICanvasWindow >() )
			{ 
				ws->DestroyWindow( w->GetWindowID() );
			}
		}
	}

	//=================================================================================

	void EditorUIEditWindow::ElementOutliner( UIElement* element )
	{ 
		Utils::TempBuffer str = Utils::TransientBuffer( "%s##%zu", element->mID.c_str(), (u32)(usize)element );
		if ( ImGui::Selectable( str.buffer, ( element == mSelectedElement ) ) )
		{
			DeselectElement();
			SelectElement( element );
		}

		for ( auto& c : element->mChildren ) 
		{
			ElementOutliner( c );
		}
	}

	//=================================================================================

	void EditorUIEditWindow::ConstructScene()
	{ 
		GUIContext* guiContext = GetGUIContext( ); 

		// Add main menu options
		guiContext->RegisterMainMenu( "File" ); 

		World* world = GetWorld( ); 

			guiContext->RegisterWindow( "Canvas", [ & ] 
		{
			if ( ImGui::BeginDock( "Canvas" ) )
			{ 
				Canvas(); 
			}
			ImGui::EndDock( ); 
		}); 

		guiContext->RegisterWindow( "UI Palette", [ & ] ()
		{
			if ( ImGui::BeginDock( "UI Palette" ) )
			{
				ImGui::ListBoxHeader( "##ui_palette", ImVec2( ImGui::GetWindowWidth() - 20.f, ImGui::GetWindowHeight() - 15.f ) );
				{
					if ( mUI )
					{ 
						if ( ImGui::Selectable( "Button" ) )
						{
							UIElementButton* button = (UIElementButton*)mUI->mRoot.AddChild( new UIElementButton() );
							//button->SetPosition( )
							//button->mPosition = Vec2( 20.f, 20.f );
							button->SetSize( Vec2( 20.f, 10.f ) );
							button->mID = "Button";
						}

						if ( ImGui::Selectable( "Text Box" ) )
						{
							UIElementText* text = (UIElementText*)mUI->mRoot.AddChild( new UIElementText() );
							//text->mPosition = Vec2( 20.f, 20.f );
							text->SetSize( Vec2( 20.f, 10.f ) );
							text->mID = "Text";
							text->mText = "Text";
						} 

						if ( ImGui::Selectable( "Image" ) )
						{
							UIElementImage* image = (UIElementImage*)mUI->mRoot.AddChild( new UIElementImage() );
							//image->mPosition = Vec2( 20.f, 20.f );
							image->SetSize( Vec2( 20.f, 10.f ) );
							image->mID = "Image"; 
						}
					} 
				}
				ImGui::ListBoxFooter(); 
			}
			ImGui::EndDock( ); 
		}); 

		guiContext->RegisterWindow( "UI Properties", [ & ] ()
		{
			if ( ImGui::BeginDock( "UI Properties" ) )
			{
				ImGui::ListBoxHeader( "##elem_inspector", ImVec2( ImGui::GetWindowWidth() - 20.f, ImGui::GetWindowHeight() - 15.f ) );
				{
					if ( mUI )
					{ 
						ImGuiManager* igm = EngineSubsystem( ImGuiManager );
						igm->InspectObject( mUI.Get() ); 
						igm->InspectObject( &mUI->mRoot );
					} 
				}
				ImGui::ListBoxFooter();
			} 
			ImGui::EndDock( ); 
		});

		guiContext->RegisterWindow( "Inspector", [ & ]
		{
			if ( ImGui::BeginDock( "Inspector" ) )
			{
				ImGui::ListBoxHeader( "##elem_inspector", ImVec2( ImGui::GetWindowWidth() - 20.f, ImGui::GetWindowHeight() - 15.f ) );
				{
					if ( mUI )
					{ 
						ImGuiManager* igm = EngineSubsystem( ImGuiManager );
						if ( mSelectedElement )
						{
							igm->InspectObject( mSelectedElement ); 

							// Show inline styles
							Utils::TempBuffer tmp = Utils::TransientBuffer( "Inline Styles##%d", (usize)(intptr_t)mSelectedElement );
							if ( ImGui::CollapsingHeader( tmp.buffer ) )
							{ 
								igm->InspectObject( &mSelectedElement->mInlineStyles );
							}
						} 
					} 
				}
				ImGui::ListBoxFooter();
			}
			ImGui::EndDock( );
		});

		guiContext->RegisterWindow( "Outliner", [ & ] ()
		{
			if ( ImGui::BeginDock( "Outliner" ) )
			{
				ImGui::ListBoxHeader( "##elem_outliner", ImVec2( ImGui::GetWindowWidth() - 20.f, ImGui::GetWindowHeight() - 15.f ) );
				{
					if ( mUI )
					{ 
						ElementOutliner( &mUI->mRoot );
					} 
				}
				ImGui::ListBoxFooter();
			}
			ImGui::EndDock();
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

		static bool mShowStyles = false;
		auto stylesMenuOption = [&]()
		{
        	ImGui::MenuItem("Styles##options", NULL, &mShowStyles);
		};
	 	auto showStylesWindowFunc = [&]()
	 	{
			if (ImGui::BeginDock("Styles##options", &mShowStyles))
			{
				ImGui::ShowStyleEditor();	
			}
			ImGui::EndDock();
	 	}; 
 
		//guiContext->RegisterMenuOption("View", "Styles##Options", stylesMenuOption);
		//guiContext->RegisterWindow("Styles", showStylesWindowFunc); 

		// Register menu options
		guiContext->RegisterMenuOption( "File", "Save##save_asset_option", saveAssetOption ); 
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Canvas", nullptr, GUIDockSlotType::Slot_Tab, 1.0f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Inspector", "Canvas", GUIDockSlotType::Slot_Left, 0.25f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "UI Properties", "Inspector", GUIDockSlotType::Slot_Tab, 0.5f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "UI Palette", "Canvas", GUIDockSlotType::Slot_Right, 0.2f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Outliner", "UI Palette", GUIDockSlotType::Slot_Top, 0.6f ) );
		guiContext->SetActiveDock( "Inspector" );
		guiContext->Finalize( ); 
	}

	//=================================================================================

	void EditorUICanvasWindow::Init( const WindowParams& params )
	{
		// Construct scene in world
		if ( !mWorld )
		{
			// Initialize new world 
			mWorld = new World( );
			// Register contexts with world
			GraphicsSubsystemContext* gCtx = mWorld->RegisterContext< GraphicsSubsystemContext >( );
			gCtx->mWriteUIIntoFrameBuffer = true;
			gCtx->EnableRenderWorld( false );
			// Set asset from data 
			mUI = ( UI* )( params.mData );
			// Construct Scene 
			ConstructScene( ); 
		} 
	}

	//================================================================================= 

	void EditorUICanvasWindow::Update()
	{
		if ( !mInitialized )
		{
			SetViewport( GetViewport() );
			mInitialized = true;
			mWorld->GetContext< GraphicsSubsystemContext >()->GetFrameBuffer()->SetClearColor( ColorRGBA32( 1.f, 1.f, 1.f, 0.f ) );
		} 

		GUIContext* gCtx = GetGUIContext();
		gCtx->SetUIStyleConfig( mUI->mStyleConfig ); 
	}

	//================================================================================= 

#define CALL_FUNC( v, ... )\
	(v)( __VA_ARGS__ )

class Test
{
	public:
		static inline void DoThing( ) { 
			printf( "Did thing!\n" );
		}
};

	void EditorUICanvasWindow::ConstructScene()
	{
		GUIContext* guiContext = GetGUIContext( ); 

		// Add main menu options
		guiContext->RegisterMainMenu( "File" ); 

		World* world = GetWorld( ); 

		// Do need to set up rendering pipelines. This isn't going to do what I want.
		guiContext->RegisterWindow( "Canvas", [ & ] () { 

			f32 w = GetScreenWidth();
			f32 h = GetScreenHeight(); 

			if ( mUI )
			{
				mUI->mRoot.SetSize( Vec2( w, h ) );
				mUI->CalculateLayout( w, h );
				mUI->OnUI();
			} 
		}); 

		// Register menu options
		GUIContextParams ctxParams;
		ctxParams.mUseRootDock = false;
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Canvas", nullptr, GUIDockSlotType::Slot_Tab, 1.0f ) );
		guiContext->SetActiveDock( "Canvas" );
		guiContext->SetGUIContextParams( ctxParams );
		guiContext->Finalize( );
	}

	//================================================================================= 
} 

/*
	// Want a way to handle these concepts: 
		- Event binding
		- Property binding (two way, if possible)		- Need to assign specific objects to this, if this will occur. COuld do this via a UUID registration system, yet that would require EVERY serialized object to have a 16-byte UUID associated with it.

		Application setup: 

		Result MyApp::Initializion()
		{
			// Cloning instance of ui from asset manager
			AssetHandle< UI > ui = EngineSubsystem( AssetManager )->GetAsset< UI >( "myUI" ); 
			UI* uiInstance = ui->MakeInstance();	
			
			// Binding various functionality to specific
			uiInstance->BindEventCallback< UIElementButton, OnClickEvent >( "#specific_button_id", function );
			uiInstance->BindProperty( UIElementButton >( "#specific_button_id", mText, this, mProperty ); 
		}

		UI::BindEventCallback( uiInstance, OnClickEvent, "#specific_button_id", function );
		UI::BindProperty( uiInstance, "#specific_button_id", mText, this, mProperty );

		template < typename T >
		void UI::BindProperty( const String& id, const String& ePropName, const Object* obj, const String& oPropName )
		{
			// Iterate through tree
			UITreeIterator* iter = &mRoot;
			for ( auto& e : IterateTree( iter ) )
			{ 
				if ( e->mID.IsEqualTo( id ) )
				{
					const MetaClass* eCls = e->Class();
					const MetaClass* oCls = obj->Class();

					const MetaProperty* eProp = eCls->GetPropertyByName( ePropName );
					const MetaProperty* oProp = oCls->GetPropertyByName( oPropName );

					if ( eProp && oProp && eProp->GetType() == oProp->GetType() )
					{
						// Register signal and property in event manager to deal with this mess? Not sure how to handle this...
						// How to detect changes? Don't want to poll for this, of course...
						EventManager::BindProperties( eProp, e, oProp, o );
					} 
				}
			} 

			// The issue here is that object pointers are subject to change...and the event manager MUST unregister properties whenever the meta registry is changed
			void EventManager::SetUpBinding( const MetaProperty* propA, const Object* objA, const MetaProperty* propB, const Object* objB )
			{ 
				// One way binding
				auto bind_func = [&]() { 
					MetaClass::SetValue( objA, propA, objB, propB );
				};

				// Push this into a property binding? 

				// Unique key? 
			} 

			// Have to make sure that the two are the same property type? 
			// What happens when an object is deleted?
		}

		void UIElementButton::OnUI()
		{
			// Do stuff
			if ( ImGui::Button( ConstChar( mText ) ) )
			{ 
				// 
				EventManager::FireEvent( OnClickEvent
			}
		}

		void UIElementText::OnUI()
		{
			// Set up ...
			ImGui::Text( "%s", mText.c_str() );				--> Want a succinct, clean way to safely bind this property to some other object's property
		} 

		// Run this subsystem every frame and pump events out? 
		EventManager
		{
			void Update()
			{
				// Run through all bindings and update them? God, that sounds fucking awful...
				for ( auto& b : mBindings )
				{
					b();
				}
			}
		};

		// Something that can bind something with some stuff and things
		class Event : public Object
		{ 
		};

		// Yeah, but just changing values directly will not work...

		// Binding properties sets up a few things - 
			- The event manager must be notified to link the two data sources together via introspected object instances and their properties
			- The direction of the binding must be established ( one way or two way binding )
			- Events must be polled for changes to either data source
				- Pump out events to listeners on change whenever found - Can this be done automatically, however? I don't believe so...

			- There is such a TIGHT coupling between specific objects. This makes me very uncomfortable. It's an incredibly brittle system. 
			- Would need a way to easily and automatically remove bindings upon either object being deleted or changing location in memory

*/





























