#include "ECS/LabelSystem.h"

namespace ECS{ namespace Systems { namespace Label {

	// Constructs and returns new AIControllerSystem
	// TODO(John): Write custom allocator for this
	struct LabelSystem* NewLabelSystem(Systems::EntityManager* Manager)
	{
		struct LabelSystem* System = new LabelSystem;
		if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_LABEL_SYSTEM::System is null"); 
		System->Manager = Manager;
		return System;
	}
	
	// Updates Label Component of entity it is attached to
	void Update(struct LabelSystem* System)
	{

	}
}}}


