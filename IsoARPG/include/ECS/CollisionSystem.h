#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

struct CollisionSystem
{
	ECS::Systems::EntityManager* Manager;
	std::vector<Enjon::uint32> Entities;
};

namespace ECS { namespace Systems { namespace Collision {

	// Updates all possible collisions
	void Update(struct EntityManager* Manager);

	// Creates new CollisionSystem
	struct CollisionSystem* NewCollisionSystem(Systems::EntityManager* Manager);		

	// Check collision type  
	Enjon::uint32 GetCollisionType(Systems::EntityManager* Manager, ECS::eid32 A, ECS::eid32 B);

	// Collide Player with Item
	void CollideWithItem(Systems::EntityManager* Manager, ECS::eid32 A, ECS::eid32 B);

	// Collide Player with Enemy
	void CollideWithEnemy(Systems::EntityManager* Manager, ECS::eid32 A, ECS::eid32 B);

	// Collide Projectile with Enemy
	void CollideWithProjectile(Systems::EntityManager* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID);

	// Totally for testing purposes...
	void DropRandomLoot(Systems::EntityManager* Manager, Enjon::uint32 count, const Enjon::Math::Vec2* Position);
}}}


#endif

