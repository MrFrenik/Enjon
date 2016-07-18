#ifndef AICONTROLLERSYSTEM_H
#define AICONTROLLERSYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

struct AIControllerSystem
{
	ECS::Systems::EntityManager* Manager;
	ECS::Component::AIController AIControllers[MAX_ENTITIES];
};

namespace ECS { namespace Systems { namespace AIController {

	// Constructs and returns new AIControllerSystem
	// TODO(John): Write custom allocator for this
	struct AIControllerSystem* NewAIControllerSystem(struct EntityManager* Manager);
	
	// Updates Controller of AI it is attached to
	void Update(struct AIControllerSystem* System, eid32 Player);

	void Reset(struct EntityManager* Manager, eid32 Entity);	
}}}


#endif



