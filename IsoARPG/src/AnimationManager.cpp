#include "AnimationManager.h"

namespace AnimationManager {

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


	// Init the AnimationManager
	void Init()
	{

	}

	// Add animations to the animation manager
	void AddAnimation()
	{

	}

	// Get specificed animation with given name
	Animation GetAnimation(const char* AnimationName)
	{
		Animation anim;


		return anim;
	}

	// Delete all animations and clear any memory used
	void DeleteAnimations()
	{

	}


}