#ifndef COMPONENT_SYSTEMS_H
#define COMPONENT_SYSTEMS_H

#include <SDL2/SDL.h> 

#include "Enjon.h"

#include "ECS/Components.h"
#include "ECS/Entity.h"

#include "Masks.h"
#include "SpatialHash.h"
#include "AnimationManager.h"

#include <vector>

#define SCREEN_VERTICAL_CENTER			1440 / 2
#define SCREEN_HORIZONTAL_CENTER		900 / 2


struct TestSystem;
struct LabelSystem;
struct AttributeSystem;
struct PlayerControllerSystem;
struct Transform3DSystem;
struct CollisionSystem;
struct Animation2DSystem;
struct InventorySystem;
struct Renderer2DSystem;
struct AIControllerSystem;
struct EffectSystem;

// EntityManagers hold pointers to their system, which hold the arrays of data
// ComponentSystems don't need to know of mangers...or do they? Yes, they'll hold pointers to the managers of which they belong
namespace ECS { namespace Systems {

	const float TILE_SIZE = 32.0f;
	
	struct EntityManager
	{
		eid32 NextAvailableID;
		eid32 MaxAvailableID;
		eid32 Length;

		struct Transform3DSystem* TransformSystem;
		struct AIControllerSystem* AIControllerSystem;
		struct PlayerControllerSystem* PlayerControllerSystem;
		struct Animation2DSystem* Animation2DSystem;
		struct LabelSystem* LabelSystem;
		struct CollisionSystem* CollisionSystem;
		struct AttributeSystem* AttributeSystem;
		struct Renderer2DSystem* Renderer2DSystem;
		struct InventorySystem* InventorySystem;
		struct EffectSystem* EffectSystem;
		EG::Particle2D::ParticleEngine2D* ParticleEngine;

		bitmask32 Masks[MAX_ENTITIES];
		Component::EntityType Types[MAX_ENTITIES];

		eid32 Player;

		int Width; 
		int Height;

		SpatialHash::Grid* Grid;
		Enjon::Graphics::Camera2D* Camera;
	};

	////////////////////
	// Entity Manager //
	////////////////////
	
	namespace EntitySystem 
	{ 
		// Creates and returns instance of an Entity Manager
		// TODO(John): Write custom allocators instead of using malloc/delete
		struct EntityManager* NewEntityManager(int Width, int Height, Enjon::Graphics::Camera2D* Camera); 
		
		// Creates a blank entity, returns the eid and places in manager
		eid32 CreateEntity(struct EntityManager* Manager, bitmask32 Components);

		// Removes entity from manager by setting its bitfield to COMPONENT_NONE
		void RemoveEntity(struct EntityManager* Manager, eid32 Entity); 

		// Returns whether or not entity's bitfield is set to COMPONENT_NONE or not
		bool IsAlive(struct EntityManager* Manager, eid32 Entity);

		/* Turns off component from entity by bitwise ^= */
		void RemoveComponents(struct EntityManager* Manager, eid32 Entity, bitmask32 Components);

		/* Adds components to entity by bitwise | */
		void AddComponents(bitmask32 Components);
	}
}}



#endif