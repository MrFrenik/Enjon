#ifndef EFFECTS_H
#define EFFECTS_H

#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"
#include "ECS/Renderer2DSystem.h"
#include "ECS/Transform3DSystem.h"

#include <Graphics/Color.h>
#include <Math/Maths.h>

/*
	A list of all the possible effects in game. 
	These effects will be loaded from script files eventually to make this more stream-lined. 
*/

namespace ECS { namespace Effects {

	inline void Stop(ECS::eid32 E, ECS::Systems::EntityManager* Manager)
	{
		Manager->TransformSystem->Transforms[E].Velocity = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
	}

	// Purely for testing this out to see if it will even transfer
	inline void ChangeColor(ECS::eid32 E, ECS::Systems::EntityManager* Manager, Enjon::Graphics::ColorRGBA16 C)
	{
		Manager->Renderer2DSystem->Renderers[E].Color = C; 
	}

	inline void Damage(ECS::eid32 E, ECS::Systems::EntityManager* Manager)
	{
		// Apply damage
		auto A = Enjon::Random::Roll(0, 3);
		auto D = A / 20.0f;
		Manager->AttributeSystem->HealthComponents[E].Health -= D;
	}

	/* Applies poison damage to entity over time */
	void Poison(ECS::eid32 E, ECS::Systems::EntityManager* Manager)
	{
		// Change to green color
		ChangeColor(E, Manager, Enjon::Graphics::RGBA16_Green());

		// Apply damage
		auto* T = &Manager->EffectSystem->TransferredEffects[E]["Poison"].Timer;
		Damage(E, Manager);
	}


	/* Slows down velocity of entity to given amount */
	inline void Cold(ECS::eid32 E, ECS::Systems::EntityManager* Manager)
	{
		// Change color to blue
		ChangeColor(E, Manager, Enjon::Graphics::RGBA16_Blue());;

		// 'Freeze' velocity
		Stop(E, Manager);

		// Apply damage
		auto* T = &Manager->EffectSystem->TransferredEffects[E]["Cold"].Timer;
		Damage(E, Manager);
	}
}}



#endif