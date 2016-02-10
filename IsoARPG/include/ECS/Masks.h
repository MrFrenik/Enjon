#ifndef MASKS_H
#define MASKS_H

#include "System/Types.h"

namespace ECS { namespace Masks {

	using EntityMask = Enjon::uint64;

	enum Type : EntityMask 
	{
		PLAYER 			= 0x0000000000000000,
		AI 				= 0x1000000000000000, 
		ITEM 			= 0x2000000000000000, 
		WEAPON  		= 0x4000000000000000
	};

	enum WeaponOptions : EntityMask
	{
		MELEE			= 0x0000000000000000, 
		PROJECTILE 		= 0x0100000000000000,
		RANGED 			= 0x0200000000000000
	};

	enum WeaponSubOptions : EntityMask
	{

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
		DAMAGED			= 0x0000000000000010
	};
}}

#endif