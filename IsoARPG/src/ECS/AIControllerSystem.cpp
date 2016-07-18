#include "ECS/AIControllerSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/EntityFactory.h"

#include <Graphics/SpriteSheet.h>
#include <Math/Maths.h>

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
				// Get AI behavior tree and blackboard
				auto AC = &Manager->AIControllerSystem->AIControllers[ai];
				auto Brain = AC->Brain;
				auto BB = AC->BB;

				// Run the tree
				Brain->Run(BB);
			}
		}
	}

	void Reset(struct EntityManager* Manager, eid32 Entity)
	{
		auto AIController = &Manager->AIControllerSystem->AIControllers[Entity];

		// Free memory / The behavior tree is managed by the BT Manager
		delete AIController->SO;
		delete AIController->BB;

		// Set to null
		AIController->Brain = nullptr;
		AIController->SO = nullptr;
		AIController->BB = nullptr;


		AIController->Entity = 0;
	}	
}}}




