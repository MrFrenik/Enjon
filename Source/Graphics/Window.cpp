#include "Graphics/Window.h"
#include "Utils/Errors.h"
#include "Math/Maths.h"
#include "Graphics/GraphicsSubsystem.h"
#include "ImGui/ImGuiManager.h"
#include "IO/InputManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"
#include "Base/World.h"
 
#ifdef ENJON_SYSTEM_WINDOWS
	#include <Windows.h>
#endif

#include <fmt/format.h>

namespace Enjon 
{ 
	// This ALL needs to be held within an object in the engine
	//SDL_GLContext Window::mGLContext = nullptr;
	//HashMap< CursorType, SDL_Cursor* > Window::mSDLCursors;
	//Vector< WindowParams > Window::mWindowsToInit;
	//Vector< Window* > Window::mWindowsToDestroy;
	//SDL_Surface* Window::mWindowIcon = nullptr; 

	void Window::InitInternal( const WindowParams& params )
	{
		m_screenWidth = params.mWidth;
		m_screenHeight = params.mHeight;
		WindowFlagsMask currentFlags = WindowFlagsMask( params.mFlags );

		Uint32 flags = SDL_WINDOW_OPENGL;

		if ( ( currentFlags & WindowFlagsMask( WindowFlags::INVISIBLE ) ) != 0 )
		{
			flags |= SDL_WINDOW_HIDDEN;
		}


		if ( ( currentFlags & WindowFlagsMask( WindowFlags::FULLSCREEN ) ) != 0 )
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			m_isfullscreen = true;
		}

		if ( ( currentFlags & WindowFlagsMask( WindowFlags::BORDERLESS ) ) != 0 )
		{
			flags |= SDL_WINDOW_BORDERLESS;
		}

		if ( ( currentFlags & WindowFlagsMask( WindowFlags::RESIZABLE ) ) != 0 )
		{
			flags |= SDL_WINDOW_RESIZABLE;
		}

		//Open an SDL window
		m_sdlWindow = SDL_CreateWindow( params.mName.c_str( ), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, params.mWidth, params.mHeight, flags );
		if ( m_sdlWindow == NULL ) {
			Utils::FatalError( "ENJON::WINDOW::CREATE::WINDOW_NOT_CREATED" );
		}

