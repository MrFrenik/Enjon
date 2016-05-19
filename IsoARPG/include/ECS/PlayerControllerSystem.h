#ifndef PLAYERCONTROLLERSYSTEM_H
#define PLAYERCONTROLLERSYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"
#include "SpatialHash.h"

#define	WALKPACE 	8.0f
#define SPRINTPACE 	WALKPACE * 2.0f

struct PlayerControllerSystem
{
	ECS::Systems::EntityManager* Manager;
	ECS::Component::PlayerController PlayerControllers[MAX_ENTITIES];
	std::vector<Enjon::uint32> Targets;  // Totally testing this for now...

	// Probably just make a targeting system and component for this data
	Enjon::uint32 CurrentIndex;
	Enjon::uint32 CurrentTarget;
	SpatialHash::Grid* Grid;
}; 

namespace ECS { namespace Systems { namespace PlayerController {

	// Constructs and returns new PlayerControllerSystem
	// TODO(John): Write custom allocator for this
	struct PlayerControllerSystem* NewPlayerControllerSystem(Systems::EntityManager* Manager);
	
	// Updates Controller of player it is attached to
	void Update(struct PlayerControllerSystem* System);	

	bool GetTargeting();
}}}


#endif

