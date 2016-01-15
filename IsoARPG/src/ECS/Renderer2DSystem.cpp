#include "ECS/Renderer2DSystem.h"


namespace ECS { namespace Systems { namespace Renderer2D {

	// Updates Renderers of EntityManager
	void Update(struct EntityManager* Manager)
	{
		// Need to render components here
		for (eid32 e = 0; e < Manager->MaxAvailableID; e++)
		{
			// Do something here...
		}
	}

	// Create new Render2DSystem
	Renderer2DSystem* NewRenderer2DSystem(struct EntityManager* Manager)
	{
		struct Renderer2DSystem* System = new Renderer2DSystem;
		if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_RENDERER2D_SYSTEM::System is null"); 
		System->Manager = Manager;
		return System;
	}
}}}



