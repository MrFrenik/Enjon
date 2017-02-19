#pragma once
#ifndef ENJON_IMGUI_DOCK_H
#define ENJON_IMGUI_DOCK_H

#include <vector>
#include <functional>

#include "ImGui/imgui.h"

class GuiWindow;

namespace ImGuiDock
{
	enum class DockSlot { Left, Right, Top, Bottom, Tab, None };
	
	struct Dock;
	class Dockspace;

	struct Container
	{
		Container *splits[2]{ nullptr, nullptr };
		Container *parent = nullptr;
		Dock *activeDock = nullptr;
		
		std::vector<Dock*>docks;

		bool verticalSplit = false;
		bool alwaysAutoResize = true;
		float size = 0;
	};

	struct Dock
	{
		Dock *initialize(const char *dtitle,  bool dcloseButton, ImVec2 dminSize, std::function<void(ImVec2)> ddrawFunction)
		{
			title = dtitle;
			closeButton = dcloseButton;
			minSize = dminSize;
			drawFunction = ddrawFunction;
			return this;
		};

		//Container *parent = nullptr;
		Container *container = nullptr;
		Dockspace *redockFrom = nullptr;
		Dock *redockTo = nullptr;

		const char *title;
		DockSlot dockSlot = DockSlot::Tab;
		DockSlot redockSlot = DockSlot::None;
		bool closeButton = true;
		bool undockable = false;
		bool draging = false;
		ImVec2 lastSize;
		ImVec2 minSize;

		std::function<void(ImVec2)> drawFunction;
		std::function<bool(void)> onCloseFunction;
	};
		
	class Dockspace
	{
	public:
		Dockspace();
		~Dockspace();

		bool dock(Dock *dock, DockSlot dockSlot, float size = 0, bool active = false);
		bool dockWith(Dock *dock, Dock *dockTo, DockSlot dockSlot, float size = 0, bool active = false);
		bool undock(Dock *dock);
		
		void updateAndDraw(ImVec2 size);
		void clear();

		std::vector<Dock*>m_docks;

		Container m_container;
		std::vector<Container*>m_containers;

	protected:

		void _renderTabBar(Container *container, ImVec2 size, ImVec2 cursorPos);
		bool _getMinSize(Container *container, ImVec2 *min);

		enum DockToAction
		{
			eUndock, eDrag, eClose, eNull
		};

		Dock *m_currentDockTo = nullptr;
		DockToAction m_currentDockToAction = eNull;
	};
};

#endif