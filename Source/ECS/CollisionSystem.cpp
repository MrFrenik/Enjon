#include "ECS/ComponentSystems.h"
#include "ECS/CollisionSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/InventorySystem.h"
#include "ECS/AIControllerSystem.h"
#include "ECS/PlayerControllerSystem.h"
#include "ECS/Renderer2DSystem.h"
#include "ECS/EffectSystem.h"
#include "ECS/Effects.h"
#include "ECS/EntityFactory.h"

#include "Level.h"

#include <Graphics/Camera2D.h>
#include <Graphics/ParticleEngine2D.h>
#include <Graphics/SpriteSheetManager.h>
#include <Graphics/FontManager.h>
#include <Graphics/Color.h>
#include <Defines.h>
#include <System/Types.h>
#include <Math/Random.h>
#include <IO/ResourceManager.h>

#include <SDL2/SDL.h>
#include <time.h>

#include <unordered_set>
#include <limits.h>

#define MAX_COLLISION_INDICIES 

namespace ECS{ namespace Systems { namespace Collision {

	// Collision BitMasks
	Enjon::uint32 COLLISION_NONE		= 0x00000000;
	Enjon::uint32 COLLISION_PLAYER		= 0x00000001;
	Enjon::uint32 COLLISION_ENEMY		= 0x00000002;
	Enjon::uint32 COLLISION_ITEM		= 0x00000004;
	Enjon::uint32 COLLISION_PROJECTILE	= 0x00000008;
	Enjon::uint32 COLLISION_WEAPON		= 0x00000010;
	Enjon::uint32 COLLISION_EXPLOSIVE	= 0x00000020;
	Enjon::uint32 COLLISION_PROP		= 0x00000040;
	Enjon::uint32 COLLISION_VORTEX  	= 0x00000080;

	auto NumberOfCollisionLoops = 0;
	std::unordered_map<std::string, Enjon::uint32> CollisionPairsWithLastFrameUpdate;
	std::unordered_set<const char*> CollisionPairs;   

	/*-- Function Declarations --*/
	void DrawBlood(ECS::Systems::EntityManager* Manager, Enjon::Math::Vec2 Pos);
	void DrawBody(ECS::Systems::EntityManager* Manager, Enjon::Math::Vec2 Pos);

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
		static Enjon::uint32 StartTime = 0;
		static Enjon::uint32 GetCellsTime = 0;
		static Enjon::uint32 CollisionCheckTime = 0;

		auto NumberOfChecks = 0;
		
		// TODO(John): This is way too slow to be creating and destroying a set each frame / creating something once and update its values each frame instead,
		// 				preferably something that has an O(1) lookup time	

		// This could really be unecessary writing...
		eid32 size = Manager->CollisionSystem->Entities.empty() ? 0 : Manager->CollisionSystem->Entities.size();	
		
