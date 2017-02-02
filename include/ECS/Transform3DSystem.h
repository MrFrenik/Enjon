#ifndef ENJON_TRANSFORM_3D_SYSTEM_H
#define ENJON_TRANSFORM_3D_SYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"

#include <Graphics/ParticleEngine2D.h>

// Forward Declaration
struct EntityManagerDeprecated;

struct Transform3DSystem
{
	ECS::Systems::EntityManagerDeprecated* Manager;
	ECS::Component::Transform3D Transforms[MAX_ENTITIES];
}; 

namespace ECS{ namespace Systems { namespace Transform {

		// Updates Transforms of EntityManagerDeprecated
		void Update(Transform3DSystem* System, Enjon::Graphics::Particle2D::ParticleBatch2D* Batch);

		// Creates new Transform3DSystem
		Transform3DSystem* NewTransform3DSystem(struct EntityManagerDeprecated* Manager);	

		// Resets transform component of entity
		void Reset(struct EntityManagerDeprecated* Manager, eid32 Entity);	
}}}


#endif

