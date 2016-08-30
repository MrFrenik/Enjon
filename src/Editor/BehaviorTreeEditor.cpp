/*-- Enjon includes --*/
#include "Enjon.h"
#include "Editor/BehaviorTreeEditor.h"
#include "BehaviorTree/BT.h"
#include "BehaviorTree/BehaviorTreeManager.h"
#include "IO/InputManager.h"
#include "IO/ResourceManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/FontManager.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/CursorManager.h"
#include "GUI/GUIGroup.h"
#include "Physics/AABB.h"
#include "Utils/Errors.h"
#include "Defines.h"

/*-- IsoARPG includes --*/
#include "EnjonAnimation.h"
#include "AnimationManager.h"
#include "AnimManager.h"
#include "Level.h"

/*-- Standard Library includes --*/
#include <unordered_map>
#include <vector>
#include <string>

/*-- 3rd Party Includes --*/
#include <SDL2/SDL.h>

using namespace EA;
using json = nlohmann::json;

struct BTObject
{
	std::string Name;
	Enjon::uint32 Index;
	std::vector<BTObject*> Objects;  
};

void FillJSONObjects(json& Object, struct BTObject* BTO)
{
	for (auto it = Object.begin(); it != Object.end(); ++it)
	{
		// Recurse through children
		if (it.value().is_object())
		{
			// Make new struct
			struct BTObject* O = new struct BTObject{};
			O->Name = it.key();
			 FillJSONObjects(it.value(), O);
			 BTO->Objects.push_back(O);
		}
	}


}

void PrintJSONObjects(struct BTObject& O, int depth = 0)
{
	for (auto& o : O.Objects)
	{
		for (auto i = 0; i < depth; i++)
		{
			std::cout << '\t';
		}

		std::cout << o->Name << std::endl;
		PrintJSONObjects(*o, depth + 1);

	}
}

void FillBT(json& Object, BT::BehaviorTree* BTree, BT::BehaviorNodeBase* Node)
{
	// Loop through object
	for (auto it = Object.begin(); it != Object.end(); ++it)
	{
		// Recurse through children
		if (it.value().is_object())
		{
			// Make new struct
			auto KeyName = it.key().substr(3, it.key().length() - 1);

			if (!KeyName.compare("Sequence"))
			{
				auto NewNode = new BT::Sequence(BTree);
				FillBT(it.value(), BTree, NewNode);
				Node->AddChild(NewNode);
			}
			else if (!KeyName.compare("SetPlayerLocationAsTarget"))
			{
				auto NewNode = new BT::SetPlayerLocationAsTarget(BTree);
				Node->AddChild(NewNode);
			}
			else if (!KeyName.compare("FindRandomLocation"))
			{
				auto NewNode = new BT::FindRandomLocation(BTree);
				Node->AddChild(NewNode);
			}
			else if (!KeyName.compare("RepeatUntilFail"))
			{
				auto NewNode = new BT::RepeatUntilFail(BTree);
				FillBT(it.value(), BTree, NewNode);
				Node->AddChild(NewNode);
			}
			else if (!KeyName.compare("IsTargetWithinRange"))
			{
				auto NewNode = new BT::IsTargetWithinRange(BTree);
				NewNode->Distance.Value = it.value().at("Distance");
				Node->AddChild(NewNode);
			}
			else if (!KeyName.compare("MoveToTargetLocation"))
			{
				auto NewNode = new BT::MoveToTargetLocation(BTree);
				NewNode->Speed.Value = it.value().at("Speed");
				Node->AddChild(NewNode);
			}
			else if (!KeyName.compare("StopMoving"))
			{
				auto NewNode = new BT::StopMoving(BTree);
				Node->AddChild(NewNode);
			}
			else if (!KeyName.compare("WaitWBBRead"))
			{
				auto NewNode = new BT::WaitWBBRead(BTree);
				Node->AddChild(NewNode);
			}
			else if (!KeyName.compare("Inverter"))
			{
				auto NewNode = new BT::Inverter(BTree);
				FillBT(it.value(), BTree, NewNode);
				Node->AddChild(NewNode);
			}
			else
			{
				// Couldn't find it, so error
				Enjon::Utils::FatalError("BehaviorTreeEditor::FillBT::Node not found in JSON file: " + KeyName);
			}
		}
	}
}

void PrintBTree(BT::BehaviorNodeBase* Root, int depth = 0)
{
	std::cout << Root->String() << std::endl;

	if (Root->Type == BT::BehaviorNodeType::COMPOSITE)
	{
		for (auto c : static_cast<BT::Sequence*>(Root)->Children)
		{
			if (c->Type == BT::BehaviorNodeType::COMPOSITE || 
				c->Type == BT::BehaviorNodeType::DECORATOR)
				PrintBTree(c, depth + 1);
		}
	}
	else if (Root->Type == BT::BehaviorNodeType::DECORATOR)
	{
		if (static_cast<BT::Inverter*>(Root)->Child->Type == BT::BehaviorNodeType::COMPOSITE || 
			static_cast<BT::Inverter*>(Root)->Child->Type == BT::BehaviorNodeType::DECORATOR)
			PrintBTree(static_cast<BT::Inverter*>(Root)->Child, depth + 1);
	}
}

