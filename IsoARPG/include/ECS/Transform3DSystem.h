#ifndef TRANSFORM3DSYSTEM_H
#define TRANSFORM3DSYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"

#include <Graphics/ParticleEngine2D.h>

// Forward Declaration
struct EntityManager;

struct Transform3DSystem
{
	ECS::Systems::EntityManager* Manager;
	ECS::Component::Transform3D Transforms[MAX_ENTITIES];
}; 

namespace ECS{ namespace Systems { namespace Transform {

		// Updates Transforms of EntityManager
		void Update(Transform3DSystem* System, Enjon::Graphics::Particle2D::ParticleBatch2D* Batch);

		// Creates new Transform3DSystem
		Transform3DSystem* NewTransform3DSystem(struct EntityManager* Manager);	

		// Resets transform component of entity
		void Reset(struct EntityManager* Manager, eid32 Entity);	
}}}


#endif

