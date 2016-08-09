
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
		AddAnimation(Animations, "CB", 					  EA::CreateAnimation("CB", 				   Frames, atlas, AnimationDir));
		AddAnimation(Animations, "Enemy_Pixel", 		  EA::CreateAnimation("Enemy_Pixel",		   Frames, atlas, AnimationDir));
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

	std::unordered_map<std::string, EA::Anim*>* GetAnimationMap()
	{
		return &Animations;
	}
}
