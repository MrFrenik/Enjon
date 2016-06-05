#ifndef GUI_ANIMATION_ELEMENT_H
#define GUI_ANIMATION_ELEMENT_H

#include "GUI/GUIElement.h"
#include "Graphics/Animations.h"
#include "Math/Vec2.h"


namespace Enjon { namespace GUI {

	// AnimationFrameElement
	struct GUIAnimationElement : GUIElement<GUIAnimationElement>
	{
		void Init() {}

		Anim* CurrentAnimation;
		uint32_t CurrentIndex;
	};

	namespace AnimationElement
	{
		void AABBSetup(GUIAnimationElement* SceneAnimation);
	}
}}


#endif