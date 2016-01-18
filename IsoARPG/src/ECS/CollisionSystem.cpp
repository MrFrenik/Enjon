#include "ECS/ComponentSystems.h"
#include "ECS/CollisionSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/InventorySystem.h"
#include "ECS/PlayerControllerSystem.h"
#include "ECS/Renderer2DSystem.h"

#include <Graphics/Camera2D.h>
#include <Math/Random.h>
#include <IO/ResourceManager.h>

namespace ECS{ namespace Systems { namespace Collision {

	// Collision BitMasks
	Enjon::uint32 COLLISION_NONE		= 0x00000000;
	Enjon::uint32 COLLISION_PLAYER		= 0x00000001;
	Enjon::uint32 COLLISION_ENEMY		= 0x00000002;
	Enjon::uint32 COLLISION_ITEM		= 0x00000004;
	Enjon::uint32 COLLISION_PROJECTILE	= 0x00000008;
	Enjon::uint32 COLLISION_WEAPON		= 0x00000010;

	// Creates new CollisionSystem
	struct CollisionSystem* NewCollisionSystem(struct EntityManager* Manager)
	{
		struct CollisionSystem* System = new CollisionSystem;
		if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_COLLISION_SYSTEM::System is null"); 
		System->Manager = Manager;
		return System;
	}		

	// Updates all possible collisions
	void Update(struct EntityManager* Manager)
	{
		static Enjon::uint32 times;
		times = 0; 
		
		eid32 size = Manager->CollisionSystem->Entities.empty() ? 0 : Manager->CollisionSystem->Entities.size();	
		// Check the quadrants of entities and then check for collisions
		for (eid32 n = 0; n < size; n++)
		{
			// Get entity
			eid32 e = Manager->CollisionSystem->Entities[n];

			// if ((Manager->AttributeSystem->Masks[e] & Masks::Type::WEAPON) && (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::PICKED_UP) == 0) continue; 

			// If entity has no transform, then continue
			if (!(Manager->Masks[e] & COMPONENT_TRANSFORM3D)) continue;
			// Get the cell that entity belongs to
			const Enjon::Math::Vec2* EPosition = &Enjon::Math::IsoToCartesian(Manager->TransformSystem->Transforms[e].GroundPosition);
			int CellIndex = SpatialHash::FindCell(Manager->Grid, e, EPosition);

			// Check all entities and neighbors
			std::vector<eid32> Entities = Manager->Grid->cells[CellIndex].entities;
			SpatialHash::GetNeighborCells(Manager->Grid, CellIndex, &Entities);  // Note(John): This is causing too much slowdown

			// Use these same entities for targets if player
			if (Manager->Masks[e] & COMPONENT_PLAYERCONTROLLER)
			{
				std::vector<eid32>* Targets = &Manager->PlayerControllerSystem->Targets;
				for (eid32 it = 0; it < Entities.size(); it++)
				{
					if (Manager->Masks[Entities[it]] & COMPONENT_AICONTROLLER)
					{
						Targets->push_back(Entities[it]);	
					}
				}
				// Targets->insert(Targets->end(), Entities.begin(), Entities.end());
			}

			// TODO(John): Keep a mapping of already checked pairs to cut this time down
			
			// Get entities 
			if (!Entities.empty())
			{
				for (eid32 collider : Entities)
				{
					if (Manager->Masks[collider] & COMPONENT_TRANSFORM3D && collider != e)
					{
						// Get EntityType of collider and entity
						Component::EntityType AType = Manager->Types[collider];
						Component::EntityType BType = Manager->Types[e];

						// Get collision mask for A and B
						Enjon::uint32 Mask = GetCollisionType(Manager, e, collider);

						if (Mask == (COLLISION_ENEMY | COLLISION_ENEMY)) 		{ CollideWithEnemy(Manager, e, collider); 		continue; }
						if (Mask == (COLLISION_WEAPON | COLLISION_ENEMY)) 		{ CollideWithEnemy(Manager, e, collider); 		continue; }
						if (Mask == (COLLISION_PROJECTILE | COLLISION_ENEMY)) 	{ CollideWithProjectile(Manager, e, collider); 	continue; } 
						if (Mask == (COLLISION_ITEM | COLLISION_PLAYER)) 		{ CollideWithItem(Manager, collider, e); 		continue; } 
						if (Mask == (COLLISION_ENEMY | COLLISION_PLAYER)) 		{ CollideWithEnemy(Manager, e, collider); 		continue; }
					}
				}	
			}
		}
		// printf("Times: %d\n", times);
		times = 0;
	} // Collision Update
	

