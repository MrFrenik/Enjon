#include "ECS/ComponentSystems.h"
#include "ECS/TestSystem.h"
#include "ECS/LabelSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/PlayerControllerSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/CollisionSystem.h"
#include "ECS/Animation2DSystem.h"
#include "ECS/InventorySystem.h"
#include "ECS/Renderer2DSystem.h"
#include "ECS/AIControllerSystem.h"
#include "Utils/Errors.h"
#include "Math/Random.h"
#include "Graphics/SpriteSheet.h"
#include "IO/ResourceManager.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>


void printDebug(char* message);

namespace ECS { namespace Systems {

	///////////////////////////
	// Entity Manager System //
	///////////////////////////

	namespace EntitySystem {

		// Creates new EntityManager and returns it
		struct EntityManager* NewEntityManager(int Width, int Height, Enjon::Graphics::Camera2D* Camera)
		{
			struct EntityManager* Manager = new EntityManager;

			if (Manager == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_ENTITY_MANAGER::Manager is null"); 

			// Initialize NextAvailableID to 0
			Manager->NextAvailableID = 0;

			// Initialize Length to 0
			Manager->Length = 0;

			// Initialize LastUsedID to 0
			Manager->MaxAvailableID = 0;

			// Initialize the component systems
			Manager->TransformSystem			= Transform::NewTransform3DSystem(Manager);
			Manager->PlayerControllerSystem 	= PlayerController::NewPlayerControllerSystem(Manager);
			Manager->AIControllerSystem			= AIController::NewAIControllerSystem(Manager);
			Manager->LabelSystem				= Label::NewLabelSystem(Manager);
			Manager->Animation2DSystem			= Animation2D::NewAnimation2DSystem(Manager);
			Manager->AttributeSystem 			= Attributes::NewAttributeSystem(Manager);
			Manager->Renderer2DSystem 			= Renderer2D::NewRenderer2DSystem(Manager);
			Manager->InventorySystem 			= Inventory::NewInventorySystem(Manager);
			Manager->CollisionSystem 			= Collision::NewCollisionSystem(Manager);

			// NOTE(John): Use this as an example for refactoring this file into separate files!
			Manager->TestSystem 				= Test::NewTestSystem(Manager);

			Manager->Width = Width;
			Manager->Height = Height;

			// Initialize spatial hash
			Manager->Grid = new SpatialHash::Grid;
			SpatialHash::Init(Manager->Grid, Manager->Width, Manager->Height);

			// Set up camera
			Manager->Camera = Camera;

			return Manager;
		} 

		// NOTE(John): Creating and Removing entities is buggy right now in the way that it manager ids
		// Creates blank entity and returns eid
		eid32 CreateEntity(struct EntityManager* Manager, bitmask32 Components)
		{
			eid32 Entity;

			// Generate a unique id for our entity if possible
			if (Manager->Length != MAX_ENTITIES) 
			{
				// Set id of entity to next available one
				Entity = Manager->NextAvailableID; 

				// Increment both NextAvailableID and Length if they're equal
				if (Manager->NextAvailableID == Manager->MaxAvailableID) 
				{
					Manager->NextAvailableID++;
					Manager->MaxAvailableID++;
				}
				else
				{
					// Otherwise set next to length, which is the greatest unused id
					Manager->NextAvailableID = Manager->MaxAvailableID;
				} 

				// Increment Length
				Manager->Length++;
				
				// Set bitfield up
				Manager->Masks[Entity] = Components;
				
				return Entity;
			}
			
			// Otherwise return MAX_ENTITIES as an error
			return MAX_ENTITIES;
		}

		// Removes selected entity from manager by setting bitfield to COMPONENT_NONE
		void RemoveEntity(struct EntityManager* Manager, eid32 Entity)
		{
			// Need to reset all of its components in here

			// Set component mask to COMPONENT_NONE to remove
			Manager->Masks[Entity] = COMPONENT_NONE;

			// Decrement length
			if (Manager->Length > 0) Manager->Length--; 

			// Set next available entity id to entity
			Manager->NextAvailableID = Entity; 
		}

		// Verifies whether or not entity is alive
		bool IsAlive(struct EntityManager* Manager, eid32 Entity)
		{
			// If alive, then its component mask will not equal COMPONENT_NONE
			return Manager->Masks[Entity] != COMPONENT_NONE;
		}

		// Turns off component
		void RemoveComponents(struct EntityManager* Manager, eid32 Entity, bitmask32 Components)
		{
			Manager->Masks[Entity] &= ~(Components);
		}

