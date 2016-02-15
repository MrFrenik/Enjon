#include "AnimationManager.h"

#include <unordered_map>

namespace AnimationManager {

	// NOTE(John): I'm too fucking tired / lazy to do this correctly right now, so just fix it later...

	typedef struct
	{
		const char* name;
		struct Animation anim; 
	} AnimationPair;

	typedef struct 
	{
		AnimationPair* pairs;
		int size;
		int capacity;
	} Animations;

	typedef struct 
	{
		Animation Walk;
		Animation Attack_Dagger;
		Animation Attack_Bow;
	} Player_Animations;

	Player_Animations PlayerAnims;

	/* Map of player animations */	
	std::unordered_map<std::string, Animation> PlayerAnimations;

	/* Init the AnimationManager */
	void Init()
	{
		// Player animations
		AddAnimation(PlayerAnimations, std::string("walk"), CreateAnimation(CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "walk")));
		AddAnimation(PlayerAnimations, std::string("attack_dagger"), CreateAnimation(CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "attack_dagger")));
		AddAnimation(PlayerAnimations, std::string("attack_bow"), CreateAnimation(CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "attack_bow")));
	}

	/* Add animations to the animation manager */
	void AddAnimation(std::unordered_map<std::string, Animation>& M, std::string N, Animation A)
	{
		// Insert into map
		// NOTE(John): Will overwrite the pre-existing kv pair if already exists!
		M[N] = A;
	}

	/* Get specificed animation with given name */
	const Animation* GetAnimation(std::string M, std::string AnimationName)
	{
		if (M == "Player")
		{
			auto it = PlayerAnimations.find(AnimationName);
			if (it != PlayerAnimations.end()) return &it->second;
			else return &PlayerAnimations["walk"];
		}
		
		else return &PlayerAnimations["walk"];
	}

	/* Delete all animations and clear any memory used */
	void DeleteAnimations()
	{

	}


}