	Enjon::uint32 GetCollisionType(Systems::EntityManager* Manager, ECS::eid32 A, ECS::eid32 B)
	{

		// Init collision mask
		Enjon::uint32 Mask = COLLISION_NONE;

		// Make sure that manager is not null
		if (Manager == nullptr) Enjon::Utils::FatalError("COLLISION_SYSTEM::GET_COLLISION_TYPE::Manager is null");

		// Get types of A and B
		const Component::EntityType* TypeA = &Manager->Types[A];
		const Component::EntityType* TypeB = &Manager->Types[B];

		// Or the mask with TypeA collision
		switch(*TypeA)
		{
			case Component::EntityType::ITEM:			Mask |= COLLISION_ITEM; 			break;
			case Component::EntityType::WEAPON:			Mask |= COLLISION_WEAPON; 			break;
			case Component::EntityType::PLAYER:			Mask |= COLLISION_PLAYER; 			break;
			case Component::EntityType::ENEMY: 			Mask |= COLLISION_ENEMY; 			break;
			case Component::EntityType::PROJECTILE: 	Mask |= COLLISION_PROJECTILE; 		break;
			default: 									Mask |= COLLISION_NONE;				break; 
		}	

		// // Or the mask with TypeB collision
		switch(*TypeB)
		{
			case Component::EntityType::ITEM:			Mask |= COLLISION_ITEM; 			break;
			case Component::EntityType::WEAPON:			Mask |= COLLISION_WEAPON; 			break;
			case Component::EntityType::PLAYER:			Mask |= COLLISION_PLAYER; 			break;
			case Component::EntityType::ENEMY: 			Mask |= COLLISION_ENEMY; 			break;
			case Component::EntityType::PROJECTILE: 	Mask |= COLLISION_PROJECTILE; 		break;
			default: 									Mask |= COLLISION_NONE;				break; 
		}

		return Mask;	
	}

