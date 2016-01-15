#include "ECS/InventorySystem.h"

namespace ECS { namespace Systems { namespace Inventory {

	// Updates Transforms of EntityManager
	void Update(InventorySystem* System)
	{

	}

	// Creates new Transform3DSystem
	InventorySystem* NewInventorySystem(struct EntityManager* Manager)
	{
		struct InventorySystem* System = new InventorySystem;
		if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_INVENTORY_SYSTEM::System is null");
		System->Manager = Manager;
		return System;
	}
}}}



