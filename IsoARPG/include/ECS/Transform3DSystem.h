#ifndef TRANSFORM3DSYSTEM_H
#define TRANSFORM3DSYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

#include <Graphics/ParticleEngine2D.h>

struct Transform3DSystem
{
	ECS::Systems::EntityManager* Manager;
	ECS::Component::Transform3D Transforms[MAX_ENTITIES];
}; 

namespace ECS{ namespace Systems { namespace Transform {

		// Updates Transforms of EntityManager
		void Update(Transform3DSystem* System, Enjon::Graphics::Particle2D::ParticleBatch2D* Batch);

		// Creates new Transform3DSystem
		Transform3DSystem* NewTransform3DSystem(Systems::EntityManager* Manager);		
}}}


#endif

