#include <Graphics/Window.h>
#include <Utils/Errors.h>

namespace Enjon { namespace Graphics {

	Window::Window()
		: m_isfullscreen(false)
	{
	}


	Window::~Window()
	{
	}

	int Window::Init( std::string windowName, int screenWidth, int screenHeight, WindowFlags currentFlags /* = WindowFlags::DEFAULT */)
	{
		m_screenWidth = screenWidth;
		m_screenHeight = screenHeight;

		Uint32 flags = SDL_WINDOW_OPENGL;
		
		if( currentFlags & INVISIBLE )
		{
			flags |= SDL_WINDOW_HIDDEN;
		}
		
		if( currentFlags & FULLSCREEN )
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			m_isfullscreen = true;
		}

		if( currentFlags & BORDERLESS )
		{
			flags |= SDL_WINDOW_BORDERLESS;
		}

		//Open an SDL window
		m_sdlWindow = SDL_CreateWindow( windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, flags );
		if (m_sdlWindow == NULL) {
			Utils::FatalError("ENJON::WINDOW::CREATE::WINDOW_NOT_CREATED");
		}

		//Set up our OpenGL context
		SDL_GLContext glContext = SDL_GL_CreateContext(m_sdlWindow);
		if (glContext == nullptr) {
			Utils::FatalError("ENJON::WINDOW::CREATE::SD_GL_CONTEXT_NOT_CREATED");
		}

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

	void Window::SwapBuffer()
	{
		SDL_GL_SwapWindow(m_sdlWindow);
	}
	
	void Window::Clear(float val, GLbitfield mask, const ColorRGBA16& color)
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

}}