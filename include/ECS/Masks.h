#ifndef ENJON_MASKS_H
#define ENJON_MASKS_H

#include "System/Types.h"

namespace ECS { namespace Masks {

	using EntityMask = Enjon::uint64;

	enum Type : EntityMask 
	{
		PLAYER 			= 0x0000000000000000,
		AI 				= 0x1000000000000000, 
		ITEM 			= 0x2000000000000000, 
		WEAPON  		= 0x4000000000000000,
		PROP			= 0x8000000000000000
	};

	enum WeaponOptions : EntityMask
	{
		MELEE			= 0x0100000000000000, 
		PROJECTILE 		= 0x0200000000000000,
		RANGED 			= 0x0400000000000000, 
		EXPLOSIVE		= 0x0800000000000000	
	};

	enum WeaponSubOptions : EntityMask
	{
		GRENADE 		= 0x0010000000000000
	};

	enum ItemOptions : EntityMask
	{
		CONSUMABLE 			= 0x0100000000000000 
	};

	enum PlayerOptions : EntityMask
	{
		
	};

	enum AIOptions : EntityMask
	{
		
	};

	enum GeneralOptions: EntityMask
	{
		RENDERABLE		= 0x0000000000000001,
		COLLIDABLE		= 0x0000000000000002,
		EQUIPPED 		= 0x0000000000000004, 
		PICKED_UP		= 0x0000000000000008, 
		DAMAGED			= 0x0000000000000010, 
		RISING 			= 0x0000000000000020, 
		EXPLODED		= 0x0000000000000040,
		BOUNCES			= 0x0000000000000080, 
		DEBRIS			= 0x0000000000000100
	};
}}

#endif