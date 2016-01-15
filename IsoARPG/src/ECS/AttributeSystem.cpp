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
}}}