		for (eid32 n = 0; n < size; n++)
		{
	
			// Get entity
			eid32 e = Manager->CollisionSystem->Entities[n];

			// Get entities
			std::vector<eid32> Entities;
			// if (Manager->AttributeSystem->Masks[e] & Masks::Type::WEAPON) Entities = SpatialHash::FindCell(Manager->Grid, e, &Manager->TransformSystem->Transforms[e].AABB);
			Entities = SpatialHash::FindCell(Manager->Grid, e, &Manager->TransformSystem->Transforms[e].AABB);
			// else 														  Entities = SpatialHash::GetEntitiesFromCells(Manager->Grid, Manager->CollisionSystem->CollisionComponents[e].Cells);
		
			// Collide with entities 
			for (eid32 collider : Entities)
			{
				bool FoundCollisionPair = false;
				bool NeedToUpdateCollisionPairFrame = false;
				std::string HashString;
	
				if (e == collider) continue;
				if (e > collider) 	HashString = std::to_string(e) + std::to_string(collider);
				else 				HashString = std::to_string(collider) + std::to_string(e);

				// This will cause unnecessary branching. Figure out a way to take it out of the loop.
				// if (CollisionPairs.find(HashString.c_str()) == CollisionPairs.end())

				// Search for key
				auto CollisionPairIterator = CollisionPairsWithLastFrameUpdate.find(HashString);

				// Set whether or not found the pair
				FoundCollisionPair = CollisionPairIterator != CollisionPairsWithLastFrameUpdate.end();

				// If found, determine whether or not this frame matches the one needing to be checked
				if (FoundCollisionPair) NeedToUpdateCollisionPairFrame = CollisionPairIterator->second != NumberOfCollisionLoops;

				if (!FoundCollisionPair || NeedToUpdateCollisionPairFrame)
				{
					// Increment number of checks
					NumberOfChecks++;

					// Hash and put back into pair
					// CollisionPairs.insert(HashString.c_str());

					// Update or insert new pair into map with current collision loop as its value
					CollisionPairsWithLastFrameUpdate[HashString] = NumberOfCollisionLoops;

					// Get EntityType of collider and entity
					Component::EntityType AType = Manager->Types[collider];
					Component::EntityType BType = Manager->Types[e];

					// Get collision mask for A and B
					Enjon::uint32 Mask = GetCollisionType(Manager, e, collider);


					// NOTE(John): I can only imagine how much branching this causes... Ugh.
					if (Mask == (COLLISION_ITEM | COLLISION_ITEM)) 			{ 
																				// CollideWithDebris(Manager, collider, e);		continue; 
																				continue; 
																			}
					if (Mask == (COLLISION_ENEMY | COLLISION_ENEMY)) 		{ 
																				CollideWithEnemy(Manager, e, collider); 		continue; 
																				// continue;
																			}
					if (Mask == (COLLISION_WEAPON | COLLISION_ENEMY)) 		{ 
																				if (AType == Component::EntityType::ENEMY)		CollideWithEnemy(Manager, e, collider); 	
																				else 											CollideWithEnemy(Manager, collider, e);
																				continue; 
																			}
					if (Mask == (COLLISION_PROJECTILE | COLLISION_ENEMY)) 	{ 
																				if (AType == Component::EntityType::PROJECTILE) 	CollideWithProjectile(Manager, collider, e); 	
																		      	else 										    	CollideWithProjectile(Manager, e, collider); 
																		      	continue;
																		  	} 
																		  	
					if (Mask == (COLLISION_ITEM | COLLISION_PLAYER)) 		{ 
																				if (AType == Component::EntityType::ITEM) 	CollideWithDebris(Manager, collider, e); 		
																			  	else 										CollideWithDebris(Manager, e, collider); 
																			  	continue; 
																			} 

					if (Mask == (COLLISION_ITEM | COLLISION_ENEMY)) 		{ 
																				if (AType == Component::EntityType::ITEM) 	CollideWithDebris(Manager, collider, e); 		
																			  	else										CollideWithDebris(Manager, e, collider); 
																			  	continue; 
																			} 

					if (Mask == (COLLISION_ITEM | COLLISION_EXPLOSIVE))		{ 
																				if (AType == Component::EntityType::ITEM) 	CollideWithDebris(Manager, collider, e);
																			  	else 										CollideWithDebris(Manager, e, collider); 		
																			  	continue; 
																			} 

					if (Mask == (COLLISION_ITEM | COLLISION_VORTEX))		{ 
																				if (AType == Component::EntityType::ITEM) 	CollideWithVortex(Manager, collider, e);
																			  	else										CollideWithVortex(Manager, e, collider); 		
																			  	continue; 
																			} 

					if (Mask == (COLLISION_PROJECTILE | COLLISION_VORTEX))	{ 
																				if (AType == Component::EntityType::PROJECTILE) 	CollideWithVortex(Manager, collider, e);
																			  	else 												CollideWithVortex(Manager, e, collider); 		
																			  	continue; 
																			} 

					if (Mask == (COLLISION_PROJECTILE | COLLISION_EXPLOSIVE)){ 
																				if (AType == Component::EntityType::PROJECTILE) 	CollideWithDebris(Manager, collider, e);
																			  	else 												CollideWithDebris(Manager, e, collider); 		
																			  	continue; 
																			} 

					if (Mask == (COLLISION_PROJECTILE | COLLISION_WEAPON))	{ 
																				if (AType == Component::EntityType::PROJECTILE) 	CollideWithDebris(Manager, collider, e);
																			  	else 												CollideWithDebris(Manager, e, collider); 		
																			  	continue; 
																			} 

					if (Mask == (COLLISION_ENEMY | COLLISION_PLAYER)) 		{ 
																				if (AType == Component::EntityType::PLAYER) 	CollideWithEnemy(Manager, e, collider); 		
																				else 											CollideWithEnemy(Manager, collider, e);
																				continue; 
																			}

					if (Mask == (COLLISION_EXPLOSIVE | COLLISION_ENEMY))	{ 
																				if (AType == Component::EntityType::ENEMY) 	CollideWithExplosive(Manager, e, collider);  	
																			  	else										CollideWithExplosive(Manager, collider, e); 
																			  	continue; 
																			}
				}
			}	
		}

		static auto t = 0.0f;
		t += 0.1f;
		if (t > 10.0f)
		{
			std::cout << "Number Of Collision pairs: " << CollisionPairsWithLastFrameUpdate.size() << std::endl;
			// std::cout << "Number Of Collision pairs: " << CollisionPairs.size() << std::endl;
			t = 0.0f;
		}

		if (CollisionPairsWithLastFrameUpdate.size() > 100000)
		{
			CollisionPairsWithLastFrameUpdate.clear();
		}

		// CollisionPairs.clear();

		// Increment number of collision loops
		NumberOfCollisionLoops = (NumberOfCollisionLoops + 1) % UINT_MAX;


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
			case Component::EntityType::EXPLOSIVE: 		Mask |= COLLISION_EXPLOSIVE; 		break;
			case Component::EntityType::PROP: 			Mask |= COLLISION_PROP; 			break;
			case Component::EntityType::VORTEX: 		Mask |= COLLISION_VORTEX;			break;
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
			case Component::EntityType::EXPLOSIVE: 		Mask |= COLLISION_EXPLOSIVE; 		break;
			case Component::EntityType::PROP: 			Mask |= COLLISION_PROP; 			break;
			case Component::EntityType::VORTEX: 		Mask |= COLLISION_VORTEX;			break;
			default: 									Mask |= COLLISION_NONE;				break; 
		}

		return Mask;	
	}