		// Creats player entity and returns eid
		eid32 CreatePlayer(struct EntityManager* Manager, Enjon::Input::InputManager* Input, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, 
						   Enjon::Graphics::SpriteSheet* Sheet, char* Name, float VelocityScale, Enjon::Math::Vec3 Velocity, Enjon::Math::Vec3 VelocityGoal, float Health, Enjon::Graphics::ColorRGBA8 Color)
		{
			// Get id for player
			eid32 Player = CreateEntity(Manager,
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

			// Set up AABB
			V2* CP = &Transform->CartesianPosition;
			V2 Min(CP->x, CP->y);
			V2 Max(CP->x + TILE_SIZE, CP->y + TILE_SIZE);
			Transform->AABB = {Min, Max};

			// Set up Input
			Manager->PlayerControllerSystem->PlayerControllers[Player].Input = Input;
			
			// Set up Animation2D
			Component::Animation2D* Animation2D = &Manager->Animation2DSystem->Animations[Player];
			Animation2D->Sheet = Sheet; 
			// These are redundant...
			Animation2D->CurrentFrame = 0; 
			Animation2D->BeginningFrame = 0;
			Animation2D->CurrentAnimation = AnimationManager::GetAnimation("Walk");
			Animation2D->SetStart = 0;

			// Set up Inventory System
			Manager->InventorySystem->Inventories[Player].Entity = Player;

			// Set up Label 
			// NOTE(John): This isn't the best way to do this; most likely will throw an error at some point
			Manager->LabelSystem->Labels[Player].Name = Name;
			Manager->LabelSystem->Labels[Player].Entity = Player;

			// Set up health component
			Manager->AttributeSystem->HealthComponents[Player].Health = Health;
			Manager->AttributeSystem->HealthComponents[Player].Entity = Player;

			// Set up Renderer
			Manager->Renderer2DSystem->Renderers[Player].Color = Color;

			// Set up Type
			Manager->Types[Player] = Component::EntityType::PLAYER;

			// Set up inventory
			Manager->InventorySystem->Inventories[Player].WeaponEquipped = NULL_ENTITY; // Set out of range of being processed
			
			return Player;
		} 

		// Creates ai entity and returns eid
		eid32 CreateAI(struct EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, char* Name, float VelocityScale,
						 Enjon::Math::Vec3 Velocity, Enjon::Math::Vec3 VelocityGoal, float Health, Enjon::Graphics::ColorRGBA8 Color)
		{
			// Get id for ai
			eid32 AI = CreateEntity(Manager, 
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

			// Set up AABB
			V2* CP = &Transform->CartesianPosition;
			V2 Min(CP->x, CP->y);
			V2 Max(CP->x + TILE_SIZE, CP->y + TILE_SIZE);
			Transform->AABB = {Min, Max};

			// Set up Animation2D
			Component::Animation2D* Animation2D = &Manager->Animation2DSystem->Animations[AI];
			Animation2D->Sheet = Sheet; 
			// These are redundant...
			Animation2D->CurrentFrame = 0; 
			Animation2D->BeginningFrame = 0; 

			
			// Set up Label
			// NOTE(John): This isn't the best way to do this; most likely will throw an error at some point
			Manager->LabelSystem->Labels[AI].Name = Name;

			// Set up Attributes
			Manager->AttributeSystem->HealthComponents[AI].Health = Health;
			Manager->AttributeSystem->HealthComponents[AI].Entity = AI;
			Manager->AttributeSystem->Masks[AI] |= Masks::Type::AI;

			// Set up Renderer
			Manager->Renderer2DSystem->Renderers[AI].Color = Color;

			// Set up type
			Manager->Types[AI] = Component::EntityType::ENEMY;

			// Set up inventory
			Manager->InventorySystem->Inventories[AI].WeaponEquipped = NULL_ENTITY;

			return AI;
		} 

		// Creates ai entity and returns eid
		eid32 CreateItem(struct EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, Masks::EntityMask Mask, 
								Component::EntityType Type, char* Name, Enjon::Graphics::ColorRGBA8 Color)
		{
			// Get id for ai
			eid32 Item = CreateEntity(Manager, 
									COMPONENT_RENDERER2D	| 
									COMPONENT_TRANSFORM3D 	| 
									COMPONENT_ANIMATION2D 	| 
									COMPONENT_LABEL 		);

			// Set up Transform
			Component::Transform3D* Transform = &Manager->TransformSystem->Transforms[Item];
			Transform->Position = Position;
			Transform->VelocityGoal = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Transform->Velocity = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Transform->Dimensions = Dimensions;
			Transform->GroundPosition = Enjon::Math::Vec2(Position.XY());
			Transform->CartesianPosition = Enjon::Math::IsoToCartesian(Transform->GroundPosition);
			Transform->BaseHeight = Position.z;

			// Set up AABB
			V2* CP = &Transform->CartesianPosition;
			V2 Min(CP->x, CP->y);
			V2 Max(CP->x + TILE_SIZE, CP->y + TILE_SIZE);
			Transform->AABB = {Min, Max};

			// Set up Animation2D
			Manager->Animation2DSystem->Animations[Item].Sheet = Sheet; 
			Manager->Animation2DSystem->Animations[Item].CurrentFrame = 0; 
			Manager->Animation2DSystem->Animations[Item].BeginningFrame = 0; 
			
			// Set up Label
			// NOTE(John): This isn't the best way to do this; most likely will throw an error at some point
			Manager->LabelSystem->Labels[Item].Name = Name;

			// Set up Renderer
			Manager->Renderer2DSystem->Renderers[Item].Color = Color;

			// Set up type
			Manager->Types[Item] = Type;

			// Set up Attributes
			Manager->AttributeSystem->Masks[Item] = Mask;

			// Set up Inventory... This has to be fixed and is a problem with having a general ECS
			Manager->InventorySystem->Inventories->WeaponEquipped = NULL_ENTITY;

			return Item;
		} 

	} // namespace EntitySystem
	
	////////////////////
	// RendererSystem //
	////////////////////

	// Reponsible for managing all the renderers
	// namespace Renderer2D
	// {
	// 	// Updates Renderers of EntityManager
	// 	void Update(struct EntityManager* Manager)
	// 	{
	// 		// Need to render components here
	// 		for (eid32 e = 0; e < Manager->MaxAvailableID; e++)
	// 		{

	// 		}
	// 	}

	// 	// Create new Render2DSystem
	// 	Renderer2DSystem* NewRenderer2DSystem(struct EntityManager* Manager)
	// 	{
	// 		struct Renderer2DSystem* System = new Renderer2DSystem;
	// 		if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_RENDERER2D_SYSTEM::System is null"); 
	// 		System->Manager = Manager;
	// 		return System;
	// 	}
	// } 
	
	// namespace AIController
	// {
	// 	struct AIControllerSystem* NewAIControllerSystem(struct EntityManager* Manager)
	// 	{ 
	// 		struct AIControllerSystem* System = new AIControllerSystem;
	// 		System->Manager = Manager;
	// 		return System;
	// 	}

	// 	// Updates Controller of AI it is attached to
	// 	void Update(struct AIControllerSystem* System, eid32 Player)
	// 	{
	// 		struct EntityManager* Manager = System->Manager;
	// 		const Component::Transform3D* Target = &Manager->TransformSystem->Transforms[Player];
	// 		// Let's just make it go towards the player for testing
	// 		for (eid32 ai = 0; ai < Manager->MaxAvailableID; ai++)
	// 		{
	// 			// Check to see if entity has ai controller
	// 			if ((Manager->Masks[ai] & (COMPONENT_AICONTROLLER | COMPONENT_TRANSFORM3D)) == (COMPONENT_AICONTROLLER | COMPONENT_TRANSFORM3D))
	// 			{
	// 				Component::Transform3D* AI = &Manager->TransformSystem->Transforms[ai];

	// 				// Find difference in positions	
	// 				Enjon::Math::Vec3 Difference = Enjon::Math::Vec3::Normalize(Enjon::Math::Vec3(Target->CartesianPosition, Target->Position.z) - Enjon::Math::Vec3(AI->CartesianPosition, AI->Position.z));

	// 				Enjon::Math::Vec2 a = Target->GroundPosition;
	// 				Enjon::Math::Vec2 b = AI->GroundPosition;
	// 				float distance = a.DistanceTo(b);

	// 				if (distance <= 60.0f) AI->VelocityGoal = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
	// 				// Move towards player
	// 				// TODO(John): Come up with some kind of passed in speed parameter to multiply by the difference vector
	// 				//AI->Velocity = Difference; 
	// 				else AI->VelocityGoal = Difference * 2.0f;
	// 			}
	// 		}
	// 	}
	// }
}}

void printDebug(char* message)
{
	static int counter = 0.0f;
	counter += 0.1f;
	if (counter > 2.0f)
	{
		printf(message);
		counter = 0.0f;
	}
}



