#ifndef TRANSFORM3DSYSTEM_H
#define TRANSFORM3DSYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

struct Transform3DSystem
{
	ECS::Systems::EntityManager* Manager;
	ECS::Component::Transform3D Transforms[MAX_ENTITIES];
}; 

namespace ECS{ namespace Systems { namespace Transform {

		// Updates Transforms of EntityManager
		void Update(Transform3DSystem* System);

		// Creates new Transform3DSystem
		Transform3DSystem* NewTransform3DSystem(Systems::EntityManager* Manager);		
}}}


#endif