	// Collide Explosive with Enemy
	void CollideWithExplosive(Systems::EntityManager* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID)
	{
		Enjon::Math::Vec3* EntityPosition = &Manager->TransformSystem->Transforms[A_ID].Position;
		Enjon::Math::Vec3* ColliderPosition = &Manager->TransformSystem->Transforms[B_ID].Position;
		Enjon::Math::Vec2* A = &Manager->TransformSystem->Transforms[B_ID].CartesianPosition;
		Enjon::Math::Vec2* B = &Manager->TransformSystem->Transforms[A_ID].CartesianPosition;
		Enjon::Math::Vec3* ColliderVelocity = &Manager->TransformSystem->Transforms[B_ID].Velocity; 
		Enjon::Math::Vec3* EntityVelocity = &Manager->TransformSystem->Transforms[A_ID].Velocity; 
		Enjon::Physics::AABB* AABB_A = &Manager->TransformSystem->Transforms[A_ID].AABB;
		Enjon::Physics::AABB* AABB_B = &Manager->TransformSystem->Transforms[B_ID].AABB;

		// Collision didn't happen
		if (!Enjon::Physics::AABBvsAABB(AABB_A, AABB_B)) return;
		
		else 
		{
			// Shake the camera for effect
			Manager->Camera->ShakeScreen(Enjon::Random::Roll(30, 40));

			// Get minimum translation distance
			V2 mtd = Enjon::Physics::MinimumTranslation(AABB_A, AABB_B);

			*EntityVelocity = EM::Vec3(EM::CartesianToIso(mtd), EntityVelocity->z);

			Enjon::Math::Vec2 Difference = Enjon::Math::Vec2::Normalize(EntityPosition->XY() - ColliderPosition->XY());

			// if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
			// 	*ColliderPosition -= Enjon::Math::Vec3(Difference * 30.0f, 0.0f);
			*ColliderVelocity = -2.0f * Enjon::Math::Vec3(Difference, 0.0f);

			// Update velocities based on "bounce" factor
			// float bf = 1.0f; // Bounce factor 
			// ColliderVelocity->x = -ColliderVelocity->x * bf;
			// ColliderVelocity->y = -ColliderVelocity->y * bf;

			// Hurt Collider
			// Get health and color of entity
			Component::HealthComponent* HealthComponent = &Manager->AttributeSystem->HealthComponents[B_ID];
			Enjon::Graphics::ColorRGBA16* Color = &Manager->Renderer2DSystem->Renderers[B_ID].Color;

			// Set option to damaged
			Manager->AttributeSystem->Masks[B_ID] |= Masks::GeneralOptions::DAMAGED;	

			if (HealthComponent == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Collider health component is null");
			if (Color == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Color component is null");

			// Add blood particle effect (totally a test)...

			const EM::Vec3* PP = &Manager->TransformSystem->Transforms[B_ID].Position;
			static GLuint PTex = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/orb.png").id;

			EG::ColorRGBA16 R = EG::RGBA16(1.0f, 0.01f, 0.01f, 1.0f);

			// Add 100 at a time
			// for (Enjon::uint32 i = 0; i < 2; i++)
			// {
				float XPos = Enjon::Random::Roll(-50, 100), YPos = Enjon::Random::Roll(-50, 100), ZVel = Enjon::Random::Roll(-10, 10), XVel = Enjon::Random::Roll(-10, 10), 
								YSize = Enjon::Random::Roll(2, 7), XSize = Enjon::Random::Roll(1, 5);

				EG::Particle2D::AddParticle(EM::Vec3(PP->x + 50.0f + XVel, PP->y + 50.0f + ZVel, 0.0f), EM::Vec3(XVel, XVel, ZVel), 
					EM::Vec2(XSize, YSize), R, PTex, 0.05f, Manager->ParticleEngine->ParticleBatches[0]);

				// Add blood overlay to level
				DrawBlood(Manager, ColliderPosition->XY());
			// }

			// Get min and max damage of weapon
			const Loot::Weapon::WeaponProfile* WP = Manager->AttributeSystem->WeaponProfiles[A_ID];

			Enjon::uint32 MiD, MaD;
			if (WP)
			{
				MiD = WP->Damage.Min;
				MaD = WP->Damage.Max;
			}
			else 
			{
				MiD = 5;
				MaD = 10;
			}

			auto Damage = Enjon::Random::Roll(MiD, MaD);
	
			// Decrement by damage	
			HealthComponent->Health -= Damage;

			// printf("Hit for %d damage\n", Damage);

			if (HealthComponent->Health <= 0.0f) 
			{
				// DrawBody(Manager, ColliderPosition->XY());

				// Remove entity if no health
				EntitySystem::RemoveEntity(Manager, B_ID);

				// Drop some loot!
				// Loot::DropLootBasedOnProfile(Manager, B_ID);

				// auto* LP = Manager->AttributeSystem->LootProfiles[B_ID];
			}

			// Remove projectile
			// EntitySystem::RemoveEntity(Manager, A_ID);

			// Continue with next entity
			return;
		}

	}

	void CollideWithProjectile(Systems::EntityManager* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID)
	{
		Enjon::Math::Vec3* EntityPosition = &Manager->TransformSystem->Transforms[A_ID].Position;
		Enjon::Math::Vec3* ColliderPosition = &Manager->TransformSystem->Transforms[B_ID].Position;
		Enjon::Math::Vec2* A = &Manager->TransformSystem->Transforms[B_ID].CartesianPosition;
		Enjon::Math::Vec2* B = &Manager->TransformSystem->Transforms[A_ID].CartesianPosition;
		Enjon::Math::Vec3* EntityVelocity = &Manager->TransformSystem->Transforms[A_ID].Velocity;
		Enjon::Math::Vec3* ColliderVelocity = &Manager->TransformSystem->Transforms[B_ID].Velocity; 
		Enjon::Physics::AABB* AABB_A = &Manager->TransformSystem->Transforms[A_ID].AABB;
		Enjon::Physics::AABB* AABB_B = &Manager->TransformSystem->Transforms[B_ID].AABB;

		// If parent, then return
		if (Manager->AttributeSystem->Groups[A_ID].Parent == B_ID) return;

		// Collision didn't happen
		if (!Enjon::Physics::AABBvsAABB(AABB_A, AABB_B)) return;
		
		else 
		{
			// If is a grenade, then spawn an explosion
			if (Manager->AttributeSystem->Masks[A_ID] & Masks::WeaponSubOptions::GRENADE)
			{
				// TODO(John): Make a "spawn" function that gets called for any entity that has a factory component
				ECS::eid32 Explosion = Factory::CreateWeapon(Manager, Enjon::Math::Vec3(Manager->TransformSystem->Transforms[A_ID].Position.XY(), 0.0f), Enjon::Math::Vec2(16.0f, 16.0f), 
															Enjon::Graphics::SpriteSheetManager::GetSpriteSheet("Orb"), 
															Masks::Type::WEAPON | Masks::WeaponOptions::EXPLOSIVE, Component::EntityType::EXPLOSIVE, "Explosion");

				Manager->AttributeSystem->Masks[Explosion] |= Masks::GeneralOptions::COLLIDABLE;

				Manager->TransformSystem->Transforms[Explosion].Velocity = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
				Manager->TransformSystem->Transforms[Explosion].VelocityGoal = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
				Manager->TransformSystem->Transforms[Explosion].BaseHeight = 0.0f;
				Manager->TransformSystem->Transforms[Explosion].MaxHeight = 0.0f;

				Manager->TransformSystem->Transforms[Explosion].AABBPadding = Enjon::Math::Vec2(200, 200);

				Enjon::Graphics::Particle2D::DrawFire(Manager->ParticleEngine->ParticleBatches[0], Manager->TransformSystem->Transforms[A_ID].Position);

				auto I = Enjon::Random::Roll(0, 2);
				Enjon::Graphics::GLTexture S;
				switch(I)
				{
					case 0: S = Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/explody.png"); break;
					case 1: S = Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/explody_2.png"); break;
					case 2: S = Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/explody_3.png"); break;
					default: break;
				}
				auto Position = &Manager->TransformSystem->Transforms[A_ID].Position;
				auto alpha = Enjon::Random::Roll(50, 255) / 255.0f;
				auto X = (float)Enjon::Random::Roll(-50, 100);
				auto Y = (float)Enjon::Random::Roll(-100, 50);
				auto C = Enjon::Graphics::RGBA16_White();
				auto DC = Enjon::Random::Roll(80, 100) / 255.0f;
				C = Enjon::Graphics::RGBA16(C.r - DC, C.g - DC, C.b - DC, alpha);
				Manager->Lvl->AddTileOverlay(S, Enjon::Math::Vec4(Position->x + X, Position->y + Y, (float)Enjon::Random::Roll(50, 100), (float)Enjon::Random::Roll(50, 100)), C);
			}
				
			// Shake the camera for effect
			if (Manager->AttributeSystem->Masks[A_ID] & Masks::WeaponOptions::EXPLOSIVE) Manager->Camera->ShakeScreen(Enjon::Random::Roll(30, 40));
			else Manager->Camera->ShakeScreen(Enjon::Random::Roll(10, 15));

			// Get minimum translation distance
			V2 mtd = Enjon::Physics::MinimumTranslation(AABB_A, AABB_B);

			*EntityVelocity = EM::Vec3(EM::CartesianToIso(mtd), EntityVelocity->z);
			// *EntityPosition -= Enjon::Math::Vec3(Enjon::Math::CartesianToIso(mtd), EntityPosition->z); 
			// Manager->TransformSystem->Transforms[A_ID].GroundPosition -= Enjon::Math::CartesianToIso(mtd); 

			// if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
			// 	*ColliderPosition += Enjon::Math::Vec3(Enjon::Math::CartesianToIso(mtd) * 1.0f, 0.0f);
			Enjon::Math::Vec2 Difference = Enjon::Math::Vec2::Normalize(EntityPosition->XY() - ColliderPosition->XY());

			// if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
			// 	*ColliderPosition -= Enjon::Math::Vec3(Difference * 30.0f, 0.0f);
			if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
				*ColliderVelocity = -2.0f * Enjon::Math::Vec3(Difference, 0.0f);

			// Update velocities based on "bounce" factor
			// float bf = 1.0f; // Bounce factor 
			// ColliderVelocity->x = -ColliderVelocity->x * bf;
			// ColliderVelocity->y = -ColliderVelocity->y * bf;

			// Hurt Collider
			// Get health and color of entity
			Component::HealthComponent* HealthComponent = &Manager->AttributeSystem->HealthComponents[B_ID];
			Enjon::Graphics::ColorRGBA16* Color = &Manager->Renderer2DSystem->Renderers[B_ID].Color;

			// Set option to damaged
			Manager->AttributeSystem->Masks[B_ID] |= Masks::GeneralOptions::DAMAGED;	

			if (HealthComponent == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Collider health component is null");
			if (Color == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Color component is null");

			// Add blood particle effect (totally a test)...

			const EM::Vec3* PP = &Manager->TransformSystem->Transforms[B_ID].Position;
			static GLuint PTex = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/orb.png").id;

			EG::ColorRGBA16 R = EG::RGBA16(1.0f, 0.01f, 0.01f, 1.0f);

			// Add 100 at a time
			// for (Enjon::uint32 i = 0; i < 2; i++)
			// {
				float XPos = Enjon::Random::Roll(-50, 100), YPos = Enjon::Random::Roll(-50, 100), ZVel = Enjon::Random::Roll(-10, 10), XVel = Enjon::Random::Roll(-10, 10), 
								YSize = Enjon::Random::Roll(2, 7), XSize = Enjon::Random::Roll(1, 5);

				EG::Particle2D::AddParticle(EM::Vec3(PP->x + 50.0f + XVel, PP->y + 50.0f + ZVel, 0.0f), EM::Vec3(XVel, XVel, ZVel), 
					EM::Vec2(XSize, YSize), R, PTex, 0.05f, Manager->ParticleEngine->ParticleBatches[0]);

				// Add blood overlay to level
				DrawBlood(Manager, ColliderPosition->XY());
			// }

			// Get min and max damage of weapon
			const Loot::Weapon::WeaponProfile* WP = Manager->AttributeSystem->WeaponProfiles[A_ID];

			Enjon::uint32 MiD, MaD;
			if (WP)
			{
				MiD = WP->Damage.Min;
				MaD = WP->Damage.Max;
			}
			else 
			{
				MiD = 5;
				MaD = 10;
			}

			auto Damage = Enjon::Random::Roll(MiD, MaD);
	
			// Decrement by damage	
			HealthComponent->Health -= Damage;

			// printf("Hit for %d damage\n", Damage);

			if (HealthComponent->Health <= 0.0f) 
			{
				//DrawBody(Manager, ColliderPosition->XY());

				// Remove entity if no health
				EntitySystem::RemoveEntity(Manager, B_ID);

				// Drop some loot!
				// Loot::DropLootBasedOnProfile(Manager, B_ID);

				// auto* LP = Manager->AttributeSystem->LootProfiles[B_ID];
			}
			else
			{
				if ((Manager->Masks[B_ID] & COMPONENT_NONE) != COMPONENT_NONE)
				{
					// Apply an effect just to see if this shit works at all...
					auto* T = &Manager->EffectSystem->TransferredEffects[B_ID]["Poison"];
					T->Type = EffectType::TEMPORARY;
					T->Apply = &Effects::Cold;
					T->Timer = Component::TimerComponent{5.0f, 0.05f, B_ID};
					T->Entity = B_ID;
				}
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

		auto AM = Manager->AttributeSystem->Masks[A_ID];
		auto BM = Manager->AttributeSystem->Masks[B_ID];

		// Leave if debris
		if (AM & Masks::GeneralOptions::DEBRIS || BM & Masks::GeneralOptions::DEBRIS) return;


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

				// Particle effects for shiggles...
				// Add blood particle effect (totally a test)...
				const EM::Vec3* PP = &Manager->TransformSystem->Transforms[Player].Position;
				static GLuint PTex = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/orb.png").id;

				EG::ColorRGBA16 C1 = EG::RGBA16(1.0f, 0, 0, 1.0f);

				// Add 100 at a time
				for (Enjon::uint32 i = 0; i < 2; i++)
				{
					float XPos = Enjon::Random::Roll(-50, 100), YPos = Enjon::Random::Roll(-50, 100), ZVel = Enjon::Random::Roll(-10, 10), XVel = Enjon::Random::Roll(-10, 10), 
									YVel = Enjon::Random::Roll(-10, 10), YSize = Enjon::Random::Roll(1, 3), XSize = Enjon::Random::Roll(1, 3);

					EG::Particle2D::AddParticle(EM::Vec3(PP->x + 50.0f, PP->y + 50.0f, 100.0f), EM::Vec3(XVel, YVel, ZVel), 
						EM::Vec2(XSize, YSize), C1, PTex, 0.05f, Manager->ParticleEngine->ParticleBatches[0]);
				}

			}
			
			// Continue to next entity 	
			return;
		}
	}
	void CollideWithDebris(Systems::EntityManager* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID)
	{
		Enjon::Math::Vec3* EntityPosition = &Manager->TransformSystem->Transforms[A_ID].Position;
		Enjon::Math::Vec3* ColliderPosition = &Manager->TransformSystem->Transforms[B_ID].Position;
		Enjon::Math::Vec2* A = &Manager->TransformSystem->Transforms[B_ID].CartesianPosition;
		Enjon::Math::Vec2* B = &Manager->TransformSystem->Transforms[A_ID].CartesianPosition;
		Enjon::Math::Vec3* EntityVelocity = &Manager->TransformSystem->Transforms[A_ID].Velocity; 
		Enjon::Math::Vec3* ColliderVelocity = &Manager->TransformSystem->Transforms[B_ID].Velocity; 
		Enjon::Physics::AABB* AABB_A = &Manager->TransformSystem->Transforms[A_ID].AABB;
		Enjon::Physics::AABB* AABB_B = &Manager->TransformSystem->Transforms[B_ID].AABB;
		float AMass = Manager->TransformSystem->Transforms[A_ID].Mass;
		float BMass = Manager->TransformSystem->Transforms[B_ID].Mass;

		// Collision didn't happen
		if (!Enjon::Physics::AABBvsAABB(AABB_A, AABB_B)) { return; }
	
		else
		{

			if (Manager->AttributeSystem->Masks[B_ID] & Masks::WeaponOptions::EXPLOSIVE)
			{
				V2 Direction = EM::Vec2::Normalize(*A - *B);
				if (Direction.x == 0) Direction.x = (float)ER::Roll(-100, 100) / 100.0f;
				if (Direction.y == 0) Direction.y = (float)ER::Roll(-100, 100) / 100.0f;
				float Length = Direction.Length();
				float Impulse = 55.0f / (Length + 0.001f);

				*EntityVelocity = (1.0f / AMass) * -Impulse * EM::Vec3(EM::CartesianToIso(Direction), EntityVelocity->z); 

				if (Manager->AttributeSystem->Masks[A_ID] & Masks::WeaponOptions::PROJECTILE)
				{
					*EntityVelocity = *EntityVelocity * 2.0f;
					Manager->TransformSystem->Transforms[A_ID].VelocityGoal = *EntityVelocity;

					EM::Vec2 R(1,0);
					float a = acos(Direction.DotProduct(R)) * 180.0f / M_PI;
					if (Direction.y < 0) a *= -1;

					Manager->TransformSystem->Transforms[A_ID].Angle = EM::ToRadians(a);

				}
			}
			else if (Manager->AttributeSystem->Masks[B_ID] & Masks::WeaponOptions::MELEE)
			{
				V2 Direction = *A - *B;
				Direction.x += ER::Roll(-10, 10);
				Direction.y += ER::Roll(-10, 10);
				Direction = EM::Vec2::Normalize(Direction);
				if (Direction.x == 0) Direction.x = (float)ER::Roll(-100, 100) / 100.0f;
				if (Direction.y == 0) Direction.y = (float)ER::Roll(-100, 100) / 100.0f;
				float Length = Direction.Length();
				float Impulse = 25.0f / (Length + 0.001f);

				*EntityVelocity = (1.0f / AMass) * -Impulse * EM::Vec3(EM::CartesianToIso(Direction), EntityVelocity->z);

				if (Manager->AttributeSystem->Masks[A_ID] & Masks::WeaponOptions::PROJECTILE)
				{
					*EntityVelocity = *EntityVelocity * 2.0f;
					Manager->TransformSystem->Transforms[A_ID].VelocityGoal = *EntityVelocity;

					EM::Vec2 R(1,0);
					float a = acos(Direction.DotProduct(R)) * 180.0f / M_PI;
					if (Direction.y < 0) a *= -1;

					Manager->TransformSystem->Transforms[A_ID].Angle = EM::ToRadians(a);

				}

			}
			else if (Manager->AttributeSystem->Masks[B_ID] & Masks::GeneralOptions::DEBRIS && 
					 Manager->AttributeSystem->Masks[A_ID] & Masks::GeneralOptions::DEBRIS)
			{
				{
					V2 mtd = Enjon::Physics::MinimumTranslation(AABB_B, AABB_A);
					// *EntityVelocity = 0.98f * *EntityVelocity + -0.05f * EM::Vec3(EM::CartesianToIso(mtd), 0.0f);
					*EntityPosition -= Enjon::Math::Vec3(Enjon::Math::CartesianToIso(mtd), EntityPosition->z); 
					// *ColliderVelocity = 0.98f * *ColliderVelocity + 0.05f * EM::Vec3(EM::CartesianToIso(mtd), 0.0f);
					// *ColliderPosition += Enjon::Math::Vec3(Enjon::Math::CartesianToIso(mtd), ColliderPosition->z); 
				}
			}
			else
			{
				// Get minimum translation distance
				V2 mtd = Enjon::Physics::MinimumTranslation(AABB_B, AABB_A);
				*EntityVelocity = 0.85f * *EntityVelocity + -0.05f * EM::Vec3(EM::CartesianToIso(mtd), 0.0f);

				// V2 Direction = *A - *B;
				// Direction.x += ER::Roll(-10, 10);
				// Direction.y += ER::Roll(-10, 10);
				// Direction = EM::Vec2::Normalize(Direction);
				// if (Direction.x == 0) Direction.x = (float)ER::Roll(-100, 100) / 100.0f;
				// if (Direction.y == 0) Direction.y = (float)ER::Roll(-100, 100) / 100.0f;
				// float Length = Direction.Length();
				// float Impulse = 25.0f / (Length + 0.001f);
				// *ColliderPosition += Enjon::Math::Vec3(Enjon::Math::CartesianToIso(mtd), EntityPosition->z);

				// *EntityVelocity = (1.0f / AMass) * 0.05f * -Impulse * EM::Vec3(EM::CartesianToIso(Direction), EntityVelocity->z);
				// *ColliderVelocity = (1.0f / BMass) * 0.05f * Impulse * EM::Vec3(EM::CartesianToIso(Direction), EntityVelocity->z);
				// *ColliderVelocity = 1.0f * *ColliderVelocity + 0.05f * EM::Vec3(EM::CartesianToIso(mtd), 0.0f);
			}
		}

		return;
	}

	void CollideWithVortex(Systems::EntityManager* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID)
	{
		static float t = 0.0f;
		t += 0.001f;

		Enjon::Math::Vec3* EntityPosition = &Manager->TransformSystem->Transforms[A_ID].Position;
		Enjon::Math::Vec3* ColliderPosition = &Manager->TransformSystem->Transforms[B_ID].Position;
		Enjon::Math::Vec2* A = &Manager->TransformSystem->Transforms[B_ID].CartesianPosition;
		Enjon::Math::Vec2* B = &Manager->TransformSystem->Transforms[A_ID].CartesianPosition;
		Enjon::Math::Vec3* EntityVelocity = &Manager->TransformSystem->Transforms[A_ID].Velocity; 
		Enjon::Math::Vec3* ColliderVelocity = &Manager->TransformSystem->Transforms[B_ID].Velocity; 
		Enjon::Physics::AABB* AABB_A = &Manager->TransformSystem->Transforms[A_ID].AABB;
		Enjon::Physics::AABB* AABB_B = &Manager->TransformSystem->Transforms[B_ID].AABB;
		float AMass = Manager->TransformSystem->Transforms[A_ID].Mass;
		float BMass = Manager->TransformSystem->Transforms[B_ID].Mass;

		// Collision didn't happen
		if (!Enjon::Physics::AABBvsAABB(AABB_A, AABB_B)) { return; }
	
		else
		{
			if (Manager->AttributeSystem->Masks[A_ID] & Masks::WeaponOptions::PROJECTILE)
			{
				auto Center = (AABB_B->Max + AABB_B->Min) / 2.0f;
				float DistFromCenter = B->DistanceTo(Center);

				if (DistFromCenter > 100.0f)
				{

					V2 Direction = EM::Vec2::Normalize(Center - *B);
					float Length = Direction.Length();
					float Impulse = 2.0f * 15 - Length;
					V2 mtd = Enjon::Physics::MinimumTranslation(AABB_B, AABB_A);
					*EntityVelocity = 0.2f * *EntityVelocity + (1.0f / 1000.0f) * Impulse * EM::Vec3(EM::CartesianToIso(Direction), sin(t) * ER::Roll(1, 10));
					*EntityPosition += EM::Vec3(ER::Roll(-1, 1), ER::Roll(-1, 1), 0.0f);

					EM::Vec2 R(1,0);
					float a = acos(Direction.DotProduct(R)) * 180.0f / M_PI;
					if (Direction.y < 0) a *= -1;

					Manager->TransformSystem->Transforms[A_ID].Angle = EM::ToRadians(a);
				}
			}
			else
			{
				auto Center = (AABB_B->Max + AABB_B->Min) / 2.0f;
				float DistFromCenter = A->DistanceTo(Center);
			
				if (DistFromCenter > 13.0f)
				{
					V2 Direction = EM::Vec2::Normalize(*A - *B);
					float Length = Direction.Length();
					float Impulse = 2.0f;
					V2 mtd = Enjon::Physics::MinimumTranslation(AABB_B, AABB_A);
					*EntityVelocity = 0.85f * *EntityVelocity + (1.0f / AMass) * Impulse * EM::Vec3(EM::CartesianToIso(Direction) + 
																						   EM::CartesianToIso(0.25f * EM::Vec2(2.0f * cos(t), sin(t))), sin(t) * 1.0f);
					Manager->TransformSystem->Transforms[A_ID].VelocityGoal = EM::Vec3(0.0f, 0.0f, 0.0f);
					// *EntityPosition += EM::Vec3(ER::Roll(-1, 1), ER::Roll(-1, 1), 0.0f);
					EM::Vec2 R(1,0);
					float a = acos(Direction.DotProduct(R)) * 180.0f / M_PI;
					if (Direction.y < 0) a *= -1;
					*EntityPosition = *EntityPosition + (2.0f / AMass) * EM::Vec3(EM::CartesianToIso(EM::Vec2(cos(EM::ToRadians(a + 180)), sin(EM::ToRadians(a + 180)))), 0.0f);
				}	
			}

			Manager->AttributeSystem->Groups[A_ID].Parent = Manager->AttributeSystem->Groups[B_ID].Parent;
		}

		return;
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

		Enjon::Math::Vec2 Difference = Enjon::Math::Vec2::Normalize(EntityPosition->XY() - ColliderPosition->XY());

		// Height not the same... Testing
		if (abs(EntityPosition->z - ColliderPosition->z) > 100.0f) return;

		// Collision didn't happen
		if (!Enjon::Physics::AABBvsAABB(AABB_A, AABB_B)) { return; }
	
		else
		{
			// Get minimum translation distance
			V2 mtd = Enjon::Physics::MinimumTranslation(AABB_B, AABB_A);

			// if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::ITEM) *EntityVelocity = 0.25f * EM::Vec3(EM::CartesianToIso(mtd), 20.0f);
			// else 
			{
				*EntityPosition -= Enjon::Math::Vec3(Enjon::Math::CartesianToIso(mtd), EntityPosition->z); 
			}
			Manager->TransformSystem->Transforms[A_ID].GroundPosition -= Enjon::Math::CartesianToIso(mtd); 

			if (!Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
				*ColliderPosition += Enjon::Math::Vec3(Enjon::Math::CartesianToIso(mtd), 0.0f);


			// if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
				// *ColliderPosition -= Enjon::Math::Vec3(Difference * 30.0f, 0.0f);
			if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
				*ColliderVelocity += -2.0f * Enjon::Math::Vec3(Difference, 0.0f);

			// Update velocities based on "bounce" factor
			float bf; // Bounce factor 
			if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
			{
				// Shake the camera for effect
				Manager->Camera->ShakeScreen(Enjon::Random::Roll(10, 15));
	
				bf = 1.2f;
			}

			else bf = 1.0f;

			// ColliderVelocity->x = -ColliderVelocity->x * bf;
			// ColliderVelocity->y = -ColliderVelocity->y * bf;


			if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
			{
				// NOTE(John): This could cause some trouble eventually
				if (Manager->AttributeSystem->Masks[B_ID] & Masks::GeneralOptions::DAMAGED) return;

				// Get min and max damage of weapon
				auto DC = Manager->AttributeSystem->WeaponProfiles[A_ID]->Damage;
				float MiD = DC.Min;
				float MaD = DC.Max;

				float Damage = static_cast<float>(Enjon::Random::Roll(MiD, MaD));

				// Get health and color of entity
				Component::HealthComponent* HealthComponent = &Manager->AttributeSystem->HealthComponents[B_ID];
				Enjon::Graphics::ColorRGBA16* Color = &Manager->Renderer2DSystem->Renderers[B_ID].Color;

				// Set option to damaged
				Manager->AttributeSystem->Masks[B_ID] |= Masks::GeneralOptions::DAMAGED;	

				if (HealthComponent == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Collider health component is null");
				if (Color == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Color component is null");
		
				// Decrement by some arbitrary amount for now	
				HealthComponent->Health -= Damage;

				// Add blood particle effect (totally a test)...
				// EM::Vec3* PP = &Manager->TransformSystem->Transforms[B_ID].Position;
				static GLuint PTex = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/blood_1.png").id;

				EG::ColorRGBA16 R = EG::RGBA16(1.0f, 0.01f, 0.01f, 1.0f);

				for (auto p = 0; p < 100; p++)
				{
					float XPos = Enjon::Random::Roll(-50, 100), YPos = Enjon::Random::Roll(-50, 100), ZVel = Enjon::Random::Roll(-10, 10), XVel = Enjon::Random::Roll(-10, 10), 
									YSize = Enjon::Random::Roll(10, 20), XSize = Enjon::Random::Roll(10, 20);

					// EG::Particle2D::AddParticle(EM::Vec3(ColliderPosition->x + 50.0f + XVel, ColliderPosition->y + 50.0f + ZVel, 0.0f), EM::Vec3(XVel, XVel, ZVel), 
					// 	EM::Vec2(XSize * 1.5f, YSize * 1.5f), R, PTex, 0.005f, Manager->ParticleEngine->ParticleBatches.at(0));

					EG::Particle2D::AddParticle(
												EM::Vec3(ColliderPosition->x + 50.0f + XVel, ColliderPosition->y + 50.0f + ZVel, 50.0f), 
												15.0f * EM::Vec3(
															-Difference.x + static_cast<float>(ER::Roll(-2, 2)) / 10.0f, 
															-Difference.y + static_cast<float>(ER::Roll(-2, 2)) / 10.0f, 
															static_cast<float>(ER::Roll(-2, 2)) / 10.0f
														), 
						EM::Vec2(XSize * 1.5f, YSize * 1.5f), R, PTex, 0.005f, Manager->ParticleEngine->ParticleBatches.at(0));
				}

				// Print out that damage, son!
				auto DamageString = std::to_string(static_cast<Enjon::uint32>(Damage));

				auto x_pos_advance = -5.0f;
				auto DamageColor = EG::RGBA16_White();
				if 		(Damage >= 0.8f * ((MaD - MiD) + MiD)) 							DamageColor = EG::RGBA16_Red();
				else if (Damage >= 0.4f * ((MaD - MiD) + MiD)) 							DamageColor = EG::RGBA16_Orange();
				else 																	DamageColor = EG::RGBA16_ZombieGreen();
				for (auto c : DamageString)
				{
					auto x_advance = 0.0f;
					auto F = EG::FontManager::GetFont("8Bit_32");
					auto CS = EG::Fonts::GetCharacterAttributes(EM::Vec2(ColliderPosition->x, ColliderPosition->y + 50.0f), 1.0f, F, c, &x_advance);

					EG::Particle2D::AddParticle(
													EM::Vec3(ColliderPosition->x + x_pos_advance, ColliderPosition->y + 200.0f, 0.0f), 
													EM::Vec3(0.0f, 0.0f, 2.0f), 
													EM::Vec2(
																EG::Fonts::GetAdvance(c, F, 1.0f), 
																EG::Fonts::GetHeight(c, F, 1.0f)
															), 
													DamageColor,
													CS.TextureID, 
													0.005f, 
													Manager->ParticleEngine->ParticleBatches.at(2)
												);

					x_pos_advance += 30.0f;
				}

				x_pos_advance = 0.0f;


					// Blood!
					DrawBlood(Manager, ColliderPosition->XY());
				// }

				// Apply an effect just to see if this shit work at all...
				// 20% chance to apply
				float Percent = 0.2f;
				float Chance = (float)Enjon::Random::Roll(0, 100) / 100.0f;
				if (Chance < Percent)
				{
					auto* T = &Manager->EffectSystem->TransferredEffects[B_ID]["Poison"];
					T->Type = EffectType::TEMPORARY;
					T->Apply = &Effects::Poison;
					T->Timer = Component::TimerComponent{3.0f, 0.05f, B_ID};
					T->Entity = B_ID;
				}

				// If dead, then kill it	
				// TODO(John): This doesn't belong here. Need to have this be a dispatched message to the attribute system or the entity manager itself.
				if (HealthComponent->Health <= 0.0f)
				{
					// Drop some loot!
					// Loot::DropLootBasedOnProfile(Manager, B_ID);

					// auto* LP = Manager->AttributeSystem->LootProfiles[B_ID];

					// Put body overlay onto the world
					// DrawBody(Manager, ColliderPosition->XY());

					// Let's try and make an explosion of debris
					// auto max_debris = ER::Roll(15, 20);
					// for (auto e = 0; e < max_debris; e++)
					// {
					// 	auto id = ECS::Factory::CreateGib(
					// 												Manager, 
					// 												*ColliderPosition, 
					// 												20.0f * EM::Vec3(
					// 																	-Difference.x + static_cast<float>(ER::Roll(-2, 2)) / 10.0f, 
					// 																	-Difference.y + static_cast<float>(ER::Roll(-2, 2)) / 10.0f, 
					// 																	0.0f
					// 																)
					// 											);
					// }

					// Remove collider
					EntitySystem::RemoveEntity(Manager, B_ID);
					
				}
			}

			// Continue with next entity
			return;
		}
	}

	void DrawBody(ECS::Systems::EntityManager* Manager, Enjon::Math::Vec2 PP)
	{
		auto S = 200.0f;
		auto C = Enjon::Graphics::RGBA16_White();
		auto DC = 0.75f;
		C = Enjon::Graphics::RGBA16(C.r - DC, C.g - DC, C.b - DC, C.a);
		Manager->Lvl->AddTileOverlay(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/body.png"), Enjon::Math::Vec4(PP, S, S * 0.26f), C);
	}

	void DrawBlood(ECS::Systems::EntityManager* Manager, Enjon::Math::Vec2 PP)
	{
		for (auto i = 0; i < 5; i++)
		{
			auto I = Enjon::Random::Roll(0, 2);
			Enjon::Graphics::GLTexture S;
			switch(I)
			{
				case 0: S = Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/blood_1.png"); break;
				case 1: S = Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/blood_2.png"); break;
				case 2: S = Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/blood_3.png"); break;
				default: break;
			}
			auto alpha = Enjon::Random::Roll(50, 255) / 255.0f;
			auto X = (float)Enjon::Random::Roll(-50, 100);
			auto Y = (float)Enjon::Random::Roll(-100, 50);
			auto C = Enjon::Graphics::RGBA16_White();
			auto DC = Enjon::Random::Roll(80, 100) / 255.0f;
			C = Enjon::Graphics::RGBA16(C.r - DC, C.g - DC, C.b - DC, alpha);
			Manager->Lvl->AddTileOverlay(S, Enjon::Math::Vec4(PP.x + X, PP.y + Y, (float)Enjon::Random::Roll(50, 100), (float)Enjon::Random::Roll(50, 100)), C);
		}
	}

}}}



