#ifndef TEST_TREE_H
#define TEST_TREE_H

#include "BehaviorTree/BT.h"
#include "ECS/ComponentSystems.h"
#include "ECS/Transform3DSystem.h"
#include "Math/Random.h"

i32 RepeaterFunc(BT::BehaviorTree* T)
{
	auto D = static_cast<BlackBoardComponent<u32>*>(T->GetBlackBoard()->GetComponent("MovementLoop"));
	auto C = D->GetData();
	D->SetData(0);
	return C;
}

inline BT::BehaviorTree* TestTree()
{
	BT::BehaviorTree* BT = new BT::BehaviorTree();

	BT::Repeater* REP = new BT::Repeater(BT);
	BT::Sequence* SEQ = new BT::Sequence(BT);

	BT::SimpleTask* ST1 = new BT::SimpleTask(BT, [](BT::BehaviorTree* BT)
										   {
										   		auto D = static_cast<BlackBoardComponent<ECS::Systems::EntityManager*>*>(BT->GetBlackBoard()->GetComponent("EntityManager"));
										   		auto P = static_cast<BlackBoardComponent<EM::Vec3>*>(BT->GetBlackBoard()->GetComponent("TargetPosition"));
										   		auto Manager = D->GetData();

										   		auto W = Manager->Lvl->GetWidth();
										   		auto H = Manager->Lvl->GetHeight();

										   		// std::cout << W << ", " << H << "." << std::endl;

										   		auto X = ER::Roll(0, 800);
										   		auto Y = ER::Roll(-2000, -200);

										   		P->SetData(EM::Vec3(X, Y, 0.0f));

										   		// std::cout << "Setting new direction..." << std::endl;

										   		// Just set its velocity goal for now...
										   });

	BT::SimpleTask* ST2 = new BT::SimpleTask(BT, [](BT::BehaviorTree* BT)
										 {
									   		auto D = static_cast<BlackBoardComponent<ECS::Systems::EntityManager*>*>(BT->GetBlackBoard()->GetComponent("EntityManager"));
									   		auto ID = static_cast<BlackBoardComponent<ECS::eid32>*>(BT->GetBlackBoard()->GetComponent("EID"));
									   		auto P = static_cast<BlackBoardComponent<EM::Vec3>*>(BT->GetBlackBoard()->GetComponent("TargetPosition"));
									   		auto Manager = D->GetData();
									   		auto Target = P->GetData();
									   		auto ai = ID->GetData();

									   		auto TargetCartesian = EM::IsoToCartesian(Target.XY());
											
											auto AI = &Manager->TransformSystem->Transforms[ai];

											// Find difference in positions	
											Enjon::Math::Vec3 Difference = EM::Vec3::Normalize(EM::Vec3(TargetCartesian, Target.z) - 
																								EM::Vec3(AI->CartesianPosition, AI->Position.z));

											float speed = 2.0f;

											AI->Velocity = Difference * speed;
											AI->VelocityGoal = Difference * speed;
										 });

	BT::RepeaterWithBBRead* RWBBR = new BT::RepeaterWithBBRead(BT, 	&RepeaterFunc);														 

	BT::BBWrite* BBW = new BT::BBWrite(BT, [](BT::BehaviorTree* BT)
							{
						   		auto D = static_cast<BlackBoardComponent<u32>*>(BT->GetBlackBoard()->GetComponent("MovementLoop"));
						   		D->SetData(1);
							});

	BT::Inverter* INV = new BT::Inverter(BT);
	BT::ConditionalTask* CT = new BT::ConditionalTask(BT, [](BT::BehaviorTree* BT)
												   {
												   		auto D = static_cast<BlackBoardComponent<ECS::Systems::EntityManager*>*>(BT->GetBlackBoard()->GetComponent("EntityManager"));
												   		auto ID = static_cast<BlackBoardComponent<ECS::eid32>*>(BT->GetBlackBoard()->GetComponent("EID"));
												   		auto P = static_cast<BlackBoardComponent<EM::Vec3>*>(BT->GetBlackBoard()->GetComponent("TargetPosition"));
												   		auto Manager = D->GetData();
												   		auto Target = P->GetData();
												   		auto ai = ID->GetData();

												   		auto AI = &Manager->TransformSystem->Transforms[ai].Position.XY();

												   		auto Distance = AI->DistanceTo(Target.XY());


												   		// std::cout << Distance << std::endl;

												   		if (Distance <= 100.0f) 
												   		{
												   			return true;
												   		}
												   		
												   		else
												   		{
													   		auto TargetCartesian = EM::IsoToCartesian(Target.XY());
															
															auto AI2 = &Manager->TransformSystem->Transforms[ai];

															// Find difference in positions	
															Enjon::Math::Vec3 Difference = EM::Vec3::Normalize(EM::Vec3(TargetCartesian, Target.z) - 
																												EM::Vec3(AI2->CartesianPosition, AI2->Position.z));
															float speed = 2.0f;

															AI2->Velocity = Difference * speed;
															AI2->VelocityGoal = Difference * speed;

													   		return false;
												   		}	
												   });

	// Build tree
	BT->SetRoot(REP);
		REP->AddChild(SEQ);
			SEQ->AddChild(ST1);
			SEQ->AddChild(ST2);
			SEQ->AddChild(RWBBR);
				RWBBR->AddChild(BBW);
					BBW->AddChild(INV);
						INV->AddChild(CT);
	BT->End();

	return BT;
}


#endif











