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
			Vec2 GetSize( );

			/**
			* @brief
			*/
			Vec2 GetPosition( ); 

		protected: 
			Vec2 mPosition;
			Vec2 mSize; 
			String mLabel;
	};

	class PopUpWindow : public GUIWidget
	{ 
		public:

			/**
			* @brief
			*/
			PopUpWindow( ) = default;

			/**
			* @brief
			*/
			PopUpWindow( const String& label, const Vec2& position, const Vec2& size );

			/**
			* @brief
			*/
			~PopUpWindow( ) = default; 

		protected:
		private:
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
			bool DragFloat2( const String& label, Vec2* vec, f32 speed = 1.0f, f32 min = 0.0f, f32 max = 0.0f );

		protected:
			void BindContext( ); 
			void ProcessEvent( SDL_Event* event );

		private:
			s32 MainMenu();
			void ImGuiStyles();
			void InitializeDefaults(); 

		private:
			std::vector<std::function<void()>> mGuiFuncs;
			std::vector<std::function<void()>> mWindows;
			std::unordered_map<std::string, std::vector<std::function<void()>>> mMainMenuOptions;
			std::vector<GUIDockingLayout> mDockingLayouts; 
			HashMap< Enjon::String, ImFont* > mFonts;
			ImGuiContext* mContext = nullptr;
	};
}

#endif