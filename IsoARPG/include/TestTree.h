#ifndef TEST_TREE_H
#define TEST_TREE_H

#include "BehaviorTree/BT.h"
#include "ECS/ComponentSystems.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/EntityFactory.h"
#include "ECS/Renderer2DSystem.h"
#include "Math/Random.h"
#include <Enjon.h>

inline BT::BehaviorTree* TestTree()
{
	BT::BehaviorTree* BT = new BT::BehaviorTree();

	BT::Sequence* SEQ = new BT::Sequence(BT);
	BT::Sequence* SEQ2 = new BT::Sequence(BT);
	BT::Sequence* SEQ3 = new BT::Sequence(BT);

	// Set PLayer location
	BT::SetPlayerLocationAsTargetTask* ST1 = new BT::SetPlayerLocationAsTargetTask(BT);

	// Move to location
	BT::MoveToTargetLocationTask* ST2 = new BT::MoveToTargetLocationTask(BT);

	// Repeat until fail
	BT::RepeatUntilFail* RUF = new BT::RepeatUntilFail(BT);

	// Inverter
	BT::Inverter* INV = new BT::Inverter(BT);

	// Check if target within range
	BT::IsTargetWithinRangeConditional* CT = new BT::IsTargetWithinRangeConditional(BT);

	BT::WaitWBBRead* W = new BT::WaitWBBRead(BT);

	BT::StopMovingTask* ST3 = new BT::StopMovingTask(BT);

	// Build tree
	BT->SetRoot(SEQ);
		SEQ->AddChild(ST1);
		SEQ->AddChild(RUF);
			RUF->AddChild(SEQ3);
				SEQ3->AddChild(INV);
					INV->AddChild(CT);
				SEQ3->AddChild(ST2);
		SEQ->AddChild(SEQ2);
			SEQ2->AddChild(ST3);
			SEQ2->AddChild(W);
	BT->End();

	return BT;
}


#endif











