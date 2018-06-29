#include "Graphics/Window.h"
#include "Utils/Errors.h"
#include "Math/Maths.h"
#include "Graphics/GraphicsSubsystem.h"
#include "ImGui/ImGuiManager.h"
#include "IO/InputManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"

namespace Enjon {

	SDL_GLContext Window::mGLContext = nullptr;
	HashMap< CursorType, SDL_Cursor* > Window::mSDLCursors;

	Window::Window()
		: m_isfullscreen(false)
	{
	} 

	Window::~Window()
	{
		mDroppedFiles.clear( );
	}

	int Window::Init(std::string windowName, int screenWidth, int screenHeight, WindowFlagsMask currentFlags) 
	{
		m_screenWidth = screenWidth;
		m_screenHeight = screenHeight;

		Uint32 flags = SDL_WINDOW_OPENGL;
		
		
		if((currentFlags & WindowFlagsMask(WindowFlags::INVISIBLE)) != 0)
		{
			flags |= SDL_WINDOW_HIDDEN;
		}
		
		
		if((currentFlags & WindowFlagsMask(WindowFlags::FULLSCREEN)) != 0)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			m_isfullscreen = true;
		}


		if((currentFlags & WindowFlagsMask(WindowFlags::BORDERLESS)) != 0)
		{
			flags |= SDL_WINDOW_BORDERLESS;
		}

		if((currentFlags & WindowFlagsMask(WindowFlags::RESIZABLE)) != 0)
		{
			flags |= SDL_WINDOW_RESIZABLE;
		}

		//Open an SDL window
		m_sdlWindow = SDL_CreateWindow( windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, flags );
		if (m_sdlWindow == NULL) {
			Utils::FatalError("ENJON::WINDOW::CREATE::WINDOW_NOT_CREATED");
		}

		//Set up our OpenGL context
		if ( !mGLContext )
		{
			mGLContext = SDL_GL_CreateContext(m_sdlWindow);
			if (mGLContext == nullptr) {
				Utils::FatalError("ENJON::WINDOW::CREATE::SD_GL_CONTEXT_NOT_CREATED");
			} 
		}

		// Set current context and window
		SDL_GL_MakeCurrent( m_sdlWindow, mGLContext );

		//Set up glew (optional but recommended)
		GLenum error = glewInit();
		if (error != GLEW_OK) {
			Utils::FatalError("ENJON::WINDOW::CREATE::GLEW_NOT_INITIALIZED");
		}
   
		//Set glewExperimental to true
		glewExperimental = GL_TRUE;

		//Check OpenGL version
		std::printf("***    OpenGL Version:  %s    ***\n", glGetString(GL_VERSION));

		//Set viewport
		glViewport(0, 0, m_screenWidth, m_screenHeight);

		//Set the background color
		glClearColor(0.2f, 0.3f, 0.8f, 1.0f);

		//Set Vsync to enabled
		SDL_GL_SetSwapInterval(0);

		//Enable alpha blending
		glEnable(GL_BLEND);

		//Set blend function type
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		return 0;
	}
 
	void Window::MakeCurrent( )
	{
		// Set current window in SDL
		SDL_GL_MakeCurrent( m_sdlWindow, mGLContext );

		// Set current ImGuiContext 
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		igm->SetContextByWindow( m_sdlWindow ); 
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

	void Window::SetViewport(s32 width, s32 height)
	{
		m_screenWidth = width;
		m_screenHeight = height;	
	}

	void Window::SetViewport(iVec2& dimensions)
	{
		// TODO(John): Need to refresh screen here...
		m_screenWidth = dimensions.x;
		m_screenHeight = dimensions.y;
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

	void Window::SetWindowTitle(const char* title)
	{
		SDL_SetWindowTitle(m_sdlWindow, title);
	}

	void Window::SetWindowFullScreen(int screenWidth, int screenHeight)
	{
		//Need to figure this one out...
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
				// Push back new dropped file
				mDroppedFiles.insert( String(event.drop.file) ); 

				// Just released, so need to explicitly set button state 
				EngineSubsystem( Input )->SetButtonState( KeyCode::LeftMouseButton, false, true ); 

				// Free the filename memory
				SDL_free( event.drop.file ); 
			} break;

			case SDL_WINDOWEVENT: 
			{
				switch ( event.window.event )
				{
					case SDL_WINDOWEVENT_RESIZED: 
					{
						SetViewport( iVec2( (u32)event.window.data1, (u32)event.window.data2 ) ); 
						gs->ReinitializeFrameBuffers( );
					}
					break; 

					case SDL_WINDOWEVENT_ENTER: 
					{
					}
					break;
					
					case SDL_WINDOWEVENT_LEAVE: 
					{
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
						return Result::FAILURE;
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

	void Window::SetWindowCursor( CursorType type )
	{
		// Error checks are for pussies.
		SDL_SetCursor( mSDLCursors[ type ] );
	}

	//==============================================================================

	void Window::InitSDLCursors( )
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

	//==============================================================================
}