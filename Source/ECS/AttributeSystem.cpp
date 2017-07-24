#include "ECS/AttributeSystem.h"

namespace ECS{ namespace Systems { namespace Attributes {

	struct AttributeSystem* NewAttributeSystem(Systems::EntityManager* Manager)
	{
		struct AttributeSystem* System = new AttributeSystem;
		if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_ATTRIBUTE_SYSTEM::System is null"); 
		System->Manager = Manager;
		return System;
	}

	// Updates Attributes of entity that it's attached to
	void Update(struct AttributeSystem* System)
	{

	}	

	void Reset(Systems::EntityManager* Manager, eid32 Entity)
	{
		auto System = Manager->AttributeSystem;

		System->HealthComponents[Entity].Health = 0.0f;
		System->HealthComponents[Entity].Entity = 0;
		System->Masks[Entity] = 0;
		System->Groups[Entity].Entity = 0;
		System->Groups[Entity].Parent = 0;
		System->LootProfiles[Entity] = nullptr;
		System->WeaponProfiles[Entity] = nullptr;
	}
}}}


