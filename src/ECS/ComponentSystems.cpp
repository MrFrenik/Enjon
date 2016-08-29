#include "ECS/ComponentSystems.h"
#include "ECS/LabelSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/PlayerControllerSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/CollisionSystem.h"
#include "ECS/Animation2DSystem.h"
#include "ECS/InventorySystem.h"
#include "ECS/Renderer2DSystem.h"
#include "ECS/AIControllerSystem.h"
#include "ECS/EffectSystem.h"
#include "Utils/Errors.h"
#include "Math/Random.h"
#include "Graphics/SpriteSheet.h"
#include "IO/ResourceManager.h"
#include "Defines.h"


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>


void printDebug(char* message);

using namespace Enjon;

namespace ECS { namespace Systems {


	///////////////////////////
	// Entity Manager System //
	///////////////////////////

	namespace EntitySystem {

		// World
		EntityManager WorldObject;

		eid32 FindNextAvailableEntity(EntityManager* Manager);

		// Sets up WorldObject and returns it
		struct EntityManager* NewEntityManager(int Width, int Height, EG::Camera2D* Camera, Level* Lvl)
		{
			// struct EntityManager* Manager = new EntityManager;

			// if (Manager == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_ENTITY_MANAGER::Manager is null"); 

			// Set up world object

			// Initialize NextAvailableID to 0
			WorldObject.NextAvailableID = 0;

			// Initialize Length to 0
			WorldObject.Length = 0;

			// Initialize LastUsedID to 0
			WorldObject.MaxAvailableID = 0;

			// Initialize LastUsedID to 0
			WorldObject.MinID = 0;

			// Initialize the component systems
			WorldObject.TransformSystem			= Transform::NewTransform3DSystem(&WorldObject);
			WorldObject.PlayerControllerSystem 	= PlayerController::NewPlayerControllerSystem(&WorldObject);
			WorldObject.AIControllerSystem		= AIController::NewAIControllerSystem(&WorldObject);
			WorldObject.LabelSystem				= Label::NewLabelSystem(&WorldObject);
			WorldObject.Animation2DSystem		= Animation2D::NewAnimation2DSystem(&WorldObject);
			WorldObject.AttributeSystem 		= Attributes::NewAttributeSystem(&WorldObject);
			WorldObject.Renderer2DSystem 		= Renderer2D::NewRenderer2DSystem(&WorldObject);
			WorldObject.InventorySystem 		= Inventory::NewInventorySystem(&WorldObject);
			WorldObject.CollisionSystem 		= Collision::NewCollisionSystem(&WorldObject);
			WorldObject.EffectSystem			= Effect::NewEffectSystem(&WorldObject);
			WorldObject.ParticleEngine 			= EG::Particle2D::NewParticleEngine();

			WorldObject.Width = Width;
			WorldObject.Height = Height;

			// Initialize spatial hash
			WorldObject.Grid = new SpatialHash::Grid;
			SpatialHash::Init(WorldObject.Grid, WorldObject.Width, WorldObject.Height);

			// Set up camera
			WorldObject.Camera = Camera;

			// Set level
			WorldObject.Lvl = Lvl;

			return &WorldObject;
		}

		eid32 CreateEntity(struct EntityManager* Manager, bitmask32 Components)
		{
			eid32 Entity = FindNextAvailableEntity(Manager);

			// Find next available ID and assign to entity
			if (Entity < MAX_ENTITIES)
			{
				// Increment Length
				Manager->Length++;

				Manager->Entities.push_back(Entity);

				// Set bitfield up
				Manager->Masks[Entity] = Components;

				// Set max id
				if (Manager->MaxAvailableID < Manager->NextAvailableID) Manager->MaxAvailableID = Manager->NextAvailableID;

				return Entity;	
			}
			
			// Otherwise return MAX_ENTITIES as an error
			return MAX_ENTITIES;
		}

		eid32 FindNextAvailableEntity(EntityManager* Manager)
		{
			// Get next available id
			auto NAID = Manager->NextAvailableID;
			
			for (auto i = NAID; i < MAX_ENTITIES - 1; ++i)
			{
				if (Manager->Masks[i] == COMPONENT_NONE)
				{
					Manager->NextAvailableID = i+1;
					return i;
				}
			}

			// Loop from beginning to NAID - 1
			for (auto i = 0; i < NAID - 2; ++i)
			{
				if (Manager->Masks[i] == COMPONENT_NONE)
				{
					Manager->NextAvailableID = i+1;
					return i;
				}
			}

			// Not sure about this...
			Manager->NextAvailableID = MAX_ENTITIES;
			return MAX_ENTITIES;
		}

		// Removes selected entity from manager by setting bitfield to COMPONENT_NONE
		void RemoveEntity(struct EntityManager* Manager, eid32 Entity)
		{
			// Need to reset/remove all of its components in here
			// TODO(John): Keep track of all its components (maps) and then remove all of those
			if (Manager->AttributeSystem->Masks[Entity] & Masks::Type::WEAPON) 
			{
				// Manager->AttributeSystem->WeaponProfiles.erase(Entity);
			}

			// Get handle to entity mask
			auto Mask = Manager->Masks[Entity];

			// Reset Transform component
			if (Mask & COMPONENT_TRANSFORM3D)
			{
				Transform::Reset(Manager, Entity);
			}

			// Reset AIController component
			if (Mask & COMPONENT_AICONTROLLER)
			{
				AIController::Reset(Manager, Entity);
			}

			// Reset Attribute System
			Attributes::Reset(Manager, Entity);

			// Set component mask to COMPONENT_NONE to remove
			Manager->Masks[Entity] = COMPONENT_NONE;

			// Not sure about that...
			if (Entity < Manager->MinID) Manager->MinID = Entity;

			// Decrement length
			if (Manager->Length > 0) Manager->Length--; 

			// Pop entity
			if (Manager->Entities.size()) Manager->Entities.pop_back();

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

		struct EntityManager* World()
		{
			// Return current world object
			return &WorldObject;
		}

		// Destroys current world object
		void DestroyWorld()
		{

		}

	} // namespace EntitySystem
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



