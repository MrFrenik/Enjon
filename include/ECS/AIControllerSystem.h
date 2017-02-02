#ifndef ENJON_AICONTROLLERSYSTEM_H
#define ENJON_AICONTROLLERSYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

#include <AI/PathFinding.h>

namespace ECS { namespace Component { 
	
	typedef struct 
	{
		std::deque<PathFinding::Node> Path;
		eid32 CurrentPathIndex;
		eid32 PathSize;
		EM::Vec4 CurrentCellDimensions;
		EM::Vec2 CellPosition;
		eid32 HasPath;
		float TimeOnNode;
	} PathFindingComponent;

}}

struct AIControllerSystem
{
	ECS::Systems::EntityManagerDeprecated* Manager;
	ECS::Component::AIController AIControllers[MAX_ENTITIES];
	ECS::Component::PathFindingComponent PathFindingComponents[MAX_ENTITIES];
};

namespace ECS { namespace Systems { namespace AIController {

	// Constructs and returns new AIControllerSystem
	// TODO(John): Write custom allocator for this
	struct AIControllerSystem* NewAIControllerSystem(struct EntityManager* Manager);
	
	// Updates Controller of AI it is attached to
	void Update(struct AIControllerSystem* System, eid32 Player);

	void Reset(struct EntityManagerDeprecated* Manager, eid32 Entity);	

	std::deque<PathFinding::Node>* GetPath();
}}}


#endif



