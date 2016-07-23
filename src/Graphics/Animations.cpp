
#include "Graphics/Animations.h"

namespace Enjon { namespace Animation {  

	// Creates image frame and returns
	ImageFrame GetImageFrame(json& Frames, const std::string Name, const std::string Path)
	{
		using namespace sajson;

	    // Get an image for testing
	 //    auto image = Frames.find_object_key(literal(Name.c_str()));
	 //    //assert(image < Frames.get_length());
	 //    const auto& Image = Frames.get_object_value(image);
	   
	 //   	// Get sub objects 
	 //    const auto imageframe = Image.find_object_key(literal("frame"));
	 //    const auto& imageFrame = Image.get_object_value(imageframe);
		// const auto sss = Image.find_object_key(literal("spriteSourceSize"));
		// const auto& SSS = Image.get_object_value(sss);

		// // UV information
		// float x = imageFrame.get_value_of_key(literal("x")).get_safe_float_value();
		// float y = imageFrame.get_value_of_key(literal("y")).get_safe_float_value();
		// float z = imageFrame.get_value_of_key(literal("w")).get_safe_float_value();
		// float w = imageFrame.get_value_of_key(literal("h")).get_safe_float_value();

		// // Size information
		// EM::Vec2 SourceSize(SSS.get_value_of_key(literal("w")).get_safe_float_value(), 
		// 					SSS.get_value_of_key(literal("h")).get_safe_float_value());

		// // Return frame
		// ImageFrame IF = {	
		// 					EM::Vec4(x, y, z, w), 
		// 				  	EM::Vec2(0.0f),
		// 				  	SourceSize,
		// 				  	{},
		// 				  	0.0f,
		// 				  	1.0f, 
		// 				  	Name, 
		// 				  	0
		// 				};


		// Need to read this value from .json 
		// Also, the TextureDir needs to be formatted at beginning of program, since it's OS specific 
		// 	in the way that forward or backslashes are read
		// IF.TextureID = EI::ResourceManager::GetTexture(Path).id;

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

		// This doc will need to be passed in, but for now, just lazy load it...
	    // using sajson::literal;
	    // std::string json = EU::read_file(Path.c_str());
	    // const sajson::document& doc = sajson::parse(sajson::string(json.c_str(), json.length()));

	    // if (!doc.is_valid())
	    // {
	    //     std::cout << "Invalid json: " << doc.get_error_message() << std::endl;
	    // }

	    // // Get root and length of animation json file
	    // const auto& anim_root = doc.get_root();
	    // const auto len = anim_root.get_length();

	    // // Get handle to animation
	    // const auto anim = anim_root.find_object_key(literal(AnimName.c_str()));
	    // //assert(anim < len);
	    // const auto& Anima = anim_root.get_object_value(anim);
	    // const auto anim_len = Anima.get_length();

	    // // Get frames array and delays array
	    // std::vector<std::string> frames;

	    // // Frames
	    // const auto fr = Anima.find_object_key(literal("frames"));
	    // // assert(fr < anim_len);
	    // const auto& Frames = Anima.get_object_value(fr);
	    // const auto frames_len = Frames.get_length();
	    // frames.reserve(frames_len);

	    // // Delays
	    // const auto de = Anima.find_object_key(literal("delays"));
	    // // assert(de < anim_len);
	    // const auto& Delays = Anima.get_object_value(de);

	    // // YOffset
	    // const auto yos = Anima.find_object_key(literal("yoffsets"));
	    // // assert(yos < anim_len);
	    // const auto& YOffset = Anima.get_object_value(yos);

	    // // XOffset
	    // const auto xos = Anima.find_object_key(literal("xoffsets"));
	    // // assert(xos < anim_len);
	    // const auto& XOffset = Anima.get_object_value(xos);
	   
	    // // ImagePath
	    // const auto ipath = Anima.find_object_key(literal("image_path"));
	    // // assert(ipath < anim_len);
	    // const auto& IPath = Anima.get_object_value(ipath);

	    // Get iframe, get its delay and offsets, push into A.frames
	    // for (auto i = 0; i < frames_len; ++i)
	    // {
	    // 	auto IF = GetImageFrame(FramesDoc, Frames.get_array_element(i).get_string_value(), IPath.get_string_value());
	    // 	IF.Delay = Delays.get_array_element(i).get_safe_float_value();
	    // 	IF.Offsets.x = XOffset.get_array_element(i).get_safe_float_value();
	    // 	IF.Offsets.y = YOffset.get_array_element(i).get_safe_float_value();
	    // 	IF.TextureAtlas = atlas;

	    // 	// push back into A.frames
	    // 	A->Frames.push_back(IF);
	    // }

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
















