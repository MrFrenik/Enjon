#include "AnimationManager.h"

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

	// Init the AnimationManager
	void Init()
	{
		// Player animations
		PlayerAnims.Walk = CreateAnimation(CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "walk"));
		PlayerAnims.Attack_Dagger = CreateAnimation(CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "attack_dagger"));
		PlayerAnims.Attack_Bow = CreateAnimation(CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "attack_bow"));
	}

	// Add animations to the animation manager
	void AddAnimation()
	{

	}

	// Get specificed animation with given name
	const Animation* GetAnimation(const char* AnimationName)
	{
		if (strcmp(AnimationName, "Walk") == 0) return &PlayerAnims.Walk;
		else if (strcmp(AnimationName, "Attack_Dagger") == 0) return &PlayerAnims.Attack_Dagger;
		else if (strcmp(AnimationName, "Attack_Bow") == 0) return &PlayerAnims.Attack_Bow;
		else return &PlayerAnims.Walk;		
	}

	// Delete all animations and clear any memory used
	void DeleteAnimations()
	{

	}


}