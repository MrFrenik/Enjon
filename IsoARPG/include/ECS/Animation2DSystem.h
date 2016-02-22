#ifndef ANIMATION2DSYSTEM_H
#define ANIMATION2DSYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

struct Animation2DSystem
{
	ECS::Systems::EntityManager* Manager;
	ECS::Component::Animation2D Animations[MAX_ENTITIES];
};

namespace ECS { namespace Systems { namespace Animation2D {
	
	enum EntityAnimationState { WALKING, ATTACKING, IDLE }; // This should be split up into continuous and discrete states
	enum Weapons { BOW, DAGGER, AXE };

	void SetPlayerState(EntityAnimationState State);
	EntityAnimationState GetPlayerState();
	void SetCurrentWeapon(Weapons CurrentWeapon);

	// Updates Transforms of EntityManager
	void Update(struct EntityManager* Manager);	
	
	// Creates new Transform3DSystem
	Animation2DSystem* NewAnimation2DSystem(struct EntityManager* Manager);
}}}


#endif

