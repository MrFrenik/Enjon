#pragma once
#ifndef ENJON_IMGUI_DOCK_H
#define ENJON_IMGUI_DOCK_H

#include <functional>

namespace ImGui
{ 
	using VoidCallback = std::function< void( void ) > ;

	enum DockSlotType
	{
		Slot_Left,
		Slot_Right,
		Slot_Top,
		Slot_Bottom,
		Slot_Tab,

		Slot_Float,
		Slot_None
	}; 

	enum CallBackEventType
	{
		OnEnterHorizontalSplitHover,
		OnEnterVerticalSplitHover,
		OnExitSplitHover
	};

	struct DockingLayout
	{
		DockingLayout(const char* child, const char* parent, ImGui::DockSlotType st, float weight)
			: mChild(child), mParent(parent), mSlotType(st), mWeight(weight)
		{}
		~DockingLayout(){}

		const char* mChild;
		const char* mParent;
		DockSlotType mSlotType;
		float mWeight;
	};

	void ConstructContext( ImGuiContext* ctx );

	IMGUI_API void ShutdownDock();
	IMGUI_API void RootDock(const ImVec2& pos, const ImVec2& size);
	IMGUI_API bool BeginDock(const char* label, bool* opened = nullptr, ImGuiWindowFlags extra_flags = 0);
	IMGUI_API void EndDock(); 
	IMGUI_API void SetDockActive( const char* label );
	IMGUI_API void LoadDock();
	IMGUI_API void SaveDock();
	IMGUI_API void Print();

	void DockWith( const char* dock, const char* container, DockSlotType slot, float weight = 0.5f );
	void SetEventCallback( const VoidCallback& callback, const CallBackEventType& eventType );
} // namespace ImGui

#endif