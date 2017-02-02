#include "BehaviorTree/BehaviorTreeManager.h"
#include "Enjon.h"
// #include "TestTree.h"

using namespace BT;
using json = nlohmann::json;

namespace BTManager
{
	std::unordered_map<std::string, BehaviorTree*> Trees;

	void FillBT(json& Object, BT::BehaviorTree* BTree, BT::BehaviorNodeBase* Node);
	BT::BehaviorTree* CreateBehaviorTreeFromJSON(json& Object, std::string TreeName);

	void Init()
	{
		// Parse json file and load in all trees
		auto Json = EU::read_file_sstream("../IsoARPG/Profiles/Behaviors/TestTree.json");

	   	// parse and serialize JSON
	   	json j_complete = json::parse(Json);

		// Load in all trees and see if they get added
		for (auto it = j_complete.begin(); it != j_complete.end(); ++it)
		{
			auto TreeName = it.key();
		    auto BTree = CreateBehaviorTreeFromJSON(j_complete, TreeName);

		    // Add to BT Manager
		    BTManager::AddBehaviorTree(TreeName, BTree);
		}
	}

	/* Add Trees to map */
	void AddBehaviorTree(std::unordered_map<std::string, BehaviorTree*>& M, std::string N, BehaviorTree* T)
	{
		// Insert into map
		// NOTE(John): Will overwrite the pre-existing kv pair if already exists!
		// Will fix this / print out warning at least
		M[N] = T;
	}

	void AddBehaviorTree(std::string N, BehaviorTree* T)
	{
		Trees[N] = T;
	}

	/* Get specificed animation with given name */
	BehaviorTree* GetBehaviorTree(std::string N)
	{
		auto it = Trees.find(N);
		if (it != Trees.end()) return it->second;
		else
		{
			printf("Behavior Tree Not found!");
			return nullptr;
		}
	}

	void DebugPrintTrees()
	{
		std::cout << "Trees: " << std::endl;

		for (auto it = Trees.begin(); it != Trees.end(); ++it)
		{
			std::cout << "\t" << it->first << std::endl;
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
				else if (!KeyName.compare("RepeatForever"))
				{
					auto NewNode = new::BT::RepeatForever(BTree);
					FillBT(it.value(), BTree, NewNode);
					Node->AddChild(NewNode);
				}
				else if (!KeyName.compare("SetViewVectorToTarget"))
				{
					auto NewNode = new::BT::SetViewVectorToTarget(BTree);
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
			BTree->Root = new BT::Sequence(BTree);
		}
		else if (!RootName.compare("RepeatForever"))
		{
			BTree->Root = new BT::RepeatForever(BTree);
		}
		else
		{
			// Couldn't find root, so error
			Enjon::Utils::FatalError("BehaviorTreeEditor::CreateBehaviorTreeFromJSON::Root null.");
		}

		// Now need to fill this fucker up!
		FillBT(RootObject.at(RootKeyName), BTree, BTree->Root);

		// Finalize BTree
		BTree->End();

		return BTree;
	}


}