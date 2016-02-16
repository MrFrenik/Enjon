#ifndef EFFECT_SYSTEM_H
#define EFFECT_SYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

#include <unordered_map>

enum EffectType 				{ TEMPORARY, CONSTANT };

typedef struct 
{
	EffectType Type;

	// Some function pointer here to actually do something...
	void (*Apply)(ECS::eid32 Entity, ECS::Systems::EntityManager* Manager);
	ECS::Component::TimerComponent Timer;
	ECS::eid32 Entity;
} EffectComponent;

struct EffectSystem
{
	ECS::Systems::EntityManager* Manager;

	/* Note(John): This takes too much memory; Might need to change this or roll a more light-weight implementation */
	std::unordered_map<std::string, EffectComponent> TransferredEffects[MAX_ENTITIES];
	// std::unordered_map<std::string, EffectComponent> TransferrableEffects[MAX_ENTITIES];
};


namespace ECS{ namespace Systems { namespace Effect {

	// Constructs and returns new EffectSystem
	struct EffectSystem* NewEffectSystem(Systems::EntityManager* Manager);
	
	// Updates any Attributes Components of entity it is attached to
	void Update(Systems::EntityManager* Manager);

}}}

#endif