BT::BehaviorTree* CreateBehaviorTreeFromJSON(json& Object, std::string TreeName)
{
	BT::BehaviorTree* BTree = new BT::BehaviorTree();

	// Get root from JSON
	auto RootObject = Object.at(TreeName);
	auto RootKeyName = RootObject.begin().key();
	auto RootName = RootKeyName.substr(3, RootKeyName.length() - 1);	

	// TODO(John): Have a map of function pointers that will create a BT type and return it for me
	if (!RootName.compare("Sequence"))
	{
		BTree->Root = new  BT::Sequence(BTree);
	}

	// Make sure valid
	if (BTree->Root == nullptr) 
	{
		Enjon::Utils::FatalError("BehaviorTreeEditor::CreateBehaviorTreeFromJSON::Root null.");
	}

	// Now need to fill this fucker up!
	FillBT(RootObject.at(RootKeyName), BTree, BTree->Root);

	// Finalize BTree
	BTree->End();

	return BTree;
}




namespace Enjon { namespace BehaviorTreeEditor {

	using namespace GUI;
	using namespace Graphics;

	/*-- Function Delcarations --*/
	bool ProcessInput(EI::InputManager* Input);

	EI::InputManager* Input = nullptr;

	////////////////////////////
	// LIGHT EDITOR ////////////
	
	float SCREENWIDTH;
	float SCREENHEIGHT;

	bool IsRunning = true;

	// Shaders	
	EG::GLSLProgram* BasicShader 	= nullptr;
	EG::GLSLProgram* TextShader 	= nullptr;

	// SpriteBatches
	EG::SpriteBatch UIBatch;
	EG::SpriteBatch SceneBatch;
	
	// Cameras	
	EG::Camera2D Camera;
	EG::Camera2D HUDCamera;

	EM::Vec2 CameraScaleVelocity;

	/*-- Function Definitions --*/
	bool Init(EI::InputManager* _Input, float SW, float SH)
	{
		// Shader for frame buffer
		BasicShader			= EG::ShaderManager::GetShader("Basic");
		TextShader			= EG::ShaderManager::GetShader("Text");

		// UI Batch
		UIBatch.Init();
		SceneBatch.Init();

		SCREENWIDTH = SW;
		SCREENHEIGHT = SH;

		// Create Camera
		Camera.Init(SCREENWIDTH, SCREENHEIGHT);

		// Create HUDCamera
		HUDCamera.Init(SCREENWIDTH, SCREENHEIGHT);

		// Set input
		Input = _Input;

		auto Json = EU::read_file_sstream("../IsoARPG/Profiles/Behaviors/TestTree.json");
	    
	   	// parse and serialize JSON
	   	json j_complete = json::parse(Json);

	    auto BTree = CreateBehaviorTreeFromJSON(j_complete, std::string("TestTree"));

	    // Add to BT Manager
	    BTManager::AddBehaviorTree("TestTree", BTree);

		return true;	
	}

	bool Update()
	{
		// Check for quit condition
		IsRunning = ProcessInput(Input);

		// Update input
		Input->Update();

		// Update cameras
		Camera.Update();
		HUDCamera.Update();

		return IsRunning;
	}		

	bool Draw()
	{
		// Set up necessary matricies
    	auto Model 		= EM::Mat4::Identity();	
    	auto View 		= HUDCamera.GetCameraMatrix();
    	auto Projection = EM::Mat4::Identity();

    	// Draw some shit
    	{
    		SceneBatch.Begin();
    		UIBatch.Begin();
    		{

    		}
    		SceneBatch.End();
    		UIBatch.End();
    	}

		return true;
	}

	bool ProcessInput(EI::InputManager* Input)
	{
	    SDL_Event event;
	    while (SDL_PollEvent(&event)) 
	    {
	        switch (event.type) 
	        {
	            case SDL_QUIT:
	                return false;
	                break;
				case SDL_KEYUP:
					Input->ReleaseKey(event.key.keysym.sym); 
					break;
				case SDL_KEYDOWN:
					Input->PressKey(event.key.keysym.sym);
					break;
				case SDL_MOUSEBUTTONDOWN:
					Input->PressKey(event.button.button);
					break;
				case SDL_MOUSEBUTTONUP:
					Input->ReleaseKey(event.button.button);
					break;
				case SDL_MOUSEMOTION:
					Input->SetMouseCoords((float)event.motion.x, (float)event.motion.y);
					break;
				case SDL_MOUSEWHEEL:
				default:
					break;
			}
	    }

	    if (Input->IsKeyPressed(SDLK_ESCAPE))
	    {
	    	return false;
	    }

		return true;
	}
}}
