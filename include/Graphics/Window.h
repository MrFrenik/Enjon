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
#include "Subsystem.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon 
{
	class WindowSubsystem;
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

	typedef std::function< const MetaClass* (void) > MetaClassFunc;

	typedef struct WindowParams
	{ 
		MetaClassFunc mMetaClassFunc;
		String mName;
		u32 mWidth;
		u32 mHeight;
		WindowFlagsMask mFlags = WindowFlags::DEFAULT;
		s32 id = -1;
		void* mData;
	} WindowParams;

	ENJON_CLASS( )
	class Window : public Enjon::Object
	{
		friend GraphicsSubsystem;
		friend WindowSubsystem;

		ENJON_CLASS_BODY( Window )

		public: 

			/*
			* @brief
			*/
			virtual void Init( const WindowParams& params ) 
			{
				// Nothing by default
			}

			/*
			* @brief
			*/
			void SetWindowTitle( const String& title );
			/*
			* @brief
			*/

			void SetWindowFullScreen( const s32& screenWidth, const s32& screenHeight );

			/*
			* @brief
			*/
			bool inline IsFullScreen() const 
			{ 
				return m_isfullscreen; 
			}

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
			void SetViewport( const s32& width, const s32& height );

			/*
			* @brief
			*/
			void SetViewport( const iVec2& dimensions ); 

			/*
			* @brief
			*/
			iVec2 GetViewport() const; 

			/*
			* @brief
			*/
			void SetPosition( const iVec2& position );

			/*
			* @brief
			*/
			void SetPosition( const u32& x, const u32& y );

			/*
			* @brief
			*/
			void MaximizeWindow( );

			/*
			* @brief
			*/
			void SetSize( const iVec2& dims );
		
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

			void SetGUIContext( const GUIContext& ctx )
			{
				mGUIContext = ctx;
			}

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
			void HideWindow( );

			/*
			* @brief
			*/
			void ShowWindow( );

			/*
			* @brief
			*/
			bool IsMouseInWindow( );


			/*
			* @brief
			*/
			void PrintDebugInfo( );

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

			u32 GetWindowID()
			{
				return mID;
			}

		protected:

			void InitInternal( const WindowParams& params );

			/** 
			* @brief
			*/
			Vector< Window* > Destroy( );

			//static void InitSDLCursors( );

			void ClearDroppedFiles( );

			virtual void ExplicitDestroy( )
			{ 
			}

			void NukeAllNextFrame( );

		protected:

			ENJON_PROPERTY( ReadOnly )
			s32 m_screenWidth;
			
			ENJON_PROPERTY( ReadOnly )
			s32 m_screenHeight;

			ENJON_PROPERTY( ReadOnly )
			bool m_isfullscreen; 
 
			World* mWorld = nullptr; 
			bool mNeedToClearDroppedFiles = false; 
			SDL_Window* m_sdlWindow;
			HashSet<String> mDroppedFiles;
			GUIContext mGUIContext;
			bool mMouseIsHovering = false; 
			s32 mID = -1;
	};

	ENJON_CLASS( )
	class WindowSubsystem : public Subsystem
	{ 
		ENJON_CLASS_BODY( WindowSubsystem )

		public: 

			/**
			*@brief
			*/
			virtual Result Initialize() override;

			/**
			*@brief
			*/
			virtual void Update( const f32 dT ) override;
			
			/**
			*@brief
			*/
			virtual Result Shutdown() override;

			/**
			*@brief
			*/
			void AttemptLoadGLContext( SDL_Window* window );

			/**
			*@brief
			*/
			SDL_GLContext GetGLContext( );

			/**
			*@brief
			*/
			SDL_Surface* AttemptLoadWindowIcon( );

			/**
			*@brief
			*/
			SDL_Cursor* GetCursor( CursorType type );

			/**
			*@brief
			*/
			Vector< Window* > GetWindows( );

			/**
			*@brief
			*/
			Window* GetWindow( const u32& id );

			/**
			*@brief CAREFUL WHEN CALLING THIS
			*/
			void ForceInitWindows( );

			/**
			*@brief CAREFUL WHEN CALLING THIS
			*/
			void ForceCleanupWindows( );

			Vec2 GetDisplaySize( const u32& displayIndex = 0 );
			u32 NumberOfHoveredWindows( );
			b32 AnyWindowHovered( );
			s32 AddNewWindow( WindowParams params ); 
			// void DestroyWindow( Window* window ); 
			void DestroyWindow( const u32& id );
			void InitializeWindows( ); 
			void CleanupWindows( b32 destroyAll = false );
			void WindowsUpdate( );

	private:
			void InitSDLCursors( );
			s32 FindNextFreeID( ); 
			void DestroyAll( );

		private:
			SDL_GLContext mGLContext = nullptr;
			HashMap<u32, SDL_Cursor*> mSDLCursors; 
			Vector< Window* > mWindows;
			Vector< WindowParams > mWindowsToInit;
			HashMap< u32, Window* > mWindowIDMap;
			Vector< u32 > mWindowsToDestroy;
			b32 mNukeAll;
			SDL_Surface* mWindowIcon = nullptr;
			u32 mFreeWindowID = 0;
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif