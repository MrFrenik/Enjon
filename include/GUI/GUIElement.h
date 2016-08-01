#ifndef GUIELEMENT_H
#define GUIELEMENT_H

#include "Defines.h" 

#include "Graphics/Color.h"
#include "Physics/AABB.h"

#include "GUI/Signal.h"
#include "GUI/Property.h"


#include <vector>
#include <string>
#include <cstdint>

namespace Enjon { namespace GUI {

	enum ButtonState { INACTIVE, ACTIVE };
	enum HoveredState { OFF_HOVER, ON_HOVER };
	enum GUIType { BUTTON, TEXTBOX, SCENE_ANIMATION };

	// GUI Element
	struct GUIElementBase
	{
		virtual void Init() = 0;

		GUIElementBase* Parent;
		EM::Vec2 Position;
		EP::AABB AABB;
		GUIType Type;

		std::string Text;
		ButtonState State;
		HoveredState HoverState;
		EGUI::Signal<> on_click;
		EGUI::Signal<> on_hover;
		EGUI::Signal<> off_hover;
		EG::ColorRGBA16 Color;
	};

	template <typename T>
	struct GUIElement : public GUIElementBase
	{
		void Init()
		{
			static_cast<T*>(this)->Init();
		}
	};


	// TextBox
	struct GUITextBox : GUIElement<GUITextBox>
	{
		void Init()
		{}

		ButtonState State;
		HoveredState HoverState;
		std::string Text;
		int32_t CursorIndex;
		
		EGUI::Signal<std::string> on_keyboard;
		EGUI::Signal<> on_backspace;
	};

	// Group is responsible for holding other gui elements and then transforming them together
	struct GUIGroup : GUIElement<GUIGroup>
	{
		void Init()
		{
			std::cout << "Yeah..." << std::endl;
		}

		// Vector of children
		std::vector<GUIElementBase*> Children;
	};

	inline GUIGroup* AddToGroup(GUIGroup* Group, GUIElementBase* Element)
	{
		// Push back into group's children
		Group->Children.push_back(Element);

		// Set Group as parent of child
		Element->Parent = Group;

		return Group;
	}


}}


#endif