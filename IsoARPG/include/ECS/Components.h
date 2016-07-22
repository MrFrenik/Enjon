#ifndef ENJON_COMPONENTS_H
#define ENJON_COMPONENTS_H

#include "System/Types.h"
#include "Math/Maths.h"
#include "Graphics/SpriteSheet.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/Animations.h"
#include "BehaviorTreeManager.h"
#include "IO/InputManager.h"
#include "Graphics/Color.h"
#include "Physics/AABB.h"
#include "Defines.h"

#include "EnjonAnimation.h"
#include "AnimManager.h"

#include <unordered_map>
#include <utility>

#define MAX_ITEMS	5000

namespace ECS { namespace Component {
	
	using eid32 = Enjon::uint32;
	using projectileTypeMask = Enjon::uint32;
	using EntityMask = Enjon::uint64;

	enum MaskType 					{ PROJECTILE_MASK, PLAYER_MASK, ENEMY_MASK };
	enum EntityType 				{ CONSUMABLE, ITEM, ENEMY, NPC, PLAYER, PROJECTILE, WEAPON, EXPLOSIVE, PROP, VORTEX};
	enum WeaponType 				{ AXE, DAGGER, BOW }; 												// These are obviously not thorough...
	enum ArmorType 					{ HELM, CHEST, ARMS, LEGS, BOOTS };									// Neither are these...
	enum StatsType 					{ WEAPONTYPE, ARMORTYPE };

	// Transform struct
	typedef struct
	{
		// NOTE(John): This is getting a bit bloated...
		// NOTE(John): Yep, make a physics component
		Enjon::Math::Vec3 Position;
		Enjon::Math::Vec3 Velocity;
		Enjon::Math::Vec3 VelocityGoal;
		Enjon::Math::Vec2 ViewVector;
		Enjon::Math::Vec2 AttackVector;
		Enjon::Math::Vec2 CartesianPosition;
		Enjon::Physics::AABB AABB;
		Enjon::Math::Vec2 AABBPadding;
		Enjon::Math::Vec2 GroundPosition;
		Enjon::Math::Vec2 Dimensions;
		Enjon::Math::Vec2 GroundPositionOffset;
		float Angle;
		float BaseHeight;
		float MaxHeight;
		float VelocityGoalScale; 
		float Mass;
		eid32 Entity;
	} Transform3D;

	// Position struct
	typedef struct
	{
		Enjon::Math::Vec3 IsoPosition;
		Enjon::Math::Vec3 CartesianPosition;
		eid32 Entity;
	} PositionComponent;

	// Velocity struct
	typedef struct
	{
		Enjon::Math::Vec3 Velocity;
		Enjon::Math::Vec3 VelocityGoal;
		float VelocityGoalScale;
		eid32 Entity;
	} VelocityComponent;

	// Inventory struct
	typedef struct 
	{
		std::vector<eid32> Items;
		eid32 WeaponEquipped; // NO NO NO, Terrible way of doing this!
		eid32 Entity;
	} InventoryComponent; 

	// Collision struct
	typedef struct
	{
		Enjon::Math::Vec3 CartesianCoords;
		eid32 Entity;
	} CollisionComponent;

	// Animation2D struct
	typedef struct
	{
		Enjon::Graphics::SpriteSheet* Sheet; // TODO(John): Pull all spritesheets from some kind of cache, either the resource manager or a spritesheet manager
		float AnimationTimer;
		Enjon::uint32 CurrentFrame;
		Enjon::uint32 SetStart;
		Enjon::uint32 BeginningFrame; 
		const Animate::Animation* CurrentAnimation;
		eid32 Entity;
	} Animation2D;	

	// AnimComponent
	typedef struct 
	{
		float AnimationTimer;
		Enjon::uint32 CurrentIndex;
		Enjon::uint32 SetStart;
		const EA::Anim* CurrentAnimation;
		eid32 Entity;
	} AnimComponent;

	// Label struct
	typedef struct
	{
		std::string Name; 
		eid32 Entity;
	} Label;

	// PlayerController struct
	typedef struct
	{
		Enjon::Input::InputManager* Input; 
		eid32 Entity;
	} PlayerController;

	// AIController struct
	typedef struct
	{
		eid32 Entity; 
		BT::BehaviorTree* Brain;
		BT::StateObject* SO;
		BT::BlackBoard* BB;
	} AIController;

	// Health Component
	typedef struct 
	{
		float Health;
		eid32 Entity;
	} HealthComponent;

	// Bitmask Component
	typedef struct 
	{
		std::unordered_map<MaskType, Enjon::uint32> Masks;	
		eid32 Entity;
	} BitmaskComponent;

	// Type Component
	typedef struct
	{
		EntityType Type;
		eid32 Entity;
	} TypeComponent;

	// Group Component
	typedef struct 
	{
		eid32 Entity;
		eid32 Parent;	
	} GroupComponent;

	// Render Component
	typedef struct 
	{
		Enjon::Graphics::ColorRGBA16 Color;
		Enjon::Graphics::SpriteBatch* Batch;
		EG::CoordinateFormat Format;
		eid32 Entity;
		// NOTE(John): Should I add a spritebatch component here as well?
	} Renderer2DComponent;

	// Timer Component
	typedef struct 
	{
		float CurrentTime;
		float DT;
		eid32 Entity;
	} TimerComponent;

	// Weapon Component
	typedef struct 
	{
		float MinDamage;
		float MaxDamage;
	} DamageComponent;
	

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