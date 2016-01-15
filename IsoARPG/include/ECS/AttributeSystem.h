#ifndef ATTRIBUTESYSTEM_H
#define ATTRIBUTESYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

struct AttributeSystem
{
	ECS::Systems::EntityManager* Manager;
	ECS::Component::HealthComponent HealthComponents[MAX_ENTITIES];
	ECS::Component::BitmaskComponent BitMasks[MAX_ENTITIES];
	ECS::Masks::EntityMask Masks[MAX_ENTITIES];
};

namespace ECS{ namespace Systems { namespace Attributes {

	// Constructs and returns new AttributeSystem
	// TODO(John): Write custom allocator for this
	struct AttributeSystem* NewAttributeSystem(Systems::EntityManager* Manager);
	
	// Updates any Attributes Components of entity it is attached to
	void Update(struct AttributeSystem* System);

}}}


#endif

