
#include "AnimManager.h"

#include <Utils/FileUtils.h>
#include <Utils/Errors.h>
#include <Utils/json.h>


namespace AnimManager {

    // using sajson::literal;
	using json = nlohmann::json;
	
	json InitFrames(EA::Atlas& atlas);

	const std::string AnimTextureDir("../IsoARPG/Assets/Textures/Animations/Player/Attack/OH_L/SE/Player_Attack_OH_L_SE.png");
	const std::string AnimTextureJSONDir("../IsoARPG/Assets/Textures/Animations/Player/Attack/OH_L/SE/Player_Attack_OH_L_SE.json");
	const std::string AnimationDir("../IsoARPG/Profiles/Animations/Player/PlayerAttackOHLSEAnimation.json");
	std::unordered_map<std::string, EA::Anim*> Animations;

	// Init the AnimationManager
	void Init()
	{
		EA::Atlas atlas;
		auto Frames = InitFrames(atlas);

		// Add animations
		AddAnimation(Animations, "Player_Attack_OH_L_SE", EA::CreateAnimation("Player_Attack_OH_L_SE", Frames, atlas, AnimationDir));
		AddAnimation(Animations, "Player_Attack_OH_R_SE", EA::CreateAnimation("Player_Attack_OH_R_SE", Frames, atlas, AnimationDir));
		AddAnimation(Animations, "Player_Attack_OH_L_SW", EA::CreateAnimation("Player_Attack_OH_L_SW", Frames, atlas, AnimationDir));
	}

	// Add animations to the animation manager
	void AddAnimation(std::unordered_map<std::string, EA::Anim*>& M, std::string N, EA::Anim* A)
	{
		// Insert into map
		// NOTE(John): Will overwrite the pre-existing kv pair if already exists!
		M[N] = A;
	}

	// Get specificed animation with given name
	EA::Anim* GetAnimation(std::string AnimationName)
	{
			auto it = Animations.find(AnimationName);
			if (it != Animations.end()) return it->second;
			else EU::FatalError("Cannot find Animation: " + AnimationName);
			return nullptr;
	}

	// Delete all animations and clear any memory used
	void DeleteAnimations()
	{

	}

	json InitFrames(EA::Atlas& atlas)
	{
	    // std::string json = EU::read_file_sstream(AnimTextureJSONDir.c_str());
	    // const sajson::document& doc = sajson::parse(sajson::string(json.c_str(), json.length()));

	    // if (!doc.is_valid())
	    // {
	    //     std::cout << "Invalid json: " << doc.get_error_message() << std::endl;;
	    // }

	    // // Get root and length of json file
	    // const auto& root = doc.get_root();
	    // const auto len = root.get_length();

	    // // // Get handle to meta deta
	    // const auto meta = root.find_object_key(literal("meta"));
	    // assert(meta < len);
	    // const auto& Meta = root.get_object_value(meta);

	    // // // Get handle to frame data
	    // const auto frames = root.find_object_key(literal("frames"));
	    // assert(frames < len);
	    // const auto& Frames = root.get_object_value(frames);

	    // // // Get image size
	    // auto ISize = Meta.get_value_of_key(literal("size"));
	    // float AWidth = ISize.get_value_of_key(literal("w")).get_safe_float_value();
	    // float AHeight = ISize.get_value_of_key(literal("h")).get_safe_float_value();

	    // atlas = {	
	    // 			EM::Vec2(AWidth, AHeight), 
    	// 			EI::ResourceManager::GetTexture(AnimTextureDir.c_str())
			  // 	};

	    // std::string j_string = EU::read_file_sstream(AnimTextureJSONDir.c_str());

		auto Json = EU::read_file_sstream(AnimTextureJSONDir.c_str());
	    
	   	// parse and serialize JSON
	   	json j_complete = json::parse(Json);

	   	// Get handle to frames data
	   	auto Frames = j_complete.at("frames");

	    // Get handle to meta deta
	    const auto Meta = j_complete.at("meta");

	    // Get image size
	    auto ISize = Meta.at("size");
	    float AWidth = ISize.at("w");
	    float AHeight = ISize.at("h");

	    atlas = {	
	    			EM::Vec2(AWidth, AHeight), 
    				EI::ResourceManager::GetTexture(AnimTextureDir.c_str())
			  	};

		return Frames;

	}
}
