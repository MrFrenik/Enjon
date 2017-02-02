#ifndef ENJON_COLLISIONSYSTEM_H
#define ENJON_COLLISIONSYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

struct CollisionSystem
{
	ECS::Systems::EntityManagerDeprecated* Manager;
	ECS::Component::CollisionComponent CollisionComponents[MAX_ENTITIES];	
	std::vector<Enjon::uint32> Entities;
};

namespace ECS { namespace Systems { namespace Collision {

	// Updates all possible collisions
	void Update(struct EntityManagerDeprecated* Manager);

	// Creates new CollisionSystem
	struct CollisionSystem* NewCollisionSystem(Systems::EntityManagerDeprecated* Manager);		

	// Check collision type  
	Enjon::uint32 GetCollisionType(Systems::EntityManagerDeprecated* Manager, ECS::eid32 A, ECS::eid32 B);

	// Collide Player with Item
	void CollideWithItem(Systems::EntityManagerDeprecated* Manager, ECS::eid32 A, ECS::eid32 B);

	// Collide Player with Enemy
	void CollideWithEnemy(Systems::EntityManagerDeprecated* Manager, ECS::eid32 A, ECS::eid32 B);

	// Collide Projectile with Enemy
	void CollideWithProjectile(Systems::EntityManagerDeprecated* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID);

	// Collide Explosive with Enemy
	void CollideWithExplosive(Systems::EntityManagerDeprecated* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID);

	void CollideWithDebris(Systems::EntityManagerDeprecated* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID);

	void CollideWithVortex(Systems::EntityManagerDeprecated* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID);

	// Totally for testing purposes...
	void DropRandomLoot(Systems::EntityManagerDeprecated* Manager, Enjon::uint32 count, const Enjon::Math::Vec2* Position);
}}}


#endif

