#include "ECS/EntityFactory.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/Animation2DSystem.h"
#include "ECS/InventorySystem.h"
#include "ECS/LabelSystem.h"
#include "ECS/PlayerControllerSystem.h"
#include "ECS/Renderer2DSystem.h"
#include "ECS/EffectSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/Entity.h"


namespace ECS { namespace Factory {

		// Creats player entity and returns eid
		eid32 CreatePlayer(Systems::EntityManager* Manager, Enjon::Input::InputManager* Input, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, 
						   Enjon::Graphics::SpriteSheet* Sheet, char* Name, float VelocityScale, Enjon::Math::Vec3 Velocity, Enjon::Math::Vec3 VelocityGoal, 
						   float Health, Enjon::Graphics::ColorRGBA16 Color)
		{
			// Get id for player
			eid32 Player = Systems::EntitySystem::CreateEntity(Manager,
										COMPONENT_RENDERER2D		| 
										COMPONENT_TRANSFORM3D 		| 
										COMPONENT_PLAYERCONTROLLER 	| 
										COMPONENT_LABEL 			| 
										COMPONENT_ANIMATION2D 		| 
										COMPONENT_HEALTH); 

			// Set up Transform
			Component::Transform3D* Transform = &Manager->TransformSystem->Transforms[Player];
			Transform->Velocity = Velocity;
			Transform->Dimensions = Dimensions;
			Transform->Position = Position;
			Transform->VelocityGoalScale = VelocityScale;
			Transform->VelocityGoal = VelocityGoal; 
			Transform->ViewVector = Enjon::Math::Vec2(1, 1);
			Transform->AttackVector = Enjon::Math::Vec2(1, 1);
			Transform->GroundPosition = Enjon::Math::Vec2(Position.x + Dimensions.x / 2.0f, Position.y);
			Transform->CartesianPosition = Enjon::Math::IsoToCartesian(Transform->GroundPosition);
			Transform->BaseHeight = Position.z;
			Transform->AABBPadding = Enjon::Math::Vec2(0.0f, 0.0f);
			Transform->Angle = 0.0f;
			Transform->Mass = 3.0f;

			// Set up AABB
			V2* CP = &Transform->CartesianPosition;
			V2 Min(CP->x, CP->y);
			V2 Max(CP->x + Systems::TILE_SIZE, CP->y + Systems::TILE_SIZE);
			Transform->AABB = {Min, Max};

			// Set up Input
			Manager->PlayerControllerSystem->PlayerControllers[Player].Input = Input;
			
			// Set up Animation2D
			Component::Animation2D* Animation2D = &Manager->Animation2DSystem->Animations[Player];
			Animation2D->Sheet = Sheet; 
			// These are redundant...
			Animation2D->CurrentFrame = 0; 
			Animation2D->BeginningFrame = 0;
			Animation2D->CurrentAnimation = AnimationManager::GetAnimation("Player", "Walk");
			Animation2D->SetStart = 0;

			// Set up renderer component
			Manager->Renderer2DSystem->Renderers[Player].Format = EG::CoordinateFormat::CARTESIAN;

			// Set up Inventory System
			Manager->InventorySystem->Inventories[Player].Entity = Player;

			// Set up Label 
			// NOTE(John): This isn't the best way to do this; most likely will throw an error at some point
			Manager->LabelSystem->Labels[Player].Name = Name;
			Manager->LabelSystem->Labels[Player].Entity = Player;

			// Set up Attribute System
			AttributeSystem* AS = Manager->AttributeSystem;

			// Set up health component
			AS->HealthComponents[Player].Health = Health;
			AS->HealthComponents[Player].Entity = Player;

			// Set up masks
			AS->Masks[Player] |= (Masks::Type::PLAYER | Masks::GeneralOptions::COLLIDABLE);

			// Set up Renderer
			Manager->Renderer2DSystem->Renderers[Player].Color = Color;

			// Set up Type
			Manager->Types[Player] = Component::EntityType::PLAYER;

			// Set up inventory
			Manager->InventorySystem->Inventories[Player].WeaponEquipped = NULL_ENTITY; // Set out of range of being processed
			
			return Player;
		} 

