#ifndef PLAYERCONTROLLERSYSTEM_H
#define PLAYERCONTROLLERSYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

struct PlayerControllerSystem
{
	ECS::Systems::EntityManager* Manager;
	ECS::Component::PlayerController PlayerControllers[MAX_ENTITIES];
}; 

namespace ECS { namespace Systems { namespace PlayerController {

	// Constructs and returns new PlayerControllerSystem
	// TODO(John): Write custom allocator for this
	struct PlayerControllerSystem* NewPlayerControllerSystem(Systems::EntityManager* Manager);
	
	// Updates Controller of player it is attached to
	void Update(struct PlayerControllerSystem* System);	
}}}


#endif

