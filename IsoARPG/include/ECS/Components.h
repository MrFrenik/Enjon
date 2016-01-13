#ifndef ENJON_COMPONENTS_H
#define ENJON_COMPONENTS_H

#include "System/Types.h"
#include "Math/Maths.h"
#include "Graphics/SpriteSheet.h"
#include "Graphics/SpriteBatch.h"
#include "IO/InputManager.h"
#include "Graphics/Color.h"
#include "Physics/AABB.h"

#include "Animation.h"

#include <map>
#include <utility>

#define MAX_ITEMS	100

namespace ECS { namespace Component {
	
	using eid32 = Enjon::uint32;
	using projectileTypeMask = Enjon::uint32;
	using EntityMask = Enjon::uint64;

	enum MaskType { PROJECTILE_MASK, PLAYER_MASK, ENEMY_MASK };
	enum EntityType { CONSUMABLE, ITEM, ENEMY, NPC, PLAYER, PROJECTILE, WEAPON };

	// Transform struct
	typedef struct
	{
		// NOTE(John): This is getting a bit bloated...
		eid32 Entity;
		Enjon::Math::Vec3 Position;
		Enjon::Math::Vec3 Velocity;
		Enjon::Math::Vec3 VelocityGoal;
		Enjon::Math::Vec2 ViewVector;
		Enjon::Math::Vec2 AttackVector;
		Enjon::Math::Vec2 CartesianPosition;
		Enjon::Physics::AABB AABB;
		Enjon::Math::Vec2 GroundPosition;
		Enjon::Math::Vec2 Dimensions;
		float BaseHeight;
		float VelocityGoalScale; 

	} Transform3D;

	// Position struct
	typedef struct
	{
		eid32 Entity;
		Enjon::Math::Vec3 IsoPosition;
		Enjon::Math::Vec3 CartesianPosition;
	} PositionComponent;

	// Velocity struct
	typedef struct
	{
		eid32 Entity;
		Enjon::Math::Vec3 Velocity;
		Enjon::Math::Vec3 VelocityGoal;
		float VelocityGoalScale;
	} VelocityComponent;

	// Inventory struct
	typedef struct 
	{
		eid32 Entity;
		std::vector<eid32> Items;
		eid32 WeaponEquipped; // NO NO NO, Terrible way of doing this!
	} InventoryComponent; 

	// Collision struct
	typedef struct
	{
		eid32 Entity;
		Enjon::Math::Vec3 CartesianCoords;
	} CollisionComponent;

	// Animation2D struct
	typedef struct
	{
		eid32 Entity;
		Enjon::Graphics::SpriteSheet* Sheet; // TODO(John): Pull all spritesheets from some kind of cache, either the resource manager or a spritesheet manager
		float AnimationTimer;
		Enjon::uint32 CurrentFrame;
		Enjon::uint32 SetStart;
		Enjon::uint32 BeginningFrame; 
		const Animation* CurrentAnimation;
	} Animation2D;

	// Label struct
	typedef struct
	{
		eid32 Entity;
		char* Name; 
	} Label;

	// PlayerController struct
	typedef struct
	{
		eid32 Entity;
		Enjon::Input::InputManager* Input; 
	} PlayerController;

	// AIController struct
	typedef struct
	{
		eid32 Entity; 
	} AIController;

	// Health Component
	typedef struct 
	{
		eid32 Entity;
		float Health;
	} HealthComponent;

	// Bitmask Component
	typedef struct 
	{
		eid32 Entity;
		std::map<MaskType, Enjon::uint32> Masks;	
	} BitmaskComponent;

	// Type Component
	typedef struct
	{
		eid32 Entity;
		EntityType Type;
	} TypeComponent;

	// Render Component
	typedef struct 
	{
		eid32 Entity;
		Enjon::Graphics::ColorRGBA8 Color;
		Enjon::Graphics::SpriteBatch* Batch;
		// NOTE(John): Should I add a spritebatch component here as well?
	} Renderer2DComponent;

	//////////////////
	// Constructors //
	//////////////////

	// Create new Transform2D Component
	inline Transform3D NewTransform3D(Enjon::Math::Vec3 Position, Enjon::Math::Vec3 Velocity, eid32 Entity, float Scale)
	{
		Transform3D Component;
		Component.Position = Position;
		Component.Velocity = Velocity;
		Component.Entity = Entity;
		Component.VelocityGoalScale = Scale;
		
		return Component;
	}

	// Create new PlayerController Component
	inline PlayerController NewPlayerController(Enjon::Input::InputManager* Input, eid32 Entity)
	{
		PlayerController Component;
		Component.Input = Input;
		Component.Entity = Entity;

		return Component;
	} 

	// Create new AIController Component
	inline AIController NewAiController(eid32 Entity)
	{ 
		AIController Component;
		Component.Entity = Entity;
	}

	// Create new Label Component
	inline Label NewLabelComponent(char* Name, eid32 Entity)
	{
		Label Component;
		Component.Name = Name;
		Component.Entity = Entity;
	} 
}}


#endif