#include "EditorUIEditWindow.h"
#include "EditorMaterialEditWindow.h"
#include "EditorApp.h"

#include <Engine.h>
#include <Graphics/GraphicsSubsystem.h>
#include <Graphics/FrameBuffer.h>
#include <Utils/FileUtils.h>
#include <IO/InputManager.h>

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

			// Construct custom render pass for context 
			//mUIPass = new UIRenderPass( mUI, this ); 
			//gCtx->AddCustomPass( mUIPass );

			// Don't need to render the world with this context ( this will be abstracted away eventually into render pipelines )
			gCtx->EnableRenderWorld( false );

			// Construct Scene 
			ConstructScene( ); 
		} 
	}

	//=================================================================================

	void EditorUIEditWindow::Update()
	{ 
		//if ( !mInitialized )
		//{ 
		//	//// Canvas window
		//	//WindowParams canvasParams;
		//	//canvasParams.mWidth = 800;
		//	//canvasParams.mWidth = 600;
		//	//canvasParams.mMetaClassFunc = [ & ] () -> const MetaClass * { return Object::GetClass < EditorUICanvasWindow >(); };
		//	//canvasParams.mName = "UI Canvas";
		//	//mCanvasWindowID = (s32)EngineSubsystem( WindowSubsystem )->AddNewWindow( canvasParams );
		//	////EngineSubsystem( WindowSubsystem )->ForceInitWindows(); 
		//	//mInitialized = true;
		//} 
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
		f32 r = 10.f / camera->GetOrthographicScale( );
		ImVec2 a = ImVec2( center.x - r / 2.f, center.y - r / 2.f );
		ImVec2 b = ImVec2( a.x + r, a.y + r );
		return Vec4( a.x, a.y, b.x, b.y );
	}


	bool EditorUIEditWindow::DrawRectHandle( const ImVec2& center, const u32& id, ImVec2* delta )
	{ 
		static ImVec2 onClickPos = ImVec2( 0.f, 0.f );
		static bool justClicked = false;
		static s32 selectedID = -1;
		f32 r = 10.f / mCamera.GetOrthographicScale( );
		ImVec2 a = ImVec2( center.x - r / 2.f, center.y - r / 2.f );
		ImVec2 b = ImVec2( a.x + r, a.y + r );
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

		dl->AddRectFilled( a, b, isHovered ? ImColor( 0.9f, 0.2f, 0.1f, 1.f ) : isSelected ? ImColor( 1.f, 0.3f, 0.2f, 1.f ) : ImColor( 1.f, 1.f, 1.f, 0.1f ) );
		return isSelected;
	}

	Vec4 TransformElement( UIElement* element, Camera* camera )
	{ 
		Vec2 ea = element->mPosition;
		Vec2 eb = element->mPosition + element->mSize; 

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

	bool CanSelectThisElement( UIElement* element, Camera* camera )
	{
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

	void EditorUIEditWindow::DrawUIElementsRecursive( UIElement* element )
	{
		auto dl = ImGui::GetWindowDrawList();

		// Handle interacting with widget
		static ImVec2 onClickPos = ImVec2( 0.f, 0.f );
		static bool justClicked = false;
		static bool usingHandle = false;

		// Do root canvas
		Vec2 pos = element->mPosition;

		Vec2 ea = element->mPosition;
		Vec2 eb = element->mPosition + element->mSize; 

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
			if ( DrawRectHandle( a, 0, &delta ) ) { 
				delta = ImVec2( delta.x * mCamera.GetOrthographicScale(), delta.y * mCamera.GetOrthographicScale() );
				element->mSize -= Vec2( delta.x, delta.y );	// TL 
				element->mPosition += Vec2( delta.x, delta.y );
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( b.x, a.y ), 1, &delta ) ) { 
				element->mSize += Vec2( delta.x, -delta.y );	// TR
				element->mPosition += Vec2( 0.f, delta.y );	// TR
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( b.x, (a.y + b.y) / 2.f ), 2, &delta ) ) {
				element->mSize += Vec2( delta.x, 0.f );
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( (a.x + b.x) / 2.f, a.y ), 3, &delta ) ) {
				element->mSize += Vec2( 0.f, -delta.y ); // TR
				element->mPosition += Vec2( 0.f, delta.y );
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( a.x, b.y ), 4, &delta ) ) {				// BL
				element->mSize -= Vec2( delta.x, -delta.y );
				element->mPosition += Vec2( delta.x, 0.f );
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( a.x, (a.y + b.y) / 2.f ), 5, &delta ) ) {
				element->mSize += Vec2( -delta.x, 0.f ); // BL
				element->mPosition += Vec2( delta.x, 0.f );
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( (a.x + b.x) / 2.f, b.y ), 6, &delta ) ) {
				element->mSize += Vec2( 0.f, delta.y ); // BL
				usedHandle = true;
			}

			if ( DrawRectHandle( b, 7, &delta ) ) {									// BR		
				element->mSize += Vec2( delta.x, delta.y );
				usedHandle = true;
			} 
		} 

		if ( hovered && mouseClicked && CanSelectThisElement( element, &mCamera ) )
		{
			DeselectElement();
			SelectElement( element );
			onClickPos = mp;
			selected = true;
		} 

		//if ( mouseClicked && !hovered && ImGui::IsMouseHoveringWindow() && selected && !usedHandle && !HoveringAnyElementRecursive( &mUI->mRoot, &mCamera ) )
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
			//Vec3 fa = ra;
			//fa += Vec3( delta.x, delta.y, 0.f );
			//Vec3 fp = Mat4x4::Inverse( mCamera.GetViewProjectionMatrix( ) ) * fa; 
			//element->mPosition = Vec2( fp.x, fp.y ); 
			element->mPosition += Vec2( delta.x, delta.y ); 
		} 

		//if ( active || selected || hovered ) {
			dl->AddRect( a, b, active || selected ? activeColor : hovered ? hoveredColor : color ); 
		//}

		// Children
		for ( auto& c : element->mChildren )
		{
			DrawUIElementsRecursive( c );
		}
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

		ImVec2 delta = ImVec2( 0.f, 0.f );

		if ( hovered && mouseClicked )
		{
			DeselectElement();
			SelectElement( &mUI->mRoot );
			onClickPos = mp;
		} 

		// Scale widget
		bool usedHandle = false;
		if ( selected ) { 

			if ( DrawRectHandle( a, 0, &delta ) ) { 
				delta = ImVec2( delta.x * mCamera.GetOrthographicScale(), delta.y * mCamera.GetOrthographicScale() );
				mUI->mRoot.mSize -= Vec2( delta.x, delta.y );	// TL 
				mUI->mRoot.mPosition += Vec2( delta.x, delta.y );
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( b.x, a.y ), 1, &delta ) ) { 
				mUI->mRoot.mSize += Vec2( delta.x, -delta.y );	// TR
				mUI->mRoot.mPosition += Vec2( 0.f, delta.y );	// TR
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( b.x, (a.y + b.y) / 2.f ), 2, &delta ) ) {
				mUI->mRoot.mSize += Vec2( delta.x, 0.f );
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( (a.x + b.x) / 2.f, a.y ), 3, &delta ) ) {
				mUI->mRoot.mSize += Vec2( 0.f, -delta.y ); // TR
				mUI->mRoot.mPosition += Vec2( 0.f, delta.y );
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( a.x, b.y ), 4, &delta ) ) {				// BL
				mUI->mRoot.mSize -= Vec2( delta.x, -delta.y );
				mUI->mRoot.mPosition += Vec2( delta.x, 0.f );
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( a.x, (a.y + b.y) / 2.f ), 5, &delta ) ) {
				mUI->mRoot.mSize += Vec2( -delta.x, 0.f ); // BL
				mUI->mRoot.mPosition += Vec2( delta.x, 0.f );
				usedHandle = true;
			}

			if ( DrawRectHandle( ImVec2( (a.x + b.x) / 2.f, b.y ), 6, &delta ) ) {
				mUI->mRoot.mSize += Vec2( 0.f, delta.y ); // BL
				usedHandle = true;
			}

			if ( DrawRectHandle( b, 7, &delta ) ) {									// BR		
				mUI->mRoot.mSize += Vec2( delta.x, delta.y );
				usedHandle = true;
			} 
		} 

		if ( mouseClicked && !hovered && ImGui::IsMouseHoveringWindow() && selected && !usedHandle )
		{
			DeselectElement();
			selected = false;
		} 
		
		ImColor activeColor = ImColor( 0.9f, 0.4f, 0.1f, 1.f );
		ImColor hoveredColor = ImColor( 0.8f, 0.2f, 0.f, 1.f );
		ImColor color = ImColor( 1.f, 1.f, 1.f, 1.f );

		// Move element if selected
		if ( mouseDown && selected && !usedHandle )
		{ 
			delta = ImVec2( mp.x - onClickPos.x, mp.y - onClickPos.y ); 
			onClickPos = mp;
		} 

		// Transform position of stuff 
		//ra = mCamera.TransformPoint( Vec3( mUI->mRoot.mPosition, 0.f ) );
		//rb = mCamera.TransformPoint( Vec3( mUI->mRoot.mPosition, 0.f ) + Vec3( mUI->mRoot.mSize, 0.f ) ); 
		//a = ImVec2( ra.x, ra.y );
		//b = ImVec2( rb.x, rb.y );
		if ( !usedHandle ) {
			Vec3 fa = ra;
			fa += Vec3( delta.x, delta.y, 0.f );
			Vec3 fp = Mat4x4::Inverse( mCamera.GetViewProjectionMatrix( ) ) * fa; 
			mUI->mRoot.mPosition = Vec2( fp.x, fp.y ); 

			//ra = mCamera.TransformPoint( Vec3( mUI->mRoot.mPosition, 0.f ) );
			//rb = mCamera.TransformPoint( Vec3( mUI->mRoot.mPosition, 0.f ) + Vec3( mUI->mRoot.mSize, 0.f ) ); 
			//a = ImVec2( ra.x, ra.y );
			//b = ImVec2( rb.x, rb.y ); 
		}

		dl->AddRect( a, b, hovered ? hoveredColor : active || selected ? activeColor : color ); 
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
		if ( !mCanvasInitialized )
		{
			Vec2 vs = Vec2( 900.f, 506.f );		// By default, it's the size of the Sandbox's window
			mViewportTransform = Transform( Vec3( 0.f ), Quaternion(), Vec3( vs.x, vs.y, 1.f ) );
			ImVec2 co = ImVec2( ImGui::GetWindowPos().x + ( ImGui::GetWindowWidth() - vs.x ) / 2.f , ImGui::GetWindowPos().y + ( ImGui::GetWindowHeight() - vs.y ) / 2.f );
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
				ImGui::ListBoxHeader( "##canvas", ImVec2( ImGui::GetWindowWidth() * 0.97f, ImGui::GetWindowHeight() * 0.95f ) );
				{
					u32 sw = w->GetScreenWidth();
					u32 sh = w->GetScreenHeight();
					GraphicsSubsystemContext* ctx = w->GetWorld()->GetContext< GraphicsSubsystemContext >(); 

					mCamera.SetAspectRatio( (f32 )sh / (f32 )sw );

					bool win_hov = ImGui::IsMouseHoveringWindow(); 

					if ( win_hov && ImGui::IsMouseClicked( 0 ) )
					{ 
						Vec2 ea = mUI->mRoot.mPosition;
						Vec2 eb = mUI->mRoot.mPosition + mUI->mRoot.mSize; 
					}

					Input* input = EngineSubsystem( Input );
					
					ImVec2 delta = HandleMousePan(); 

					mCamera.SetPosition( mCamera.GetPosition() - Vec3( delta.x, delta.y, 0.f ) );

					if ( win_hov && input->GetMouseWheel().y )
					{
						Vec3 to = Vec3( ImGui::GetMousePos().x, ImGui::GetMousePos().y, 0.f );
						ZoomOrthoCamera( &mCamera, to, input->GetMouseWheel().y / 10.f );
					} 

					auto dl = ImGui::GetWindowDrawList();
					ImTextureID img = (ImTextureID)Int2VoidP(ctx->GetFrameBuffer()->GetTexture());

					ImVec2 ia = Vec2ToImVec2( mCamera.TransformPoint( Vec3( 0.f, 0.f, 0.f ) ) );
					ImVec2 ib = Vec2ToImVec2( mCamera.TransformPoint( Vec3( sw, sh ) ) ) ;

					// Add border around viewport to denote where it be
					AddDashedLineRect( ia, ib, ImColor( 1.f, 1.f, 1.f, 0.5f ) );

					// Add image from ui buffer
					dl->AddImage( img, ia, ib, ImVec2( 0, 1 ), ImVec2( 1, 0 ) );

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
				ws->ForceCleanupWindows();
			}
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
							UIElementButton* button = (UIElementButton*)mUI->mRoot.AddChild( new UIElementButton() );
							button->mPosition = mUI->mRoot.mPosition + Vec2( 20.f, 20.f );
							button->mSize = Vec2( 20.f, 10.f );
							button->mLabel = "Button";
						}

						ImGuiManager* igm = EngineSubsystem( ImGuiManager );
						if ( mSelectedElement )
						{
							igm->InspectObject( mSelectedElement ); 
						} 

						//igm->InspectObject( mUI->mRoot.mChildren.at( 0 ) ); 
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
 
		guiContext->RegisterMenuOption("View", "Styles##Options", stylesMenuOption);
		guiContext->RegisterWindow("Styles", showStylesWindowFunc); 

		// Register menu options
		guiContext->RegisterMenuOption( "File", "Save##save_asset_option", saveAssetOption ); 
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Canvas", nullptr, GUIDockSlotType::Slot_Tab, 1.0f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Properties", "Canvas", GUIDockSlotType::Slot_Left, 0.25f ) );
		guiContext->SetActiveDock( "Canvas" );
		guiContext->Finalize( ); 
	}

	//=================================================================================

	void EditorUICanvasWindow::Init( const WindowParams& params )
	{
		// Construct scene in world
		if ( !mInitialized )
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

			//mInitialized = true;
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
	}

	//================================================================================= 

	void EditorUICanvasWindow::ConstructScene()
	{
		GUIContext* guiContext = GetGUIContext( ); 

		// Add main menu options
		guiContext->RegisterMainMenu( "File" ); 

		World* world = GetWorld( ); 

		// Do need to set up rendering pipelines. This isn't going to do what I want.
		guiContext->RegisterWindow( "Canvas", [ & ] () {

			if ( mUI )
			{
				mUI->OnUI();
			} 
		}); 

		auto saveAssetOption = [ & ] ( )
		{
			if ( ImGui::MenuItem( "Save##save_asset_option", NULL ) )
			{
			}
		}; 

		// Register menu options
		GUIContextParams ctxParams;
		ctxParams.mUseRootDock = false;
		guiContext->RegisterMenuOption( "File", "Save##save_asset_option", saveAssetOption ); 
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Canvas", nullptr, GUIDockSlotType::Slot_Tab, 1.0f ) );
		guiContext->SetActiveDock( "Canvas" );
		guiContext->SetGUIContextParams( ctxParams );
		guiContext->Finalize( );
	}

	//================================================================================= 
}
