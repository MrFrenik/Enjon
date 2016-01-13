#ifndef MASKS_H
#define MASKS_H

#include "System/Types.h"

namespace ECS { namespace Masks {

	using EntityMask = Enjon::uint64;

	enum Entity_Type : EntityMask 
	{
		PLAYER 			= 0x0000000000000000,
		AI 				= 0x1000000000000000, 
		ITEM 			= 0x2000000000000000, 
		WEAPON  		= 0x4000000000000000
	};

	enum Weapon_Options : EntityMask
	{
		MELEE			= 0x0000000000000000, 
		PROJECTILE 		= 0x0000000000000001
	};

	enum Item_Options : EntityMask
	{
		CONSUMABLE 		= 0x0000000000000000		
	};

	enum Player_Options : EntityMask
	{

	};

	enum AI_Options : EntityMask
	{

	};


}}

#endif