#ifndef LABELSYSTEM_H
#define LABELSYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

struct LabelSystem
{
	ECS::Systems::EntityManager* Manager;
	ECS::Component::Label Labels[MAX_ENTITIES];
};

namespace ECS{ namespace Systems { namespace Label {

	// Constructs and returns new AIControllerSystem
	// TODO(John): Write custom allocator for this
	struct LabelSystem* NewLabelSystem(Systems::EntityManager* Manager);
	
	// Updates Label Component of entity it is attached to
	void Update(struct LabelSystem* System);
}}}


#endif

