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

			/*
			* @brief
			*/
			inline int ShowMouseCursor(int val) const { return SDL_ShowCursor(val); } 

			/*
			* @brief
			*/
			inline int GetScreenWidth() const { return m_screenWidth; }

			/*
			* @brief
			*/
			inline int GetScreenHeight() const { return m_screenHeight; }

			/*
			* @brief
			*/
			virtual void Update( );

			/*
			* @brief
			*/
			void SetViewport(int width, int height);

			/*
			* @brief
			*/
			void SetViewport(Enjon::iVec2& dimensions);

			/*
			* @brief
			*/
			iVec2 GetViewport() const;
		
			/*
			* @brief
			*/
			void Clear(float val = 1.0f, 
						GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, 
						const ColorRGBA32& color = RGBA32_Black());

			/*
			* @brief
			*/
			void SwapBuffer(); 

			/*
			* @brief
			*/
			void MakeCurrent( );

			/*
			* @brief
			*/
			Result ProcessInput( const SDL_Event& event );

			/*
			* @brief
			*/
			SDL_Window* GetWindowContext() { return m_sdlWindow; }

			/*
			* @brief
			*/
			SDL_Window* GetSDLWindow() { return m_sdlWindow; }

			/*
			* @brief
			*/
			const HashSet<String>& GetDroppedFiles( ); 

			/*
			* @brief
			*/
			static void SetWindowCursor( CursorType type );

			/*
			* @brief
			*/
			GUIContext* GetGUIContext( )
			{
				return &mGUIContext;
			}

			/*
			* @brief
			*/
			bool IsMouseInWindow( );


			/*
			* @brief
			*/
			void Window::PrintDebugInfo( );

			/*
			* @brief
			*/
			void SetFocus( );

			/*
			* @brief
			*/
			bool IsFocused( );

			// TOTAL HACKS FOR NOW
			void SetWorld( World* world );
			World* GetWorld( );

			static bool AnyWindowHovered( );
			static void AddNewWindow( const WindowParams& params ); 
			static void DestroyWindow( Window* window ); 
			static void InitializeWindows( ); 
			static void CleanupWindows( bool destroyAll = false );
			static void WindowsUpdate( );

		protected:

			/** 
			* @brief
			*/
			Vector< Window* > Destroy( );

			static void InitSDLCursors( );

			void ClearDroppedFiles( );

			virtual void ExplicitDestroy( )
			{ 
			}

			void NukeAllNextFrame( );

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
			static bool mNukeAll;
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif