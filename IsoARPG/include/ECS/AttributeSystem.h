#ifndef ATTRIBUTESYSTEM_H
#define ATTRIBUTESYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"
#include "Loot.h"

#include <unordered_map>

struct AttributeSystem
{
	ECS::Systems::EntityManager* Manager;
	ECS::Component::HealthComponent HealthComponents[MAX_ENTITIES];
	ECS::Component::BitmaskComponent BitMasks[MAX_ENTITIES];
	ECS::Masks::EntityMask Masks[MAX_ENTITIES];
	ECS::Component::GroupComponent Groups[MAX_ENTITIES];

	const Loot::LootProfile* LootProfiles[MAX_ENTITIES];
	const Loot::Weapon::WeaponProfile* WeaponProfiles[MAX_ENTITIES];

	// Trying this out with a hashmap
	// std::unordered_map<ECS::eid32, const Loot::Weapon::WeaponProfile*> WeaponProfiles;
	// std::unordered_map<ECS::eid32, const Loot::LootProfile*> LootProfiles;
};

namespace ECS{ namespace Systems { namespace Attributes {

	// Constructs and returns new AttributeSystem
	// TODO(John): Write custom allocator for this
	struct AttributeSystem* NewAttributeSystem(Systems::EntityManager* Manager);
	
	// Updates any Attributes Components of entity it is attached to
	void Update(struct AttributeSystem* System);

}}}

#endif

