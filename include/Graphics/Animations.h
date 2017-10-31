#ifndef ENJON_ANIMATIONS_H
#define ENJON_ANIMATIONS_H

/*-- External/Engine Libraries includes --*/
#include "Graphics/GLTexture.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/Color.h"
#include "IO/ResourceManager.h"
#include "Math/Vec2.h"
#include "Utils/FileUtils.h"
#include "Utils/json.h"

// #include <sajson/sajson.h>

#include <vector>
#include <string>

namespace Enjon { namespace Animation {
	
	typedef struct 
	{
		Enjon::Vec2 AtlasSize;
		GLTexture Texture;	
	} Atlas;

	typedef struct
	{
		Vec4 UVs;
		Enjon::Vec2 Offsets;
		Enjon::Vec2 SourceSize;
		Atlas TextureAtlas;
		float Delay;
		float ScalingFactor;
		const std::string Name;
		GLuint TextureID;
	} ImageFrame;

	typedef struct 
	{
		std::vector<ImageFrame> Frames;
		uint32_t TotalFrames;
		std::string FilePath;
		std::string Name;
	} Anim;

	// Creates image frame and returns
	ImageFrame GetImageFrame(nlohmann::json& Frames, const std::string Name, const std::string Path);

	// Creates animation and returns
	Anim* CreateAnimation(const std::string& AnimName, nlohmann::json& FramesDoc, EA::Atlas atlas, const std::string Path);
	
	// Draws a single animation frame	
	void DrawFrame(const ImageFrame& Image, Enjon::Vec2 Position, SpriteBatch* Batch, const ColorRGBA32& Color = RGBA32_White(), float ScalingFactor = 1.0f, float Depth = 0.0f);

}}



#endif