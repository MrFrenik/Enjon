#pragma once
#ifndef ENJON_IM_GUI_MANAGER_H
#define ENJON_IM_GUI_MANAGER_H

#include "ImGui/imgui_impl_sdl_gl3.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_dock.h"

#include "Math/Vec2.h"
#include "System/Types.h"
#include "Subsystem.h"
#include "Defines.h"

#include <SDL2/sdl.h>

#include <functional>
#include <unordered_map>
#include <vector>

struct ImFont;
struct ImGuiContext;

namespace Enjon
{ 
	class Window;
	class Vec4;
	class Object;
	class MetaClass;
	class MetaPropertyArrayBase;
	class MetaPropertyHashMapBase;
	class MetaProperty;
	class Engine;

	enum GUIDockSlotType
	{
		Slot_Left,
		Slot_Right,
		Slot_Top,
		Slot_Bottom,
		Slot_Tab, 
		Slot_Float,
		Slot_None
	};

	enum class GUIFlags
	{
		AutoCalculateSize,
		None
	};

	class GUIWidget
	{ 
		public:
			/**
			* @brief
			*/
			GUIWidget( ) = default;

			/**
			* @brief
			*/
			GUIWidget( const String& label );

			/**
			* @brief
			*/
			GUIWidget( const String& label, const Vec2& position, const Vec2& size );

			/**
			* @brief
			*/
			virtual ~GUIWidget( ) = default;

			/**
			* @brief
			*/
			virtual void DoWidget( )
			{ 
				// Nothing by default
			}

			/**
			* @brief
			*/
			void SetSize( const Vec2& size );

			/**
			* @brief
			*/
			Vec2 GetSize( );

			/**
			* @brief
			*/
			Vec2 GetPosition( ); 

		protected: 
			Vec2 mPosition;
			Vec2 mSize = Vec2( 200.0f, 400.0f );
			String mLabel;
			bool mEnabled = false; 
			bool mHovered = false;
			bool mAutoCalculateSize = true;
	};

	using GUICallbackFunc = std::function<void( )>;

	class PopupWindow : public GUIWidget
	{ 
		public:

			/**
			* @brief
			*/
			PopupWindow( ) = default;

			/**
			* @brief
			*/
			PopupWindow( const String& label, const Vec2& position, const Vec2& size );

			/**
			* @brief
			*/
			~PopupWindow( ) = default; 

			/**
			* @brief
			*/
			void operator+=( const GUICallbackFunc& func );

			/**
			* @brief
			*/
			bool Enabled( ); 

			/**
			* @brief
			*/
			void RegisterCallback( const GUICallbackFunc& func ); 

			/**
			* @brief
			*/
			void SetFadeInSpeed( const f32& speed );

			/** 
			* @brief
			*/
			void SetFadeOutSpeed( const f32& speed );

			/**
			* @brief
			*/
			void Activate( const Vec2& position );

			/**
			* @brief
			*/
			void Deactivate( );

			/**
			* @brief
			*/
			bool Hovered( );

			/**
			* @brief
			*/
			virtual void DoWidget( ) override;

		protected:

			/**
			* @brief
			*/
			void Reset( );

		private:
			Vector< GUICallbackFunc > mCallbacks;
			f32 mFadeTimer = 0.0f;
			f32 mFadeInSpeed = 5.0f;
			f32 mFadeOutSpeed = 10.0f;
			f32 mFadeOutTimer = 1.0f;
			bool mBeginDisable = false;
	};

	struct GUIDockingLayout
	{
		GUIDockingLayout(const char* child, const char* parent, GUIDockSlotType st, float weight)
			: mChild(child), mParent(parent), mSlotType(st), mWeight(weight)
		{}
		~GUIDockingLayout(){}

		const char* mChild;
		const char* mParent;
		GUIDockSlotType mSlotType;
		float mWeight;
	};

	/*
		Static class meant to be a central hub for registering 
		ImGui commands
	*/ 
	ENJON_CLASS( )
	class ImGuiManager : public Subsystem
	{
		friend Engine;
		friend GUIWidget;

		ENJON_CLASS_BODY( )

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

			void Init(SDL_Window* window);
			void LateInit(SDL_Window* window);
			void Register(std::function<void()> func);
			void RegisterMenuOption(std::string name, std::function<void()> func);
			void RegisterWindow(std::function<void()> func);
			void RegisterDockingLayout(const GUIDockingLayout& layout);
			void RenderGameUI(Enjon::Window* window, f32* view, f32* projection);
			void Render(SDL_Window* window);
			void InspectObject( const Object* object );
			void DebugDumpObject( const Enjon::Object* object );
			void DebugDumpProperty( const Enjon::Object* object, const Enjon::MetaProperty* prop );
			void DebugDumpArrayProperty( const Enjon::Object* object, const Enjon::MetaPropertyArrayBase* prop );
			void DebugDumpHashMapProperty( const Enjon::Object* object, const Enjon::MetaPropertyHashMapBase* prop );
			ImFont* GetFont( const Enjon::String& name );
			ImGuiContext* GetContext( );

		public:

			/**
			* @brief
			*/
			void Text( const String& text );

			/**
			* @brief
			*/
			void Separator( );

			/**
			* @brief
			*/
			void PushFont( const String& fontName );

			/**
			* @brief
			*/
			void PopFont( );

			/**
			* @brief
			*/
			bool DragFloat2( const String& label, Vec2* vec, f32 speed = 1.0f, f32 min = 0.0f, f32 max = 0.0f );

		protected:
			void BindContext( ); 
			void ProcessEvent( SDL_Event* event );

		private:
			s32 MainMenu();
			void ImGuiStyles();
			void InitializeDefaults(); 

		private:
			Vector<std::function<void()>> mGuiFuncs;
			Vector<std::function<void()>> mWindows;
			HashMap<String, Vector<std::function<void()>>> mMainMenuOptions;
			Vector<GUIDockingLayout> mDockingLayouts; 
			HashMap< Enjon::String, ImFont* > mFonts;
			ImGuiContext* mContext = nullptr;
	};
}

#endif