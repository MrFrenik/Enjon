#ifndef TEST_TREE_H
#define TEST_TREE_H

#include "BehaviorTree/BT.h"
#include "ECS/ComponentSystems.h"
#include "Math/Random.h"

inline BT::BehaviorTree* TestTree()
{
	BT::BehaviorTree* BT = new BT::BehaviorTree();
	BT->SetRoot(new BT::Repeater(BT, new BT::Sequence(BT,
											{
												new BT::SimpleTask(BT, [](BT::BehaviorTree* BT) 
																  { 
																   		auto D = static_cast<BlackBoardComponent<ECS::eid32>*>(BT->GetBlackBoard()->GetComponent("EID"));
																   		auto B = D->GetData();
															  			std::cout << "ID: " << B << std::endl; 
																  }),
												new BT::SimpleTask(BT, [](BT::BehaviorTree* BT)
																   {
																   		auto D = static_cast<BlackBoardComponent<ECS::Systems::EntityManager*>*>(BT->GetBlackBoard()->GetComponent("EntityManager"));
																   		auto Manager = D->GetData();
																   		std::cout << Manager->MaxAvailableID << std::endl;
																   })	
											}), 
							-1
							)
	           );
	BT->End();

	return BT;
}


#endif