		// Creates ai entity and returns eid
		eid32 CreateAI(Systems::EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, char* Name, float VelocityScale,
						 Enjon::Math::Vec3 Velocity, Enjon::Math::Vec3 VelocityGoal, float Health, Enjon::Graphics::ColorRGBA16 Color)
		{
			// Get id for ai
			eid32 AI = Systems::EntitySystem::CreateEntity(Manager, 
									COMPONENT_RENDERER2D	| 
									COMPONENT_TRANSFORM3D 	| 
									COMPONENT_AICONTROLLER 	| 
									COMPONENT_ANIMATION2D 	| 
									COMPONENT_LABEL 		| 
									COMPONENT_HEALTH		);

			// Set up Transform
			Component::Transform3D* Transform = &Manager->TransformSystem->Transforms[AI];
			Transform->Velocity = Velocity;
			Transform->Position = Position;
			Transform->Dimensions = Dimensions;
			Transform->VelocityGoal = VelocityGoal;
			Transform->VelocityGoalScale = VelocityScale;
			Transform->ViewVector = Enjon::Math::Vec2(1, 1);
			Transform->AttackVector = Enjon::Math::Vec2(1, 1);
			Transform->GroundPosition = Enjon::Math::Vec2(Position.XY());
			Transform->CartesianPosition = Enjon::Math::IsoToCartesian(Transform->GroundPosition);
			Transform->BaseHeight = Position.z;
			Transform->AABBPadding = Enjon::Math::Vec2(0.0f, 0.0f);
			Transform->Angle = 0.0f;
			Transform->Mass = 3.0f;

			// Set up AABB
			V2* CP = &Transform->CartesianPosition;
			V2 Min(CP->x, CP->y);
			V2 Max(CP->x + Systems::TILE_SIZE, CP->y + Systems::TILE_SIZE);
			Transform->AABB = {Min, Max};

			// Set up Animation2D
			Component::Animation2D* Animation2D = &Manager->Animation2DSystem->Animations[AI];
			Animation2D->Sheet = Sheet; 
			// These are redundant...
			Animation2D->CurrentFrame = 0; 
			Animation2D->BeginningFrame = 0;

			// Set up renderer component
			Manager->Renderer2DSystem->Renderers[AI].Format = EG::CoordinateFormat::CARTESIAN;
			
			// Set up Label
			// NOTE(John): This isn't the best way to do this; most likely will throw an error at some point
			Manager->LabelSystem->Labels[AI].Name = Name;

			// Set up Attributes
			AttributeSystem* AS = Manager->AttributeSystem;
			AS->HealthComponents[AI].Health = Health;
			AS->HealthComponents[AI].Entity = AI;

			// Set up loot drop profile
			auto NR = Enjon::Random::Roll(0, 1);
			std::string N;
			if (NR == 1) N = "Monster1";
			else N = "Monster2"; 
			AS->LootProfiles[AI] = Loot::GetProfile(N);
			
			// Set up masks
			AS->Masks[AI] |= (Masks::Type::AI | Masks::GeneralOptions::COLLIDABLE);

			// Set up Renderer
			Manager->Renderer2DSystem->Renderers[AI].Color = Color;

			// Set up type
			Manager->Types[AI] = Component::EntityType::ENEMY;

			// Set up inventory
			Manager->InventorySystem->Inventories[AI].WeaponEquipped = NULL_ENTITY;

			return AI;
		} 

		// Creates ai entity and returns eid
		eid32 CreateItem(Systems::EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, Masks::EntityMask Mask, 
								Component::EntityType Type, char* Name, Enjon::Graphics::ColorRGBA16 Color)
		{
			// Get id for ai
			eid32 Item = Systems::EntitySystem::CreateEntity(Manager, 
									COMPONENT_RENDERER2D	| 
									COMPONENT_TRANSFORM3D 	| 
									COMPONENT_ANIMATION2D 	| 
									COMPONENT_LABEL 		);

			// Set up Transform
			Component::Transform3D* Transform = &Manager->TransformSystem->Transforms[Item];
			Transform->Position = Position;
			Transform->VelocityGoal = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Transform->Velocity = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Transform->VelocityGoalScale = 0.3f;
			Transform->Dimensions = Dimensions;
			Transform->GroundPosition = Enjon::Math::Vec2(Position.XY());
			Transform->CartesianPosition = Enjon::Math::IsoToCartesian(Transform->GroundPosition);
			Transform->BaseHeight = Position.z;
			Transform->AABBPadding = Enjon::Math::Vec2(0.0f, 0.0f);
			Transform->Angle = 0.0f;
			Transform->Mass = 1.2f;

			// Set up AABB
			V2* CP = &Transform->CartesianPosition;
			V2 Min(CP->x, CP->y);
			V2 Max(CP->x + Systems::TILE_SIZE, CP->y + Systems::TILE_SIZE);
			Transform->AABB = {Min, Max};

			// Set up Animation2D
			Manager->Animation2DSystem->Animations[Item].Sheet = Sheet; 
			Manager->Animation2DSystem->Animations[Item].CurrentFrame = 0; 
			Manager->Animation2DSystem->Animations[Item].BeginningFrame = 0; 

			// Set up renderer component
			Manager->Renderer2DSystem->Renderers[Item].Format = EG::CoordinateFormat::CARTESIAN;
			
			// Set up Label
			// NOTE(John): This isn't the best way to do this; most likely will throw an error at some point
			Manager->LabelSystem->Labels[Item].Name = Name;

			// Set up Renderer
			Manager->Renderer2DSystem->Renderers[Item].Color = Color;

			// Set up type
			Manager->Types[Item] = Type;

			// Set up Attributes
			AttributeSystem* AS = Manager->AttributeSystem;		
			AS->Masks[Item] = Mask;

			// Set up Inventory... This has to be fixed and is a problem with having a general ECS
			Manager->InventorySystem->Inventories->WeaponEquipped = NULL_ENTITY;

			return Item;
		} 
		// Creates Item entity and returns eid
		eid32 CreateWeapon(Systems::EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, Masks::EntityMask Mask, 
									Component::EntityType Type, char* Name,  Enjon::Graphics::ColorRGBA16 Color)
		{
			// TODO(John): Load all of this from a scripted profile, similar to animations

			eid32 Weapon = Systems::EntitySystem::CreateEntity(Manager, 
										COMPONENT_RENDERER2D  	|  
										COMPONENT_TRANSFORM3D 	|  
										COMPONENT_ANIMATION2D	);

			// Set up Transform
			Component::Transform3D* Transform = &Manager->TransformSystem->Transforms[Weapon];
			Transform->Position = Position;
			Transform->VelocityGoal = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Transform->Velocity = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Transform->VelocityGoalScale = 0.8f;
			Transform->Dimensions = Dimensions;
			Transform->GroundPosition = Enjon::Math::Vec2(Position.XY());
			Transform->CartesianPosition = Enjon::Math::IsoToCartesian(Transform->GroundPosition);
			Transform->BaseHeight = Position.z;
			Transform->AABBPadding = Enjon::Math::Vec2(0.0f, 0.0f);
			Transform->Angle = 0.0f;
			Transform->Mass = 1.2f;

			// Set up AABB
			V2* CP = &Transform->CartesianPosition;
			V2 Min(CP->x, CP->y);
			V2 Max(CP->x + Systems::TILE_SIZE, CP->y + Systems::TILE_SIZE);
			Transform->AABB = {Min, Max};

			// Set up Animation2D
			Manager->Animation2DSystem->Animations[Weapon].Sheet = Sheet; 
			Manager->Animation2DSystem->Animations[Weapon].CurrentFrame = 0; 
			Manager->Animation2DSystem->Animations[Weapon].BeginningFrame = 0; 

			// Set up renderer component
			Manager->Renderer2DSystem->Renderers[Weapon].Format = EG::CoordinateFormat::CARTESIAN;
			
			// Set up Label
			// NOTE(John): This isn't the best way to do this; most likely will throw an error at some point
			Manager->LabelSystem->Labels[Weapon].Name = Name;

			// Set up Renderer
			Manager->Renderer2DSystem->Renderers[Weapon].Color = Color;

			// Set up type
			Manager->Types[Weapon] = Type;

			// Set up Attributes
			AttributeSystem* AS = Manager->AttributeSystem;		
			AS->Masks[Weapon] = Mask;

			// Get Weapon Profile
			auto WP = Loot::Weapon::GetProfile(std::string("Dagger"));
			AS->WeaponProfiles[Weapon] = WP;

			// Set up Inventory... This has to be fixed and is a problem with having a general ECS
			Manager->InventorySystem->Inventories->WeaponEquipped = NULL_ENTITY;

			return Weapon;
		}

