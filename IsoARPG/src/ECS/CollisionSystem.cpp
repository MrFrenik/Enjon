#include "ECS/ComponentSystems.h"
#include "ECS/CollisionSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/InventorySystem.h"
#include "ECS/PlayerControllerSystem.h"
#include "ECS/Renderer2DSystem.h"
#include "ECS/EffectSystem.h"
#include "ECS/Effects.h"
#include "ECS/EntityFactory.h"

#include <Graphics/Camera2D.h>
#include <Graphics/ParticleEngine2D.h>
#include <Graphics/FontManager.h>
#include <Graphics/Color.h>
#include <Defines.h>
#include <System/Types.h>
#include <Math/Random.h>
#include <IO/ResourceManager.h>

#include <SDL2/SDL.h>
#include <time.h>

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
		static Enjon::uint32 StartTime = 0;
		static Enjon::uint32 GetCellsTime = 0;
		static Enjon::uint32 CollisionCheckTime = 0;

		eid32 size = Manager->CollisionSystem->Entities.empty() ? 0 : Manager->CollisionSystem->Entities.size();	
		
		for (eid32 n = 0; n < size; n++)
		{
			// Get entity
			eid32 e = Manager->CollisionSystem->Entities[n];

			// If entity has no transform, then continue
			if (!(Manager->Masks[e] & COMPONENT_TRANSFORM3D)) continue;
			
			// Get the cell(s) that entity belongs to
			const Enjon::Math::Vec2* EPosition = &Enjon::Math::IsoToCartesian(Manager->TransformSystem->Transforms[e].GroundPosition);
			std::vector<eid32> Entities = SpatialHash::FindCell(Manager->Grid, e, &Manager->TransformSystem->Transforms[e].AABB);

			// Use these same entities for targets if player
			if (Manager->Masks[e] & COMPONENT_PLAYERCONTROLLER)
			{
				// printf("size: %d\n", Entities.size());
				std::vector<eid32>* Targets = &Manager->PlayerControllerSystem->Targets;
				for (eid32 it = 0; it < Entities.size(); it++)
				{
					if (Manager->Masks[Entities[it]] & COMPONENT_AICONTROLLER)
					{
						Targets->push_back(Entities[it]);	
					}
				}
			}

			// TODO(John): Keep a mapping of already checked pairs to cut this time down
		
			// Collide with entities 
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
			Manager->Camera->ShakeScreen(Enjon::Random::Roll(0, 15));

			// Get minimum translation distance
			V2 mtd = Enjon::Physics::MinimumTranslation(AABB_A, AABB_B);

			// Update velocities based on "bounce" factor
			float bf = 1.0f; // Bounce factor 
			ColliderVelocity->x = -ColliderVelocity->x * bf;
			ColliderVelocity->y = -ColliderVelocity->y * bf;

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
			for (Enjon::uint32 i = 0; i < 10; i++)
			{
				float XPos = Enjon::Random::Roll(-50, 100), YPos = Enjon::Random::Roll(-50, 100), ZVel = Enjon::Random::Roll(-10, 10), XVel = Enjon::Random::Roll(-10, 10), 
								YSize = Enjon::Random::Roll(2, 7), XSize = Enjon::Random::Roll(1, 5);

				EG::Particle2D::AddParticle(EM::Vec3(PP->x + 50.0f + XVel, PP->y + 50.0f + ZVel, 0.0f), EM::Vec3(XVel, XVel, ZVel), 
					EM::Vec2(XSize, YSize), R, PTex, 0.05f, Manager->ParticleEngine->ParticleBatches[0]);
			}

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
				// Remove entity if no health
				EntitySystem::RemoveEntity(Manager, B_ID);

				// Drop some loot!
				Loot::DropLootBasedOnProfile(Manager, B_ID);

				auto* LP = Manager->AttributeSystem->LootProfiles[B_ID];

				printf("Chance to Drop: %.2f\n", 100.0f * LP->ChanceToDrop);
			}
			else
			{
				// Apply an effect just to see if this shit works at all...
				auto* T = &Manager->EffectSystem->TransferredEffects[B_ID]["Poison"];
				T->Type = EffectType::TEMPORARY;
				T->Apply = &Effects::Cold;
				T->Timer = Component::TimerComponent{5.0f, 0.05f, B_ID};
				T->Entity = B_ID;
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

				// Particle effects for shiggles...
				// Add blood particle effect (totally a test)...
				const EM::Vec3* PP = &Manager->TransformSystem->Transforms[Player].Position;
				static GLuint PTex = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/orb.png").id;

				EG::ColorRGBA16 C1 = EG::RGBA16(1.0f, 0, 0, 1.0f);

				// Add 100 at a time
				for (Enjon::uint32 i = 0; i < 5; i++)
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

			*EntityPosition -= Enjon::Math::Vec3(Enjon::Math::CartesianToIso(mtd), EntityPosition->z); 
			Manager->TransformSystem->Transforms[A_ID].GroundPosition -= Enjon::Math::CartesianToIso(mtd); 

			// if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
			// 	*ColliderPosition += Enjon::Math::Vec3(Enjon::Math::CartesianToIso(mtd) * 1.0f, 0.0f);
			Enjon::Math::Vec2 Difference = Enjon::Math::Vec2::Normalize(EntityPosition->XY() - ColliderPosition->XY());

			if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
				*ColliderPosition -= Enjon::Math::Vec3(Difference * 30.0f, 0.0f);

			// Update velocities based on "bounce" factor
			float bf; // Bounce factor 
			if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
			{
				// Shake the camera for effect
				Manager->Camera->ShakeScreen(Enjon::Random::Roll(10, 15));
	
				bf = 1.2f;
			}

			else bf = 1.0f;

			ColliderVelocity->x = -ColliderVelocity->x * bf;
			ColliderVelocity->y = -ColliderVelocity->y * bf;


			if (Manager->AttributeSystem->Masks[A_ID] & Masks::Type::WEAPON)
			{
				// Get min and max damage of weapon
				auto DC = Manager->AttributeSystem->WeaponProfiles[A_ID]->Damage;
				auto MiD = DC.Min;
				auto MaD = DC.Max;

				auto Damage = Enjon::Random::Roll(MiD, MaD);

				// Get health and color of entity
				Component::HealthComponent* HealthComponent = &Manager->AttributeSystem->HealthComponents[B_ID];
				Enjon::Graphics::ColorRGBA16* Color = &Manager->Renderer2DSystem->Renderers[B_ID].Color;

				// Set option to damaged
				Manager->AttributeSystem->Masks[B_ID] |= Masks::GeneralOptions::DAMAGED;	

				if (HealthComponent == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Collider health component is null");
				if (Color == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Color component is null");
		
				// Decrement by some arbitrary amount for now	
				HealthComponent->Health -= Damage;

				// printf("Hit for %d damage.\n", Damage);

				// Add blood particle effect (totally a test)...
				const EM::Vec3* PP = &Manager->TransformSystem->Transforms[B_ID].Position;
				static GLuint PTex = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/orb.png").id;

				EG::ColorRGBA16 R = EG::RGBA16(1.0f, 0.01f, 0.01f, 1.0f);

				// Add 100 at a time
				for (Enjon::uint32 i = 0; i < 5; i++)
				{
					float XPos = Enjon::Random::Roll(-50, 100), YPos = Enjon::Random::Roll(-50, 100), ZVel = Enjon::Random::Roll(-10, 10), XVel = Enjon::Random::Roll(-10, 10), 
									YSize = Enjon::Random::Roll(2, 7), XSize = Enjon::Random::Roll(1, 5);

					EG::Particle2D::AddParticle(EM::Vec3(PP->x + 50.0f + XVel, PP->y + 50.0f + ZVel, 0.0f), EM::Vec3(XVel, XVel, ZVel), 
						EM::Vec2(XSize * 1.5f, YSize * 1.5f), R, PTex, 0.05f, Manager->ParticleEngine->ParticleBatches[0]);
				}


				// This doesn't work too well right now...
				// std::string S("23");
			 //    std::string::const_iterator c;
			 //    float x = PP->x + 100.0f;
			 //    float y = PP->y + 150.0f;
			 //    float advance = 0.0f;
			 //    float scale = 1.5f;
			 //    for (c = S.begin(); c != S.end(); c++) 
			 //    {
				// 	EG::Fonts::CharacterStats CS = 
				// 				EG::Fonts::GetCharacterAttributes(EM::Vec2(x, y), scale, EG::FontManager::GetFont("Bold"), c, &advance);

				// 	// Create particle
				// 	EG::Particle2D::AddParticle(EM::Vec3(CS.DestRect.x, CS.DestRect.y, 0.0f), EM::Vec3(0.0f, 0.0f, 10.0f), EM::Vec2(CS.DestRect.z, CS.DestRect.w), 
				// 								EG::RGBA16_White(), CS.TextureID, 0.025f, Manager->ParticleEngine->ParticleBatches[0]);

				// 	x += advance;
			 //    }


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
				if (HealthComponent->Health <= 0.0f)
				{
					// Drop some loot!
					Loot::DropLootBasedOnProfile(Manager, B_ID);

					auto* LP = Manager->AttributeSystem->LootProfiles[B_ID];

					printf("Chance to Drop: %.2f\n", 100.0f * LP->ChanceToDrop);

					// Remove collider
					EntitySystem::RemoveEntity(Manager, B_ID);
					
				}
			}

			// Continue with next entity
			return;
		}
	}


	// TODO(John): Create "loot profiles" for AI and feed that into this function and any like it
	void DropRandomLoot(Systems::EntityManager* Manager, Enjon::uint32 count, const Enjon::Math::Vec2* Position)
	{
		static Enjon::Graphics::SpriteSheet ItemSheet; 
		if (!ItemSheet.IsInit()) ItemSheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Enjon::Math::iVec2(1, 1));

		for (int i = 0; i < count; i++)
		{
			int Roll = Enjon::Random::Roll(0, 5);

			Enjon::Graphics::ColorRGBA16 ItemColor;

			if (Roll == 0) ItemColor = Enjon::Graphics::RGBA16_Red();
			if (Roll == 1) ItemColor = Enjon::Graphics::RGBA16_Orange();
			if (Roll == 2) ItemColor = Enjon::Graphics::RGBA16_Blue();
			if (Roll == 3) ItemColor = Enjon::Graphics::RGBA16_Green();
			if (Roll == 4) ItemColor = Enjon::Graphics::RGBA16_Yellow();
			if (Roll == 5) ItemColor = Enjon::Graphics::RGBA16_Magenta();

			eid32 id = Factory::CreateItem(Manager, Enjon::Math::Vec3(Enjon::Random::Roll(Position->x - 64.0f, Position->x + 64.0f), 
												  Enjon::Random::Roll(Position->y - 64.0f, Position->y + 64.0f), 0.0f), 
												  Enjon::Math::Vec2(16.0f, 16.0f), &ItemSheet, (Masks::Type::ITEM | Masks::ItemOptions::CONSUMABLE), 
												  Component::EntityType::ITEM, "Item", Enjon::Graphics::SetOpacity(ItemColor, 0.5f));
			Manager->TransformSystem->Transforms[id].VelocityGoal.x = 0.0f;
			Manager->TransformSystem->Transforms[id].VelocityGoal.y = 0.0f;
			Manager->TransformSystem->Transforms[id].Velocity = {0.0f, 0.0f, 0.0f};
		} 

	}

}}}



