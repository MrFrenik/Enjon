#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include "IO/ResourceManager.h"

#include "Defines.h"

#include <vector>
#include <string>

typedef struct
{
	EM::Vec4 UVs;
	EM::Vec2 Offsets;
	EM::Vec2 SourceSize;
	float Delay;
	float ScalingFactor;
	const std::string Name;
	GLuint TextureID;
} ImageFrame;

typedef struct 
{
	EM::Vec2 AtlasSize;
	EG::GLTexture Texture;	
} Atlas;

typedef struct 
{
	std::vector<ImageFrame> Frames;
	uint32_t TotalFrames;
	std::string Name;
} Anim;


#endif