		void CreateExplosion(Systems::EntityManager* Manager, EM::Vec3 Pos)
		{
			// TODO(John): Make a "spawn" function that gets called for any entity that has a factory component
			ECS::eid32 Explosion = Factory::CreateWeapon(Manager, Pos, Enjon::Math::Vec2(16.0f, 16.0f), 
														Enjon::Graphics::SpriteSheetManager::GetSpriteSheet("Orb"), 
														Masks::Type::WEAPON | Masks::WeaponOptions::EXPLOSIVE, Component::EntityType::EXPLOSIVE, "Explosion");

			Manager->Camera->ShakeScreen(Enjon::Random::Roll(30, 40));
			Manager->AttributeSystem->Masks[Explosion] |= Masks::GeneralOptions::COLLIDABLE;

			Manager->TransformSystem->Transforms[Explosion].Velocity = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Manager->TransformSystem->Transforms[Explosion].VelocityGoal = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Manager->TransformSystem->Transforms[Explosion].BaseHeight = 0.0f;
			Manager->TransformSystem->Transforms[Explosion].MaxHeight = 0.0f;

			Manager->TransformSystem->Transforms[Explosion].AABBPadding = Enjon::Math::Vec2(500, 500);
		}

		void CreateVortex(Systems::EntityManager* Manager, EM::Vec3 Pos)
		{
			// TODO(John): Make a "spawn" function that gets called for any entity that has a factory component
			ECS::eid32 Vortex = Factory::CreateWeapon(Manager, Pos, Enjon::Math::Vec2(16.0f, 16.0f), 
														Enjon::Graphics::SpriteSheetManager::GetSpriteSheet("Orb"), 
														Masks::Type::WEAPON | Masks::WeaponOptions::EXPLOSIVE, Component::EntityType::VORTEX, "Vortex");

			Manager->Camera->ShakeScreen(Enjon::Random::Roll(10, 15));
			Manager->AttributeSystem->Masks[Vortex] |= Masks::GeneralOptions::COLLIDABLE;

			Manager->TransformSystem->Transforms[Vortex].Velocity = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Manager->TransformSystem->Transforms[Vortex].VelocityGoal = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Manager->TransformSystem->Transforms[Vortex].BaseHeight = 0.0f;
			Manager->TransformSystem->Transforms[Vortex].MaxHeight = 0.0f;

			Manager->TransformSystem->Transforms[Vortex].AABBPadding = Enjon::Math::Vec2(500, 500);
		}
}}