		//Set up our OpenGL context
		WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );
		ws->AttemptLoadGLContext( m_sdlWindow );

		//Set up glew (optional but recommended)
		GLenum error = glewInit( );
		if ( error != GLEW_OK ) {
			Utils::FatalError( "ENJON::WINDOW::CREATE::GLEW_NOT_INITIALIZED" );
		}

		//Set glewExperimental to true
		glewExperimental = GL_TRUE;

		//Set Vsync to enabled
		SDL_GL_SetSwapInterval( 0 );

		// Construct gui context
		mGUIContext = GUIContext( this );

		// Initialize ImGUI context and set
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		mGUIContext.mContext = igm->Init( this );

		// Set window icon
		SDL_Surface* icon = ws->AttemptLoadWindowIcon( );
		if ( icon )
		{
			SDL_SetWindowIcon( m_sdlWindow, icon );
		} 

		this->Init( params );
	} 

	//=================================================================================================================

	void Window::Update( )
	{ 
	}

	//=================================================================================================================
 
	void Window::MakeCurrent( )
	{
		WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );
		// Set current window in SDL
		SDL_GL_MakeCurrent( m_sdlWindow, ws->GetGLContext() );

		// Set current ImGuiContext 
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		igm->SetContextByWindow( this ); 
	} 

	void Window::ClearDroppedFiles( )
	{
		mDroppedFiles.clear( );
		mNeedToClearDroppedFiles = false;
	}

	const HashSet<String>& Window::GetDroppedFiles( )
	{
		mNeedToClearDroppedFiles = true;
		return mDroppedFiles;
	}

	void Window::SetViewport( const s32& width, const s32& height )
	{
		m_screenWidth = width;
		m_screenHeight = height;	
	}

	void Window::SetViewport( const iVec2& dimensions )
	{
		// TODO(John): Need to refresh screen here...
		m_screenWidth = dimensions.x;
		m_screenHeight = dimensions.y;

		if ( mWorld )
		{
			GraphicsSubsystemContext* ctx = mWorld->GetContext< GraphicsSubsystemContext >( );
			ctx->ReinitializeFrameBuffers( GetViewport( ) );
		}
	}

	void Window::MaximizeWindow( )
	{
		SDL_MaximizeWindow( m_sdlWindow );
	} 

	void Window::SetSize( const iVec2& dims )
	{
		SDL_SetWindowSize( m_sdlWindow, dims.x, dims.y ); 
		SetViewport( dims );
	}

	void Window::SetPosition( const iVec2& position )
	{
		SDL_SetWindowPosition( m_sdlWindow, position.x, position.y );
	}

	void Window::SetPosition( const u32& x, const u32& y )
	{
		SDL_SetWindowPosition( m_sdlWindow, x, y );
	}

	iVec2 Window::GetViewport() const 
	{
		return iVec2(m_screenWidth, m_screenHeight);
	}

	void Window::SwapBuffer()
	{
		SDL_GL_SwapWindow(m_sdlWindow);
	}
	
	void Window::Clear(float val, GLbitfield mask, const ColorRGBA32& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glClearDepth(val);
		glClear(mask); 
	}

	void Window::SetWindowTitle( const String& title )
	{
		SDL_SetWindowTitle( m_sdlWindow, title.c_str( ) );
	}

	void Window::SetWindowFullScreen( const s32& screenWidth, const s32& screenHeight )
	{
		//Need to figure this one out...
	}

	//==================================================================================================

	void Window::HideWindow( )
	{
		SDL_HideWindow( m_sdlWindow );
	}

	//==================================================================================================

	void Window::ShowWindow( )
	{
		SDL_ShowWindow( m_sdlWindow );
	}

	//==================================================================================================

	void Window::PrintDebugInfo( )
	{
#ifdef ENJON_SYSTEM_WINDOWS
		s32 x, y, sizeX, sizeY;
		SDL_GetWindowPosition( m_sdlWindow, &x, &y ); 
		SDL_GetWindowSize( m_sdlWindow, &sizeX, &sizeY );
		Vec2 mouseCoords = EngineSubsystem( Input )->GetMouseCoords( ); 
		POINT point;
		GetCursorPos( &point ); 
		std::cout << fmt::format( "WindowMin: <{}, {}>, WindowMax: <{}, {}>, MouseCoords: <{}, {}>, Contains: {}\n", x, y, x + sizeX, y + sizeY, point.x, point.y, IsMouseInWindow( ) );
#endif
	}

	bool Window::IsMouseInWindow( )
	{
		s32 x, y, sizeX, sizeY;
		SDL_GetWindowPosition( m_sdlWindow, &x, &y ); 
		SDL_GetWindowSize( m_sdlWindow, &sizeX, &sizeY );

		// Define rect
		Rect rect( x, y, sizeX, sizeY );

		Vec2 mouseCoords;

#ifdef ENJON_SYSTEM_WINDOWS
	POINT point;
	GetCursorPos( &point ); 
	mouseCoords = Vec2( point.x, point.y );
#endif
 
		return ( rect.Contains( mouseCoords, RectContainType::Exclusive ) ); 
	}

	Result Window::ProcessInput( const SDL_Event& event )
	{ 
		GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );

		// Clear all dropped files from previous frame
		if ( mNeedToClearDroppedFiles )
		{
			ClearDroppedFiles( );
		} 

		switch ( event.type )
		{
			case SDL_DROPFILE:
			{ 
				if ( mMouseIsHovering )
				{
					// Push back new dropped file
					mDroppedFiles.insert( String(event.drop.file) ); 

					// Just released, so need to explicitly set button state 
					EngineSubsystem( Input )->SetButtonState( KeyCode::LeftMouseButton, false, true ); 

					// Free the filename memory
					SDL_free( event.drop.file ); 
				}
			} break;

			case SDL_WINDOWEVENT: 
			{
				switch ( event.window.event )
				{
					case SDL_WINDOWEVENT_RESIZED: 
					{
						// For now only reinitialize the frame buffers for main window
						if ( event.window.windowID == SDL_GetWindowID( EngineSubsystem( GraphicsSubsystem )->GetMainWindow()->GetSDLWindow() ) )
						{
							gs->ReinitializeFrameBuffers( );
						}

						// Resize viewport
						SetViewport( iVec2( (u32)event.window.data1, (u32)event.window.data2 ) ); 
					}
					break; 

					case SDL_WINDOWEVENT_ENTER: 
					{
						if ( event.window.windowID == SDL_GetWindowID( m_sdlWindow ) )
						{
							mMouseIsHovering = true;
						}
					}
					break;
					
					case SDL_WINDOWEVENT_LEAVE: 
					{
						if ( event.window.windowID == SDL_GetWindowID( m_sdlWindow ) )
						{
							mMouseIsHovering = false;
						}
					}
					break;

					case SDL_WINDOWEVENT_MOVED:
					{
					}
					break;

					case SDL_WINDOWEVENT_RESTORED:
					{ 
					} break;

					case SDL_WINDOWEVENT_FOCUS_GAINED: 
					{
					} break;

					case SDL_WINDOWEVENT_FOCUS_LOST: 
					{
					} break;
					
					case SDL_WINDOWEVENT_CLOSE: 
					{ 
						if ( event.window.windowID == SDL_GetWindowID( m_sdlWindow ) )
						{
							// Need to push back into static window for destruction at top of frame
							WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );
							ws->DestroyWindow( this );
							//return Result::FAILURE;
						}
					} break; 
				} 
			} break; 

			default:
			{ 
			} break;
		} 

		return Result::PROCESS_RUNNING;
	} 

	//==============================================================================

	Vector< Window* > Window::Destroy( )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );

		mDroppedFiles.clear( ); 

		// Cache main window
		Window* mainWindow = gfx->GetMainWindow( );

		// Remove from graphics subsystem
		gfx->RemoveWindow( this ); 

		// Destroy SDL window
		SDL_DestroyWindow( m_sdlWindow );

		Vector< Window* > windowsToPostDestroy;

		// Need to check for main window to destroy everything
		if ( mainWindow == this )
		{ 
			for ( auto& w : gfx->GetWindows( ) )
			{
				windowsToPostDestroy.push_back( w );
			}
		}

		// Delete world
		if ( mWorld )
		{
			delete mWorld;
			mWorld = nullptr;
		}

		this->ExplicitDestroy( );

		return windowsToPostDestroy;
	}

	//==============================================================================

	void Window::SetWindowCursor( CursorType type )
	{
		// Error checks are for pussies.
		WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );
		SDL_SetCursor( ws->GetCursor(type) );
	}

	//==============================================================================

	void Window::SetFocus( )
	{
		SDL_RaiseWindow( m_sdlWindow ); 
	}

	//==============================================================================

	bool Window::IsFocused( )
	{
		return ( m_sdlWindow == SDL_GetMouseFocus( ) );
	} 

	//==============================================================================

	void Window::SetWorld( World* world )
	{
		mWorld = world;
	}

	World* Window::GetWorld( )
	{
		return mWorld;
	}


	//==============================================================================================

	Result WindowSubsystem::Initialize( )
	{ 
		AttemptLoadWindowIcon( );
		InitSDLCursors( );
		return Result::SUCCESS;
	}

	//==============================================================================================

	void WindowSubsystem::Update( const f32 dT )
	{ 
		// Cleanup all previous windows that need destruction
		CleanupWindows( );
		// Initialize any new windows
		InitializeWindows( ); 
	}

	//==============================================================================================
	
	Result WindowSubsystem::Shutdown( )
	{ 
		// Release things
		return Result::SUCCESS;
	}

	//============================================================================================== 

	void WindowSubsystem::AttemptLoadGLContext( SDL_Window* window )
	{ 
		if (mGLContext == nullptr )
		{
			mGLContext = SDL_GL_CreateContext( window );
			if (mGLContext == nullptr) {
				Utils::FatalError("ENJON::WINDOW::CREATE::SD_GL_CONTEXT_NOT_CREATED");
			} 
		}
		
	}

	//============================================================================================== 

	SDL_Surface* WindowSubsystem::AttemptLoadWindowIcon( )
	{
		if ( !mWindowIcon )
		{
			mWindowIcon = SDL_LoadBMP( ( Engine::GetInstance( )->GetConfig( ).GetEngineResourcePath( ) + "/Textures/enjonicon_32x32.bmp" ).c_str( ) );
		} 
		return mWindowIcon;
	}

	//============================================================================================== 

	void WindowSubsystem::InitSDLCursors( )
	{ 
		mSDLCursors[ CursorType::Arrow ]	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_ARROW );
		mSDLCursors[ CursorType::IBeam ]	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_IBEAM );
		mSDLCursors[ CursorType::SizeAll ]	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZEALL );
		mSDLCursors[ CursorType::SizeNESW ]	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZENESW );
		mSDLCursors[ CursorType::SizeNWSE ]	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZENWSE );
		mSDLCursors[ CursorType::SizeWE ]	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZEWE );
		mSDLCursors[ CursorType::SizeNS ]	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZENS );
		mSDLCursors[ CursorType::Hand ]		= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_HAND );
	}

	//============================================================================================== 

	void WindowSubsystem::InitializeWindows( )
	{
		// Initialize new windows
		for ( auto& wp : mWindowsToInit )
		{
			// TODO(John): Assert that the given class is derived from Window
			const MetaClass* cls = wp.mWindowClass;
			if ( !cls ) 
			{ 
				continue;
			}

			// ...Otherwise this will fail
			Window* window = (Window*)cls->Construct();
			if ( !window ) 
			{
				continue;
			}
			// Initialize window
			window->InitInternal( wp );
			// Late initialize gui context
			window->GetGUIContext( )->LateInit( );
			// Resize viewport
			window->SetViewport( iVec2( wp.mWidth, wp.mHeight ) );
			// Add window id
			window->mID = wp.id;
			// Add to id map
			mWindowIDMap[ wp.id ] = window;

			mWindows.push_back( window );
		}

		// Clear window set
		mWindowsToInit.clear( ); 
	}

	// TODO(): This is crashing currently when existing the program
	void WindowSubsystem::DestroyAll( )
	{ 
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		ImGuiManager* igm = EngineSubsystem( ImGuiManager ); 
		Vector< Window* > postDestroy;

		// Destroy all windows
		for ( auto& w : mWindows )
		{
			if ( w )
			{ 
				for ( auto& win : w->Destroy( ) )
				{
					postDestroy.push_back( win );
				}

				mWindowIDMap.erase( w->mID );
				igm->RemoveWindowFromContextMap( w->m_sdlWindow );
				delete w;
				w = nullptr;
			}
		} 

		mWindows.clear( );
	}

	void WindowSubsystem::CleanupWindows( b32 destroyAll )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		Vector< Window* > postDestroy;

		if ( destroyAll )
		{
			DestroyAll( );
		}
		else
		{
			for ( auto& id : mWindowsToDestroy )
			{
				Window* w = mWindowIDMap.find( id )->second;
				if ( w )
				{
					// Remove from windows list
					auto it = std::remove( mWindows.begin( ), mWindows.end( ), w );
					mWindows.erase( it );

					for ( auto& win : w->Destroy( ) )
					{
						postDestroy.push_back( win );
					}

					mWindowIDMap.erase( id ); 
					igm->RemoveWindowFromContextMap( w->m_sdlWindow ); 
					delete w;
					w = nullptr; 
				}
			} 
		} 

		// Clear windows
		mWindowsToDestroy.clear( );

		for ( auto& w : postDestroy )
		{
			if ( w )
			{
				w->Destroy( );
				delete w;
				w = nullptr;
			}
		} 
	}

	SDL_Cursor* WindowSubsystem::GetCursor( CursorType type )
	{
		return mSDLCursors[ type ];
	} 

	s32 WindowSubsystem::AddNewWindow( WindowParams params )
	{
		params.id = mFreeWindowID++;
		mWindowsToInit.push_back( params ); 
		return params.id;
	}

	void WindowSubsystem::DestroyWindow( Window* window )
	{ 
		mWindowsToDestroy.push_back( window->mID );
	}

	// Want to destroy a window by a given id
	void WindowSubsystem::DestroyWindow( const u32& id )
	{ 
		mWindowsToDestroy.push_back( id );
	}
 
	void WindowSubsystem::WindowsUpdate( )
	{
		// Cleanup all previous windows that need destruction
		CleanupWindows( );
		// Initialize any new windows
		InitializeWindows( ); 
	}

	//==============================================================================================

	b32 WindowSubsystem::AnyWindowHovered( )
	{
		for ( auto& w : EngineSubsystem( GraphicsSubsystem )->GetWindows( ) )
		{
			if ( w->IsMouseInWindow( ) )
			{
				return true;
			}
		}

		return false;
	}

	//==============================================================================================

	u32 WindowSubsystem::NumberOfHoveredWindows( )
	{
		u32 count = 0;
		for ( auto& w : EngineSubsystem( GraphicsSubsystem )->GetWindows( ) ) 
		{
			if ( w->IsMouseInWindow( ) )
			{
				count++;
			}
		}

		return count;
	}

	//==============================================================================================

	Vec2 WindowSubsystem::GetDisplaySize( const u32& displayIndex )
	{ 
		if ( displayIndex > SDL_GetNumVideoDisplays( ) )
		{
			return Vec2( -1.f, -1.f );
		}
		SDL_DisplayMode curr;
		s32 shouldBeZero = SDL_GetCurrentDisplayMode( displayIndex, &curr );
		return Vec2( curr.w, curr.h );
	}

	SDL_GLContext WindowSubsystem::GetGLContext( )
	{
		return mGLContext;
	} 

	Vector< Window* > WindowSubsystem::GetWindows( ) 
	{
		return mWindows;
	}

	Window* WindowSubsystem::GetWindow( const u32& id )
	{
		if ( mWindowIDMap.find( id ) != mWindowIDMap.end( ) )
		{
			return mWindowIDMap[ id ];
		}
		return nullptr;
	}

	void WindowSubsystem::ForceInitWindows( )
	{
		InitializeWindows( );
	}

	void WindowSubsystem::ForceCleanupWindows( )
	{
		CleanupWindows( );
	}
}