	void CollideWithProjectile(Systems::EntityManager* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID)
	{
		Enjon::Math::Vec3* EntityPosition = &Manager->TransformSystem->Transforms[A_ID].Position;
		Enjon::Math::Vec3* ColliderPosition = &Manager->TransformSystem->Transforms[B_ID].Position;
		Enjon::Math::Vec2* A = &Manager->TransformSystem->Transforms[B_ID].CartesianPosition;
		Enjon::Math::Vec2* B = &Manager->TransformSystem->Transforms[A_ID].CartesianPosition;
		Enjon::Math::Vec3* ColliderVelocity = &Manager->TransformSystem->Transforms[B_ID].Velocity; 
		Enjon::Physics::AABB* AABB_A = &Manager->TransformSystem->Transforms[A_ID].AABB;
		Enjon::Physics::AABB* AABB_B = &Manager->TransformSystem->Transforms[B_ID].AABB;

		// Collision didn't happen
		if (!Enjon::Physics::AABBvsAABB(AABB_A, AABB_B)) return;
		
		else 
		{
			// Shake the camera for effect
			Manager->Camera->ShakeScreen(Enjon::Random::Roll(10, 15));

			// Get minimum translation distance
			V2 mtd = Enjon::Physics::MinimumTranslation(AABB_A, AABB_B);

			// Update velocities based on "bounce" factor
			float bf = 1.0f; // Bounce factor 
			ColliderVelocity->x = -ColliderVelocity->x * bf;
			ColliderVelocity->y = -ColliderVelocity->y * bf;

			// Hurt Collider
			// Get health and color of entity
			Component::HealthComponent* HealthComponent = &Manager->AttributeSystem->HealthComponents[B_ID];
			Enjon::Graphics::ColorRGBA8* Color = &Manager->Renderer2DSystem->Renderers[B_ID].Color;

			// Set option to damaged
			Manager->AttributeSystem->Masks[B_ID] |= Masks::GeneralOptions::DAMAGED;	

			if (HealthComponent == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Collider health component is null");
			if (Color == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Color component is null");
	
			// Decrement by some arbitrary amount for now	
			HealthComponent->Health -= 10.0f;

			if (HealthComponent->Health <= 0.0f) 
			{
				// Remove entity if no health
				EntitySystem::RemoveEntity(Manager, B_ID);

				// Drop random loot
				if (Enjon::Random::Roll(0, 10) >= 5) Collision::DropRandomLoot(Manager, 3, &ColliderPosition->XY());
			}

			// Remove projectile
			EntitySystem::RemoveEntity(Manager, A_ID);

			// Continue with next entity
			return;
		}

	}

	void CollideWithItem(Systems::EntityManager* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID)
	{
		Component::EntityType AType = Manager->Types[A_ID];
		Component::EntityType BType = Manager->Types[B_ID];

		Enjon::Math::Vec3* EntityPosition = &Manager->TransformSystem->Transforms[A_ID].Position;
		Enjon::Math::Vec3* ColliderPosition = &Manager->TransformSystem->Transforms[B_ID].Position;
		Enjon::Math::Vec2* A = &Manager->TransformSystem->Transforms[A_ID].CartesianPosition;
		Enjon::Math::Vec2* B = &Manager->TransformSystem->Transforms[B_ID].CartesianPosition;
		Enjon::Physics::AABB* AABB_A = &Manager->TransformSystem->Transforms[A_ID].AABB;
		Enjon::Physics::AABB* AABB_B = &Manager->TransformSystem->Transforms[B_ID].AABB;

		// Collision didn't happen
		if (!Enjon::Physics::AABBvsAABB(AABB_A, AABB_B)) return;
			
		// Picking up an item
		else
		{
			eid32 Item 	= AType == Component::EntityType::ITEM ? A_ID : B_ID;
			eid32 Player = Item == A_ID ? B_ID : A_ID;
			
			if (Manager->InventorySystem->Inventories[Player].Items.size() < MAX_ITEMS)
			{
				printf("Picked up item!\n");
			
				// Place in player inventory
				Manager->InventorySystem->Inventories[Player].Items.push_back(Item);

				printf("Inventory Size: %d\n", Manager->InventorySystem->Inventories[Player].Items.size());
				
				// Turn off render and transform components of item
				EntitySystem::RemoveComponents(Manager, Item, COMPONENT_RENDERER2D | COMPONENT_TRANSFORM3D);

				// Set item to picked up
				Manager->AttributeSystem->Masks[Item] |= Masks::GeneralOptions::PICKED_UP;
			}
			else 
			{
				printf("Inventory already full!\n");
			}

			// Continue to next entity 	
			return;
		}
	}

	// Collide Player with Enemy
	void CollideWithEnemy(Systems::EntityManager* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID)
	{
		Enjon::Math::Vec3* EntityPosition = &Manager->TransformSystem->Transforms[A_ID].Position;
		Enjon::Math::Vec3* ColliderPosition = &Manager->TransformSystem->Transforms[B_ID].Position;
		Enjon::Math::Vec2* A = &Manager->TransformSystem->Transforms[B_ID].CartesianPosition;
		Enjon::Math::Vec2* B = &Manager->TransformSystem->Transforms[A_ID].CartesianPosition;
		Enjon::Math::Vec3* EntityVelocity = &Manager->TransformSystem->Transforms[A_ID].Velocity; 
		Enjon::Math::Vec3* ColliderVelocity = &Manager->TransformSystem->Transforms[B_ID].Velocity; 
		Enjon::Physics::AABB* AABB_A = &Manager->TransformSystem->Transforms[A_ID].AABB;
		Enjon::Physics::AABB* AABB_B = &Manager->TransformSystem->Transforms[B_ID].AABB;

		// Collision didn't happen
		if (!Enjon::Physics::AABBvsAABB(AABB_A, AABB_B)) { return; }
		
		else
		{
			// Get minimum translation distance
			V2 mtd = Enjon::Physics::MinimumTranslation(AABB_B, AABB_A);

			*EntityPosition -= Enjon::Math::Vec3(Enjon::Math::CartesianToIso(mtd), ColliderPosition->z); 

			// Update velocities based on "bounce" factor
			float bf; // Bounce factor 
			if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
			{
				// Shake the camera for effect
				Manager->Camera->ShakeScreen(Enjon::Random::Roll(10, 15));
	
				bf = 1.2f;
			}

			else bf = 1.0f;

			EntityVelocity->x = -EntityVelocity->x * bf; 
			EntityVelocity->y = -EntityVelocity->y * bf;
			ColliderVelocity->x = -ColliderVelocity->x * bf;
			ColliderVelocity->y = -ColliderVelocity->y * bf;


			if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
			{
				// Get health and color of entity
				Component::HealthComponent* HealthComponent = &Manager->AttributeSystem->HealthComponents[B_ID];
				Enjon::Graphics::ColorRGBA8* Color = &Manager->Renderer2DSystem->Renderers[B_ID].Color;

				// Set option to damaged
				Manager->AttributeSystem->Masks[B_ID] |= Masks::GeneralOptions::DAMAGED;	

				if (HealthComponent == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Collider health component is null");
				if (Color == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Color component is null");
		
				// Decrement by some arbitrary amount for now	
				HealthComponent->Health -= Enjon::Random::Roll(10.0f, 20.0f);


				// If dead, then kill it	
				if (HealthComponent->Health <= 0.0f)
				{
					// Remove collider
					EntitySystem::RemoveEntity(Manager, B_ID);
					
					// Drop some loot!
					Collision::DropRandomLoot(Manager, 5, &ColliderPosition->XY());
				}
			}

			// Continue with next entity
			return;
		}
	}


	void DropRandomLoot(Systems::EntityManager* Manager, Enjon::uint32 count, const Enjon::Math::Vec2* Position)
	{
		static Enjon::Graphics::SpriteSheet ItemSheet; 
		if (!ItemSheet.IsInit()) ItemSheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Enjon::Math::iVec2(1, 1));

		for (int i = 0; i < count; i++)
		{
			int Roll = Enjon::Random::Roll(0, 5);

			Enjon::Graphics::ColorRGBA8 ItemColor;

			if (Roll == 0) ItemColor = Enjon::Graphics::RGBA8_Red();
			if (Roll == 1) ItemColor = Enjon::Graphics::RGBA8_Orange();
			if (Roll == 2) ItemColor = Enjon::Graphics::RGBA8_Blue();
			if (Roll == 3) ItemColor = Enjon::Graphics::RGBA8_Green();
			if (Roll == 4) ItemColor = Enjon::Graphics::RGBA8_Yellow();
			if (Roll == 5) ItemColor = Enjon::Graphics::RGBA8_Magenta();

			eid32 id = EntitySystem::CreateItem(Manager, Enjon::Math::Vec3(Enjon::Random::Roll(Position->x - 64.0f, Position->x + 64.0f), 
												  Enjon::Random::Roll(Position->y - 64.0f, Position->y + 64.0f), 0.0f), 
												  Enjon::Math::Vec2(16.0f, 16.0f), &ItemSheet, (Masks::Type::ITEM | Masks::ItemOptions::CONSUMABLE), 
												  Component::EntityType::ITEM, "Item", Enjon::Graphics::SetOpacity(ItemColor, 0.5f));
			Manager->TransformSystem->Transforms[id].VelocityGoal.x = 0.0f;
			Manager->TransformSystem->Transforms[id].VelocityGoal.y = 0.0f;
			Manager->TransformSystem->Transforms[id].Velocity = {0.0f, 0.0f, 0.0f};
		} 

	}

}}}



