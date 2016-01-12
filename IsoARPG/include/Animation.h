#ifndef ANIMATION_H
#define ANIMATION_H 

#include <stdlib.h>
#include <Utils/FileUtils.h>
#include <Utils/Errors.h>

#define ORIENTATIONS	8 

enum Orientation { NW, NE, E, W, N, S, SE, SW};

struct AnimationProfile
{ 
	int FrameCount;
	char* FilePath;
	int Starts[ORIENTATIONS];
	float* Delays;
};

struct Animation
{
	struct AnimationProfile* Profile;
	int BeginningFrame;
	float AnimationTimer;
	float AnimationTimerIncrement;
}; 

typedef struct Animation Animation;

struct AnimationProfile* CreateAnimationProfile(char* ProfileFilePath, char* AnimationName);
struct Animation CreateAnimation(struct AnimationProfile* Profile, float AnimationTimerIncrement = 0.1f, Orientation Orient = Orientation::NW);


#endif