#include "ECS/ECS.h"
#include "Loot.h"

#include <Graphics/SpriteSheetManager.h>
#include <Utils/Errors.h>

#include <string>

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
	void Update(Transform3DSystem* System, Enjon::Graphics::Particle2D::ParticleBatch2D* Batch)
	{
		EntityManager* Manager = System->Manager;
		// Look at the entities in the Manager up to the last entered position and then update based on component masks
		for (eid32 e = 0; e < Manager->MaxAvailableID; e++)
		{
			// If equal then transform that entity
			if (Manager->Masks[e] & COMPONENT_TRANSFORM3D)
			{
				// If equipped, then don't update transform here
				if (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::EQUIPPED) continue;

				if (Manager->AttributeSystem->Masks[e] & Masks::WeaponOptions::EXPLOSIVE) 
				{
					if ((Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::EXPLODED) == 0)
					{
						Manager->AttributeSystem->Masks[e] |= Masks::GeneralOptions::EXPLODED;
					}
					else EntitySystem::RemoveEntity(Manager, e);
				}

				// Get distance to player
				V2* GP = &Manager->TransformSystem->Transforms[e].GroundPosition;
				V2* PGP = &Manager->TransformSystem->Transforms[Manager->Player].GroundPosition;

				if (PGP->DistanceTo(*GP) >= 5000) continue; // TODO(John): Make this squared distance so as to not use a square root function EVERY frame for EVERY entity


				// Use these same entities for targets if player
					// printf("size: %d\n", Entities.size());
				std::vector<eid32>* Targets = &Manager->PlayerControllerSystem->Targets;
				if (Manager->Masks[e] & COMPONENT_AICONTROLLER)
				{
					if (PGP->DistanceTo(*GP) <= 1000) Targets->push_back(e);	
				}

				// If an item
				
				if ((Manager->AttributeSystem->Masks[e] & Masks::Type::ITEM) && (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::PICKED_UP) == 0)
				{
					// Need to see whether or not the player is within range before I turn on its collision component
					Enjon::Math::Vec3* P = &Manager->TransformSystem->Transforms[e].Position;

					if (PGP->DistanceTo(*GP) <= 2000) Manager->CollisionSystem->Entities.push_back(e);

					if (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::DEBRIS)
						Manager->TransformSystem->Transforms[e].Angle += 0.01f * Manager->TransformSystem->Transforms[e].Velocity.x;
				}

				// First transform the velocity by LERPing it
				Component::Transform3D* Transform = &System->Transforms[e];
				float Scale = Transform->VelocityGoalScale; 
				Enjon::Math::Vec3* Velocity = &Transform->Velocity; 
				Enjon::Math::Vec3* VelocityGoal = &Transform->VelocityGoal;
				Enjon::Math::Vec3* Position = &Transform->Position; 
				Enjon::Math::Vec2* GroundPosition = &Transform->GroundPosition; 
				float TileWidth = 32.0f;


				// Push back into renderer system
				if (Manager->Masks[e] & COMPONENT_RENDERER2D && Manager->Camera->IsBoundBoxInCamView(Position->XY(), Manager->TransformSystem->Transforms[e].Dimensions))
				{
					Manager->Renderer2DSystem->Entities.push_back(e);
				}
				
				Velocity->x = Enjon::Math::Lerp(VelocityGoal->x, Velocity->x, Scale); 
				Velocity->y = Enjon::Math::Lerp(VelocityGoal->y, Velocity->y, Scale); 
				Velocity->z = Enjon::Math::Lerp(VelocityGoal->z, Velocity->z, Scale);
				
				// Set entity's position after interpolating
				Position->x += Velocity->x;
				Position->z += Velocity->z;

				// Clamp z position to BaseHeight
				if (Position->z < Transform->BaseHeight) 
				{
					if ((Manager->AttributeSystem->Masks[e] & (Masks::WeaponOptions::PROJECTILE)) && (Manager->AttributeSystem->Masks[e] & Masks::WeaponSubOptions::GRENADE) == 0)
					{
						EntitySystem::RemoveEntity(Manager, e);
					}

					Velocity->z = 0.0f;
					Position->z = Transform->BaseHeight;
					// Position->y = GroundPosition->y + Position->z;
				} 

				if (Manager->AttributeSystem->Masks[e] & (Masks::WeaponSubOptions::GRENADE))
				{
					// Update grenade
					auto GrP = &Manager->TransformSystem->Transforms[e].Position.z;
					auto GrV = Manager->TransformSystem->Transforms[e].Velocity.z;
					auto BH = Manager->TransformSystem->Transforms[e].BaseHeight;
					float* TOP = &Manager->TransformSystem->Transforms[e].MaxHeight;
					if (*GrP >= *TOP && (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::RISING))
					{
						Manager->TransformSystem->Transforms[e].VelocityGoal.z = -(GrV - GrV / 4.0f);	
						*TOP -= *TOP / 4.0f;
						Manager->AttributeSystem->Masks[e] &= ~Masks::GeneralOptions::RISING;
					}
					else if (*GrP <= Manager->TransformSystem->Transforms[e].BaseHeight + *TOP / 2.0f && *TOP > 2.0f)
					{
						Manager->AttributeSystem->Masks[e] |= Masks::GeneralOptions::RISING;
						Manager->TransformSystem->Transforms[e].Velocity.z = *TOP;
					}
					else if (*TOP < 2.0f)
					{
						if ((Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::EXPLODED) == 0)
						{
							// auto B = Manager->ParticleEngine->ParticleBatches.at(0);
							if (Batch == nullptr) Enjon::Utils::FatalError("TRANSFORMSYSTEM::UPDATE::LINE_112::SpriteBatch Null");
							else Enjon::Graphics::Particle2D::DrawFire(Batch, Manager->TransformSystem->Transforms[e].Position);

							auto I = Enjon::Random::Roll(0, 2);
							Enjon::Graphics::GLTexture S;
							switch(I)
							{
								case 0: S = Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/explody.png"); break;
								case 1: S = Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/explody_2.png"); break;
								case 2: S = Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/explody_3.png"); break;
								default: break;
							}
							auto Position = &Manager->TransformSystem->Transforms[e].Position;
							auto alpha = Enjon::Random::Roll(50, 255) / 255.0f;
							auto X = (float)Enjon::Random::Roll(-50, 100);
							auto Y = (float)Enjon::Random::Roll(-100, 50);
							auto C = Enjon::Graphics::RGBA16_White();
							auto DC = Enjon::Random::Roll(80, 100) / 255.0f;
							C = Enjon::Graphics::RGBA16(C.r - DC, C.g - DC, C.b - DC, alpha);
							Manager->Lvl->AddTileOverlay(S, Enjon::Math::Vec4(Position->x + X, Position->y + Y, (float)Enjon::Random::Roll(50, 100), (float)Enjon::Random::Roll(50, 100)), C);

							Manager->Camera->ShakeScreen(Enjon::Random::Roll(30, 40));
							Manager->AttributeSystem->Masks[e] |= Masks::GeneralOptions::EXPLODED;

							// TODO(John): Make a "spawn" function that gets called for any entity that has a factory component
							ECS::eid32 Explosion = Factory::CreateWeapon(Manager, Enjon::Math::Vec3(Manager->TransformSystem->Transforms[e].Position.XY(), 0.0f), Enjon::Math::Vec2(16.0f, 16.0f), 
																		Enjon::Graphics::SpriteSheetManager::GetSpriteSheet("Orb"), 
																		Masks::Type::WEAPON | Masks::WeaponOptions::EXPLOSIVE, Component::EntityType::EXPLOSIVE, "Explosion");

							// Remove explosion
							ECS::Systems::EntitySystem::RemoveEntity(Manager, e);

							Manager->AttributeSystem->Masks[Explosion] |= Masks::GeneralOptions::COLLIDABLE;

							Manager->TransformSystem->Transforms[Explosion].Velocity = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
							Manager->TransformSystem->Transforms[Explosion].VelocityGoal = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
							Manager->TransformSystem->Transforms[Explosion].BaseHeight = 0.0f;
							Manager->TransformSystem->Transforms[Explosion].MaxHeight = 0.0f;

							Manager->TransformSystem->Transforms[Explosion].AABBPadding = Enjon::Math::Vec2(500, 500);
						}
					}
				}

				// Add a tracer to projectiles for testing
				if (Manager->LabelSystem->Labels[e].Name.compare("Arrow") == 0)
				{
					auto NV = EM::Vec2::Normalize(Velocity->XY());
					for (auto i = 0; i < 2; i++)
					{
						EM::Vec3 Vel((float)ER::Roll(-1, 1), (float)ER::Roll(-1, 1), (float)ER::Roll(-1, 1));
						Vel = Vel * 0.5f;
						EM::Vec3 Diff(ER::Roll(-10, 10) * i * 2, ER::Roll(-20, 10) * i * 2, ER::Roll(-2, 2));
						EM::Vec2 Size(ER::Roll(2, 5), ER::Roll(2, 5));
						auto C = EG::RGBA16(static_cast<float>(ER::Roll(2, 10)) / 5.0f, static_cast<float>(ER::Roll(1, 10)) / 10.0f, 0.0f, 1.0f);
						static GLuint PTex = EI::ResourceManager::GetTexture("../IsoARPG/assets/textures/verticlebar.png").id;
						EG::Particle2D::AddParticle(EM::Vec3(Position->x + Diff.x, Position->y + Diff.y, 0.0f), EM::Vec3(Vel.x, Vel.y, Vel.z), 
							EM::Vec2(Size.x, Size.y), C, PTex, 0.015f, Manager->ParticleEngine->ParticleBatches.at(0));
					}
				}

				// Get ground position offset
				auto GPO = &Manager->TransformSystem->Transforms[e].GroundPositionOffset;
					
				// Set position.y to be a sum of y and z velocities
				Position->y += Velocity->y + Velocity->z; 
				
				// Set up GroundPosition
				GroundPosition->x = Position->x + GPO->x;
				GroundPosition->y = Position->y - Position->z + GPO->y; 

				// Set up CartesianPosition
				Transform->CartesianPosition = Enjon::Math::IsoToCartesian(*GroundPosition);


				// Make sure that position is within bounds of World
				int Width = Manager->Width, Height = Manager->Height;
				bool CollideWithLevel = false;
				float Multiplier = -0.5f;

				if ((Manager->Types[e] & Component::EntityType::ENEMY | Component::EntityType::PLAYER) == 0)
				{
					if (Transform->CartesianPosition.x < -Width + TileWidth * 2.0f) { Transform->CartesianPosition.x = -Width + TileWidth * 2.0f; Velocity->x *= Multiplier; VelocityGoal->x *= Multiplier; CollideWithLevel = true; }   
					if (Transform->CartesianPosition.x > -TileWidth) { Transform->CartesianPosition.x = -TileWidth; Velocity->x *= Multiplier; VelocityGoal->x *= Multiplier; CollideWithLevel = true; }
					if (Transform->CartesianPosition.y > -TileWidth) { Transform->CartesianPosition.y = -TileWidth; Velocity->y *= Multiplier; VelocityGoal->y *= Multiplier; CollideWithLevel = true; }
					if (Transform->CartesianPosition.y < -Height + TileWidth * 2.0f) { Transform->CartesianPosition.y = -Height + TileWidth * 2.0f; Velocity->y *= Multiplier; VelocityGoal->y *= Multiplier; CollideWithLevel = true; }
				}


				// Delete projectile for now if it collides with level
				if ((Manager->Types[e] == Component::EntityType::PROJECTILE) && CollideWithLevel)
				{
					if ((Manager->AttributeSystem->Masks[e] & Masks::WeaponSubOptions::GRENADE) == 0 &&
						(Manager->AttributeSystem->Masks[e] & Masks::WeaponOptions::EXPLOSIVE) == 0)
					{
						EntitySystem::RemoveEntity(Manager, e);
						continue;
					}
					
				}

				
				*GroundPosition = Enjon::Math::CartesianToIso(Transform->CartesianPosition);
				Position->y = GroundPosition->y + Position->z - GPO->y;
				Position->x = GroundPosition->x - GPO->x;


				// Set up AABB
				Enjon::Physics::AABB* AABB = &Manager->TransformSystem->Transforms[e].AABB;
				V2* CP = &Transform->CartesianPosition;
				auto Dims = &Manager->TransformSystem->Transforms[e].AABBPadding;
				V2 Min(CP->x - Dims->x, CP->y - Dims->y);
				V2 Max(CP->x + TILE_SIZE + Dims->x, CP->y + TILE_SIZE + Dims->y);
				*AABB = {Min, Max};


				// Set up animation based on velocity
				if (Manager->Types[e] == Component::EntityType::PLAYER)
				{
					if (Animation2D::GetPlayerState() != Animation2D::EntityAnimationState::ATTACKING)
					{
						if (Velocity->y == 0 && Velocity->x == 0)
						{
							if (Animation2D::GetPlayerState() != Animation2D::EntityAnimationState::IDLE) Manager->Animation2DSystem->AnimComponents[e].CurrentIndex = 0;
							Animation2D::SetPlayerState(Animation2D::EntityAnimationState::IDLE);
						}
						else
						{
							if (Animation2D::GetPlayerState() != Animation2D::EntityAnimationState::WALKING) Manager->Animation2DSystem->AnimComponents[e].CurrentIndex = 0;
	 						Animation2D::SetPlayerState(Animation2D::EntityAnimationState::WALKING);
						}
					}
				}

				if (Manager->Types[e] == Component::EntityType::ENEMY)
				{
					auto AnimComponent = &Manager->Animation2DSystem->AnimComponents[e];
					auto State = AnimComponent->AnimState;

					if (State != Component::AnimationState::ATTACKING)
					{
						if (Velocity->y == 0 && Velocity->x == 0)
						{
							if (State != Component::AnimationState::IDLE) Manager->Animation2DSystem->AnimComponents[e].CurrentIndex = 0;
							AnimComponent->AnimState = Component::AnimationState::IDLE;
						}
						else
						{
							if (State != Component::AnimationState::WALKING) Manager->Animation2DSystem->AnimComponents[e].CurrentIndex = 0;
							AnimComponent->AnimState = Component::AnimationState::WALKING;
						}
					}
				}

				// Push back into collision system
				if (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::COLLIDABLE) 
				{
					Manager->CollisionSystem->Entities.push_back(e);

					// Find cell
					SpatialHash::FindCells(Manager->Grid, e, AABB, &Manager->CollisionSystem->CollisionComponents[e].Cells, Manager->CollisionSystem->CollisionComponents[e].ObstructionValue);
				}

				else if ((Manager->AttributeSystem->Masks[e] & Masks::Type::ITEM) && (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::PICKED_UP) == 0)
				{
					// Find cell
					SpatialHash::FindCells(Manager->Grid, e, AABB, &Manager->CollisionSystem->CollisionComponents[e].Cells, Manager->CollisionSystem->CollisionComponents[e].ObstructionValue);
				}

				// Go through the items in this entity's inventory and set to this position
				// NOTE(John): Note sure if I like this here... or at all...

				// Be careful, cause this has fucked me up...
				if (Manager->AttributeSystem->Masks[e] & Masks::Type::ITEM)
				{
					continue;	
				}

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

				// AABB structs
				Enjon::Physics::AABB NE;
				Enjon::Physics::AABB N;
				Enjon::Physics::AABB NW;
				Enjon::Physics::AABB W;
				Enjon::Physics::AABB SW;
				Enjon::Physics::AABB S;
				Enjon::Physics::AABB SE;
				Enjon::Physics::AABB E;

				// Get reach of weapon from its profile
				auto* WP = Manager->AttributeSystem->WeaponProfiles[WeaponEquipped];
				float WeaponSize = WP->Reach;
				auto ReachType = WP->Spread;

				// Get reach type of weapon to determine how to draw the AABB of the weapon
				if (ReachType == Loot::Weapon::ReachType::OMNIDIRECTION)
				{
					WeaponTransform->AABB = {V2(Min.x - WeaponSize / 2.0f, Min.y - WeaponSize / 2.0f), V2(Max.x + WeaponSize / 2.0f, Max.y + WeaponSize / 2.0f)};
				}

				else
				{
					// Calculate AABB
					NE 		= {V2(Min.x + TILE_SIZE / 2.0f, Min.y - WeaponSize / 2.0f), V2(Max.x + WeaponSize, Max.y + WeaponSize / 2.0f)};
					N 		= {V2(Min.x + TILE_SIZE / 2.0f, Min.y + TILE_SIZE / 2.0f), V2(Max.x + WeaponSize, Max.y + WeaponSize)};
					NW 		= {V2(Min.x - WeaponSize / 2.0f, Min.y + TILE_SIZE / 2.0f), V2(Max.x + WeaponSize / 2.0f, Max.y + WeaponSize)};
					W 		= {V2(Min.x - WeaponSize, Min.y + TILE_SIZE / 2.0f), V2(Max.x - TILE_SIZE / 2.0f, Max.y + WeaponSize)};
					SW 		= {V2(Min.x - WeaponSize, Min.y - WeaponSize / 2.0f), V2(Max.x - TILE_SIZE / 2.0f, Max.y + WeaponSize / 2.0f)};
					S 		= {V2(Min.x - WeaponSize, Min.y - WeaponSize), V2(Max.x - TILE_SIZE / 2.0f, Max.y - TILE_SIZE / 2.0f)};
					SE 		= {V2(Min.x - WeaponSize / 2.0f, Min.y - WeaponSize), V2(Max.x + WeaponSize / 2.0f, Max.y - TILE_SIZE / 2.0f)};
					E 		= {V2(Min.x + TILE_SIZE / 2.0f, Min.y - WeaponSize), V2(Max.x + WeaponSize, Max.y - TILE_SIZE / 2.0f)};

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
				}

				if (Manager->Masks[WeaponEquipped] & COMPONENT_TRANSFORM3D)
				{
					// Push weapon back into collision system
					Manager->CollisionSystem->Entities.push_back(WeaponEquipped);

					if (Manager->AttributeSystem->Masks[WeaponEquipped] & Masks::Type::WEAPON && Manager->Masks[WeaponEquipped] & COMPONENT_TRANSFORM3D)
					{
						// std::cout << "Hitting" << std::endl;
					}

					SpatialHash::FindCells(Manager->Grid, e, AABB, &Manager->CollisionSystem->CollisionComponents[WeaponEquipped].Cells, Manager->CollisionSystem->CollisionComponents[WeaponEquipped].ObstructionValue);
				}

			}
		}
	}

	// Resets transform component of entity
	void Reset(Systems::EntityManager* Manager, eid32 Entity)
	{
		// Get handle to transforms
		auto Transform = &Manager->TransformSystem->Transforms[Entity];

		Transform->Position 			= EM::Vec3(0.0f, 0.0f, 0.0f);
		Transform->Velocity 			= EM::Vec3(0.0f, 0.0f, 0.0f);
		Transform->VelocityGoal 		= EM::Vec3(0.0f, 0.0f, 0.0f);
		Transform->ViewVector 			= EM::Vec2(0.0f, 0.0f);
		Transform->AttackVector 		= EM::Vec2(0.0f, 0.0f);
		Transform->CartesianPosition 	= EM::Vec2(0.0f, 0.0f);
		Transform->AABB 				= {EM::Vec2(0.0f, 0.0f), EM::Vec2(0.0f, 0.0f)};
		Transform->AABBPadding 			= EM::Vec2(0.0f, 0.0f);
		Transform->GroundPosition 		= EM::Vec2(0.0f, 0.0f);
		Transform->Dimensions 			= EM::Vec2(0.0f, 0.0f);
		Transform->GroundPositionOffset = EM::Vec2(0.0f, 0.0f);

		Transform->Angle 				= 0.0f;
		Transform->BaseHeight 			= 0.0f;
		Transform->VelocityGoalScale 	= 0.0f;
		Transform->Mass 				= 0.0f;
		Transform->Entity 				= 0;
	}

}}}
















