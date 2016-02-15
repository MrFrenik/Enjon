#include "ECS/EffectSystem.h"

#include <Utils/Errors.h>

#include <stdio.h>
#include <type_traits>


namespace ECS{ namespace Systems { namespace Effect {

	// Constructs and returns new EffectSystem
	struct EffectSystem* NewEffectSystem(Systems::EntityManager* Manager)
	{
		struct EffectSystem* System = new EffectSystem;
		if (System == nullptr) Enjon::Utils::FatalError("EFFECT_SYSTEM::NEW_EFFECT_SYSTEM::System is null.");
		System->Manager = Manager;
		return System;
			
	}
	
	// Updates any Attributes Components of entity it is attached to
	void Update(Systems::EntityManager* Manager)
	{
		for (ECS::eid32 e = 0; e < Manager->MaxAvailableID; e++)
		{
			// Loop through all effects and apply them
			auto* S = &Manager->EffectSystem->TransferredEffects[e];
			auto it = S->begin();
			while(it != S->end())
			{
				auto* E = &it->second;

				// If there's a timer, then need to decrement 
				if (E->Type == EffectType::TEMPORARY)
				{
					auto* T = &E->Timer; 
					T->CurrentTime -= T->DT; 

					// If the timer is out, then need to remove the effect 
					if (T->CurrentTime <= 0.0f)
					{
						it = S->erase(it);
					}
					// Otherwise apply the effect
					else
					{
						E->Apply(E->Entity, Manager);
						++it;
					}
				}
				// Otherwise is permanent, so apply effect
				else
				{
					E->Apply(E->Entity, Manager);
					++it;
				}
			}
		}
	}

}}}


