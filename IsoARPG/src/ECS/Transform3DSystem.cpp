
#include "ECS/Transform3DSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/CollisionSystem.h"
#include "ECS/InventorySystem.h"

namespace ECS{ namespace Systems { namespace Transform {

	// Create a new Transform3D Sytem
	Transform3DSystem* NewTransform3DSystem(struct EntityManager* Manager)
	{
		Transform3DSystem* System = new Transform3DSystem;
		if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_TRANSFORM_3D_SYSTEM::System is null"); 
		System->Manager = Manager;
		return System;
	}

	// Updates a Transform3D system
	void Update(Transform3DSystem* System)
	{
		EntityManager* Manager = System->Manager;
		// Look at the entities in the Manager up to the last entered position and then update based on component masks
		for (eid32 e = 0; e <Manager->MaxAvailableID; e++)
		{
			// If equal then transform that entity
			if (Manager->Masks[e] & COMPONENT_TRANSFORM3D)
			{
				// If equipped, then don't update transform here
				if (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::EQUIPPED) continue;

				// Get distance to player
				V2* GP = &Manager->TransformSystem->Transforms[e].GroundPosition;
				V2* PGP = &Manager->TransformSystem->Transforms[Manager->Player].GroundPosition;

				if (PGP->DistanceTo(*GP) >= 5000) continue; // TODO(John): Make this squared distance so as to not use a square root function EVERY frame for EVERY entity

				// If an item
				if ((Manager->AttributeSystem->Masks[e] & Masks::Type::ITEM) && (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::PICKED_UP) == 0)
				{
					// Need to see whether or not the player is within range before I turn on its collision component
					Enjon::Math::Vec3* P = &Manager->TransformSystem->Transforms[e].Position;

					if (PGP->DistanceTo(*GP) <= TILE_SIZE * 2) Manager->CollisionSystem->Entities.push_back(e);

					// Set up GroundPosition
					GP->x = P->x + Manager->TransformSystem->Transforms[e].Dimensions.x / 2.0f - 32.0f; // Tilewidth
					GP->y = P->y - P->z; 
					continue;
				}
				// Push back into collision system
				else if (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::COLLIDABLE) Manager->CollisionSystem->Entities.push_back(e);

				// First transform the velocity by LERPing it
				Component::Transform3D* Transform = &System->Transforms[e];
				float Scale = Transform->VelocityGoalScale; 
				Enjon::Math::Vec3* Velocity = &Transform->Velocity; 
				Enjon::Math::Vec3* VelocityGoal = &Transform->VelocityGoal;
				Enjon::Math::Vec3* Position = &Transform->Position; 
				Enjon::Math::Vec2* GroundPosition = &Transform->GroundPosition; 
				float TileWidth = 32.0f;
				
				Velocity->x = Enjon::Math::Lerp(VelocityGoal->x, Velocity->x, Scale); 
				Velocity->y = Enjon::Math::Lerp(VelocityGoal->y, Velocity->y, Scale); 
				Velocity->z = Enjon::Math::Lerp(VelocityGoal->z, Velocity->z, Scale);
				
				// Set entity's position after interpolating
				Position->x += Velocity->x;
				Position->z += Velocity->z;

				// Clamp z position to BaseHeight
				if (Position->z < Transform->BaseHeight) 
				{
					if (Manager->AttributeSystem->Masks[e] & (Masks::Type::WEAPON | Masks::WeaponOptions::PROJECTILE))
					{
						EntitySystem::RemoveEntity(Manager, e);
					}

					Velocity->z = 0.0f;
					Position->z = Transform->BaseHeight;
					Position->y = GroundPosition->y + Position->z;
				} 
				
				// Set position.y to be a sum of y and z velocities
				Position->y += Velocity->y + Velocity->z; 
				
				// Set up GroundPosition
				GroundPosition->x = Position->x + Transform->Dimensions.x / 2.0f - TileWidth;
				GroundPosition->y = Position->y - Position->z; 

				// Set up CartesianPosition
				Transform->CartesianPosition = Enjon::Math::IsoToCartesian(*GroundPosition);

				// Make sure that position is within bounds of World
				int Width = Manager->Width, Height = Manager->Height;
				bool CollideWithLevel = false;
				if (Transform->CartesianPosition.x < -Width + TileWidth * 2.0f) { Transform->CartesianPosition.x = -Width + TileWidth * 2.0f; Velocity->x *= -1; CollideWithLevel = true; }   
				if (Transform->CartesianPosition.x > -TileWidth) { Transform->CartesianPosition.x = -TileWidth; Velocity->x *= -1; CollideWithLevel = true; }
				if (Transform->CartesianPosition.y > -TileWidth) { Transform->CartesianPosition.y = -TileWidth; Velocity->y *= -1; CollideWithLevel = true; }
				if (Transform->CartesianPosition.y < -Height + TileWidth * 2.0f) { Transform->CartesianPosition.y = -Height + TileWidth * 2.0f; Velocity->y *= -1; CollideWithLevel = true; }

				// Delete projectile for now if it collides with level
				if ((Manager->Types[e] == Component::EntityType::PROJECTILE) && CollideWithLevel)
				{
					printf("EntityAmount before delete: %d\n", Manager->Length);
					EntitySystem::RemoveEntity(Manager, e);
					printf("EntityAmount after delete: %d\n", Manager->Length);
				}

				*GroundPosition = Enjon::Math::CartesianToIso(Transform->CartesianPosition);
				Position->y = GroundPosition->y + Position->z;
				Position->x = GroundPosition->x - Transform->Dimensions.x / 2.0f + TileWidth;

				// Set up AABB
				Enjon::Physics::AABB* AABB = &Manager->TransformSystem->Transforms[e].AABB;
				V2* CP = &Transform->CartesianPosition;
				V2 Min(CP->x, CP->y);
				V2 Max(CP->x + TILE_SIZE, CP->y + TILE_SIZE);
				*AABB = {Min, Max};

				// Go through the items in this entity's inventory and set to this position
				// NOTE(John): Note sure if I like this here... or at all...
				std::vector<eid32>* Items = &Manager->InventorySystem->Inventories[e].Items;
				eid32 WeaponEquipped = Manager->InventorySystem->Inventories[e].WeaponEquipped;
				for (eid32 i : *Items)
				{
					if (i == WeaponEquipped) continue;
					Component::Transform3D* ItemTransform = &System->Transforms[i];
					ItemTransform->Position = *Position;
					ItemTransform->CartesianPosition = Transform->CartesianPosition;
					ItemTransform->GroundPosition = *GroundPosition;
				}

				// Leave if weapon not equipped  
				// NOTE(John): Again, another reason I don't like this being here. If the weapon were processed in the transform loop, it would be skipped until its Transform 
				// 			   Component came online
				if (WeaponEquipped == NULL_ENTITY || Manager->AttributeSystem->Masks[e] & Masks::Type::WEAPON) continue;

				// Calculate equipped weapon Transform
				Component::Transform3D* WeaponTransform = &System->Transforms[WeaponEquipped];
				WeaponTransform->Position = *Position;
				WeaponTransform->GroundPosition = *GroundPosition;
				WeaponTransform->CartesianPosition = Transform->CartesianPosition;

				static float WeaponSize = 64.0f;

				// Calculate AABB
				Enjon::Physics::AABB NE 	= {V2(Min.x + TILE_SIZE / 2.0f, Min.y - WeaponSize / 2.0f), V2(Max.x + WeaponSize, Max.y + WeaponSize / 2.0f)};
				Enjon::Physics::AABB N 		= {V2(Min.x + TILE_SIZE / 2.0f, Min.y + TILE_SIZE / 2.0f), V2(Max.x + WeaponSize, Max.y + WeaponSize)};
				Enjon::Physics::AABB NW 	= {V2(Min.x - WeaponSize / 2.0f, Min.y + TILE_SIZE / 2.0f), V2(Max.x + WeaponSize / 2.0f, Max.y + WeaponSize)};
				Enjon::Physics::AABB W 		= {V2(Min.x - WeaponSize, Min.y + TILE_SIZE / 2.0f), V2(Max.x - TILE_SIZE / 2.0f, Max.y + WeaponSize)};
				Enjon::Physics::AABB SW 	= {V2(Min.x - WeaponSize, Min.y - WeaponSize / 2.0f), V2(Max.x - TILE_SIZE / 2.0f, Max.y + WeaponSize / 2.0f)};
				Enjon::Physics::AABB S 		= {V2(Min.x - WeaponSize, Min.y - WeaponSize), V2(Max.x - TILE_SIZE / 2.0f, Max.y - TILE_SIZE / 2.0f)};
				Enjon::Physics::AABB SE 	= {V2(Min.x - WeaponSize / 2.0f, Min.y - WeaponSize), V2(Max.x + WeaponSize / 2.0f, Max.y - TILE_SIZE / 2.0f)};
				Enjon::Physics::AABB E 		= {V2(Min.x + TILE_SIZE / 2.0f, Min.y - WeaponSize), V2(Max.x + WeaponSize, Max.y - TILE_SIZE / 2.0f)};

				// Get Attack Vector 
				V2* AttackVector = &Transform->AttackVector;

				// Apply AABB to weapon
				if 		(*AttackVector == NORTH) 		WeaponTransform->AABB = N;
				else if (*AttackVector == NORTHEAST) 	WeaponTransform->AABB = NE;
				else if (*AttackVector == NORTHWEST) 	WeaponTransform->AABB = NW;
				else if (*AttackVector == WEST) 		WeaponTransform->AABB = W;
				else if (*AttackVector == SOUTHWEST) 	WeaponTransform->AABB = SW;
				else if (*AttackVector == SOUTH) 		WeaponTransform->AABB = S;
				else if (*AttackVector == SOUTHEAST) 	WeaponTransform->AABB = SE;
				else if (*AttackVector == EAST) 		WeaponTransform->AABB = E;
				else									WeaponTransform->AABB = NW;

				// Push weapon back into collision system
				Manager->CollisionSystem->Entities.push_back(WeaponEquipped);
			}
		}
	}
}}}



