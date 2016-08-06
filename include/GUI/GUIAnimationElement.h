#ifndef GUI_ANIMATION_ELEMENT_H
#define GUI_ANIMATION_ELEMENT_H

#include "GUI/GUIElement.h"
#include "Graphics/Animations.h"
#include "Math/Vec2.h"
#include "Defines.h"

#include "AnimManager.h"

namespace Enjon { namespace GUI {

	// AnimationFrameElement
	struct GUIAnimationElement : GUIElement<GUIAnimationElement>
	{
		GUIAnimationElement()
		{
			// Set up type
			this->Type = GUIType::SCENE_ANIMATION;

			// Set up states
			this->State = ButtonState::INACTIVE;
			this->HoverState = HoveredState::OFF_HOVER;

			// Set up member variables
			this->CurrentIndex = 0;
			this->CurrentAnimation = nullptr;

			// Set up SceneAnimation's on_hover signal
			this->on_hover.connect([&]()
			{
				this->HoverState = HoveredState::ON_HOVER;
			});

			// Set up SceneAnimation's off_hover signal
			this->off_hover.connect([&]()
			{
				this->HoverState = HoveredState::OFF_HOVER;
			});

		}

		void Init() {}

		void Update()
		{
			
		}

		bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
		{
			return true;
		}

		void Draw(EG::SpriteBatch* TB)
		{

		}

		EA::Anim* CurrentAnimation;
		uint32_t CurrentIndex;
	};

	namespace AnimationElement
	{
		void AABBSetup(GUIAnimationElement* SceneAnimation);
	}
}}


#endif