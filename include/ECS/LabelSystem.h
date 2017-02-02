#ifndef ENJON_LABEL_SYSTEM_H
#define ENJON_LABEL_SYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

struct LabelSystem
{
	ECS::Systems::EntityManagerDeprecated* Manager;
	ECS::Component::Label Labels[MAX_ENTITIES];
};

namespace ECS{ namespace Systems { namespace Label {

	// Constructs and returns new AIControllerSystem
	// TODO(John): Write custom allocator for this
	struct LabelSystem* NewLabelSystem(Systems::EntityManagerDeprecated* Manager);
	
	// Updates Label Component of entity it is attached to
	void Update(struct LabelSystem* System);
}}}


#endif

