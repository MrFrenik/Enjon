#include "GUI/GUIAnimationElement.h"
#include "Utils/Errors.h"


namespace Enjon { namespace GUI {

	namespace AnimationElement
	{
		void AABBSetup(GUIAnimationElement* SceneAnimation)
		{
			auto AABB = &SceneAnimation->AABB;
			auto& Position = SceneAnimation->Position;
			
			// Get information of current frame
			auto CurrentAnimation = SceneAnimation->CurrentAnimation;

			if (CurrentAnimation != nullptr)
			{
				float ScalingFactor = 1.0f;
				auto Frame = &CurrentAnimation->Frames.at(SceneAnimation->CurrentIndex);
				auto& Dims = Frame->UVs;
				auto& SSize = Frame->SourceSize;
				auto& Offsets = Frame->Offsets;

				AABB->Min = Enjon::Vec2(Position.x + Offsets.x * ScalingFactor, 
									 Position.y + Offsets.y * ScalingFactor);

				AABB->Max = AABB->Min + Enjon::Vec2(SSize.x, SSize.y) * ScalingFactor;
			}

			else
			{
				EU::FatalError("AABBSetup: CurrentAnimation is null.");
			}
		}
	}
}}
