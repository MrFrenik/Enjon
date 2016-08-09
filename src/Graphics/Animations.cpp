
#include "Graphics/Animations.h"

namespace Enjon { namespace Animation {  

	using json = nlohmann::json;

	// Creates image frame and returns
	ImageFrame GetImageFrame(json& Frames, const std::string Name, const std::string Path)
	{
			// Get image
		auto Image = Frames.at(Name);

		// Get sub objects
		const auto IFrame = Image.at("frame");
		const auto SSS = Image.at("spriteSourceSize");

		// Get UV information
		float x = IFrame.at("x");
		float y = IFrame.at("y");
		float w = IFrame.at("w");
		float h = IFrame.at("h");

		// Size information
		float s_w = SSS.at("w");
		float s_h = SSS.at("h");
		EM::Vec2 SourceSize(s_w, s_h);

		// Create ImageFrame
		ImageFrame IF = 
						{
							EM::Vec4(x, y, w, h),
							EM::Vec2(0.0f),
							SourceSize,
							{},
							0.0f, 
							1.0f,
							Name, 
							0
						};

		IF.TextureID = EI::ResourceManager::GetTexture(Path).id;


		// ImageFrame IF;
		return IF;
	}

	// Creates animation and returns
	Anim* CreateAnimation(const std::string& AnimName, json& FramesDoc, EA::Atlas atlas, const std::string Path)
	{
		// need to parse the file for a specific animatioa
		Anim* A = new Anim;

		auto Json = EU::read_file_sstream(Path.c_str());

		// fill a stream with JSON text
		std::stringstream ss;
		ss << Json;
		    
		// parse and serialize JSON
		json j_complete = json::parse(Json);

		// Get handle to animation
		auto Anima = j_complete.at(AnimName);

		// Get Frames
		const auto Frames = Anima.at("frames");

		// Get Delays
		const auto Delays = Anima.at("delays");

		// Get XOffsets
		const auto XOffset = Anima.at("xoffsets");

		// Get YOffsets
		const auto YOffset = Anima.at("yoffsets");

		// Get Image Path
		const auto IPath = Anima.at("image_path");

		// Get File Path
		const auto FPath = Anima.at("file_path");

		// Get Length of frames
		const auto frames_len = Frames.size(); 

	    // Get iframe, get its delay and offsets, push into A.frames
	    for (auto i = 0; i < frames_len; ++i)
	    {
	    	auto IF = GetImageFrame(FramesDoc, Frames.at(i), IPath);
	    	IF.Delay = Delays.at(i);
	    	IF.Offsets.x = XOffset.at(i);
	    	IF.Offsets.y = YOffset.at(i);
	    	IF.TextureAtlas = atlas;

	    	// push back into A.frames
	    	A->Frames.push_back(IF);
	    }



	    // Get total number of frames in vector
	    A->TotalFrames = A->Frames.size();

	    // Set animation name
	    A->Name = AnimName;

	    A->FilePath = FPath;

		return A;
	}

	// Draws a single animation frame	
	void DrawFrame(const ImageFrame& Image, EM::Vec2 Position, EG::SpriteBatch* Batch, const EG::ColorRGBA16& Color, float ScalingFactor, float Depth)
	{
		auto& Dims = Image.UVs;
		auto& SSize = Image.SourceSize;
		auto& Offsets = Image.Offsets;
		auto& A = Image.TextureAtlas;

		auto AWidth = A.AtlasSize.x;
		auto AHeight = A.AtlasSize.y;

		Batch->Add(
					EM::Vec4(Position.x + Offsets.x * ScalingFactor, 
							Position.y + Offsets.y * ScalingFactor, 
							EM::Vec2(SSize.x, SSize.y) * ScalingFactor), 
					EM::Vec4(Dims.x / AWidth, 
							(AHeight - Dims.y - Dims.w) / AHeight, 
							 Dims.z / AWidth, 
							 Dims.w / AHeight), 
					A.Texture.id, 
					Color,
					Depth
				  );
	}

}}
















