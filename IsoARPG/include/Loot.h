#ifndef LOOT_H
#define LOOT_H

#include <System/Types.h>
#include <Math/Maths.h>

#include "ECS/ComponentSystems.h"

#include <unordered_map>

namespace Loot {

	enum Uniqueness 				{ COMMON, UNCOMMON, RARE, MAGIC, UNIQUE, LEGEND };

	typedef struct 
	{
		Enjon::uint32 Min;
		Enjon::uint32 Max;	
	} Range;


	typedef struct
	{
		Enjon::uint32 NumOfDrops;
		float ChanceToDrop;
		float LegendRate;
		float RareRate;
		float MagicRate;
		float UncommonRate;
		float CommoneRate;
	} LootProfile;

	/*-- Function Declarations --*/

	/* Creates drop profiles for entities */	
	void Init();		

	/* Gets specific profile based on entity type */	
	const LootProfile* GetProfile(std::string& N);

	/* Drops loot based on specified profile */
	void DropLootBasedOnProfile(ECS::Systems::EntityManager* Manager, ECS::eid32 E);

	void PrintCounts();

	/* Sub-namespaces */

	namespace Weapon {
	
		enum BaseType 	{ DAGGER, BOW, ARROW };
		enum ReachType 	{ UNIDIRECTION, OMNIDIRECTION };
	
		typedef struct 
		{
			Range Damage;
			float Reach;
			BaseType Base;
			ReachType Spread;  // I don't like this name... but oh well 
		} WeaponProfile;

		/* Gets specific weapon profile given */
		const WeaponProfile* GetProfile(std::string& N);

		/* Creates weapon profiles for entities */	
		void Init();
	}

	namespace Item {
	
		typedef struct 
		{
		} ItemProfile;

	}

	namespace Armor {

		enum BaseType {};
	
		typedef struct 
		{
			Range Defense;
		} ArmorProfile;

	}


}



#endif