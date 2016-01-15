#include "ECS/AIControllerSystem.h"
#include "ECS/Transform3DSystem.h"

namespace ECS { namespace Systems { namespace AIController {

	struct AIControllerSystem* NewAIControllerSystem(struct EntityManager* Manager)
	{ 
		struct AIControllerSystem* System = new AIControllerSystem;
		System->Manager = Manager;
		return System;
	}

	// Updates Controller of AI it is attached to
	void Update(struct AIControllerSystem* System, eid32 Player)
	{
		struct EntityManager* Manager = System->Manager;
		const Component::Transform3D* Target = &Manager->TransformSystem->Transforms[Player];
		// Let's just make it go towards the player for testing
		for (eid32 ai = 0; ai < Manager->MaxAvailableID; ai++)
		{
			// Check to see if entity has ai controller
			if ((Manager->Masks[ai] & (COMPONENT_AICONTROLLER | COMPONENT_TRANSFORM3D)) == (COMPONENT_AICONTROLLER | COMPONENT_TRANSFORM3D))
			{
				Component::Transform3D* AI = &Manager->TransformSystem->Transforms[ai];

				// Find difference in positions	
				Enjon::Math::Vec3 Difference = Enjon::Math::Vec3::Normalize(Enjon::Math::Vec3(Target->CartesianPosition, Target->Position.z) - Enjon::Math::Vec3(AI->CartesianPosition, AI->Position.z));

				Enjon::Math::Vec2 a = Target->GroundPosition;
				Enjon::Math::Vec2 b = AI->GroundPosition;
				float distance = a.DistanceTo(b);

				if (distance <= 60.0f) AI->VelocityGoal = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
				// Move towards player
				// TODO(John): Come up with some kind of passed in speed parameter to multiply by the difference vector
				//AI->Velocity = Difference; 
				else AI->VelocityGoal = Difference * 2.0f;
			}
		}
	}
}}}




