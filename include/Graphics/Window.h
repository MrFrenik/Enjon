#ifndef ENJON_WINDOW_H
#define ENJON_WINDOW_H 

#include <string>
#include <bitset>

#include <SDL2/SDL.h>
#include <GLEW/glew.h>

#include "ImGui/ImGuiManager.h"
#include "Base/Object.h"
#include "Graphics/Vertex.h"
#include "System/Types.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace Enjon 
{
	class GraphicsSubsystem;
	class iVec2;

	enum CursorType
	{
		Arrow,
		IBeam,
		SizeNWSE,
		SizeNESW,
		SizeWE,
		SizeNS,
		SizeAll,
		Hand 
	};

	enum WindowFlags : u32
	{ 
		DEFAULT 	= 0x00, 
		INVISIBLE 	= 0x01, 
		FULLSCREEN 	= 0x02, 
		BORDERLESS 	= 0x04,
		RESIZABLE 	= 0x08,
		COUNT 
	}; 

	inline WindowFlags operator|( WindowFlags a, WindowFlags b )
	{
		return static_cast<WindowFlags>( static_cast<u32>( a ) | static_cast<u32>( b ) );
	}

	inline void operator|=( WindowFlags& a, WindowFlags b )
	{
		a = a | b;
	}

	enum MouseCursorFlags{ GET_STATUS = -1, HIDE = 0, SHOW = 1 };

	typedef std::bitset<static_cast<size_t>(WindowFlags::COUNT)> WindowFlagsMask;

	struct WindowParams
	{
		Window* mWindow;
		String mName;
		u32 mWidth;
		u32 mHeight;
		WindowFlagsMask mFlags;
	};

	class Window : public Enjon::Object
	{
		friend GraphicsSubsystem;

		public: 
			Window();
			~Window();

			virtual int Init( std::string windowName, int screenWidth, int screenHeight, WindowFlagsMask currentFlags = WindowFlagsMask( (u32)WindowFlags::DEFAULT ) ); 
			void SetWindowTitle(const char* title);
			void SetWindowFullScreen(int screenWidth, int screenHeight);

			bool inline IsFullScreen() const { return m_isfullscreen; }

			inline int ShowMouseCursor(int val) const { return SDL_ShowCursor(val); } 
			inline int GetScreenWidth() const { return m_screenWidth; }
			inline int GetScreenHeight() const { return m_screenHeight; }

			virtual void Update( );

			void SetViewport(int width, int height);
			void SetViewport(Enjon::iVec2& dimensions);
			iVec2 GetViewport() const;
		
			void Clear(float val = 1.0f, 
						GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, 
						const ColorRGBA32& color = RGBA32_Black());
			void SwapBuffer(); 

			void MakeCurrent( );

			Result ProcessInput( const SDL_Event& event );

			SDL_Window* GetWindowContext() { return m_sdlWindow; }
			SDL_Window* GetSDLWindow() { return m_sdlWindow; }

			const HashSet<String>& GetDroppedFiles( ); 

			static void SetWindowCursor( CursorType type );

			GUIContext* GetGUIContext( )
			{
				return &mGUIContext;
			}

			bool IsMouseInWindow( );

			// TOTAL HACKS FOR NOW
			void SetWorld( World* world );
			World* GetWorld( );

			static void AddNewWindow( const WindowParams& params );
			static void DestroyWindow( Window* window ); 
			static void InitializeWindows( );

		protected:

			/** 
			* @brief
			*/
			void Destroy( );

			static void InitSDLCursors( );

			void ClearDroppedFiles( );

			virtual void ExplicitDestroy( )
			{ 
			}

		protected:
			static SDL_GLContext mGLContext;
			SDL_Window* m_sdlWindow;
			int m_screenWidth;
			int m_screenHeight;
			bool m_isfullscreen; 
			HashSet<String> mDroppedFiles;
			bool mNeedToClearDroppedFiles = false;
			static HashMap<CursorType, SDL_Cursor*> mSDLCursors; 
			GUIContext mGUIContext;
			bool mMouseIsHovering = false;
			World* mWorld = nullptr; 
			static Vector< WindowParams > mWindowsToInit;
			static Vector< Window* > mWindowsToDestroy;
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif