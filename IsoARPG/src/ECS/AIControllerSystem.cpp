#include "ECS/AIControllerSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/EntityFactory.h"

#include <Graphics/SpriteSheet.h>
#include <Math/Maths.h>

using namespace PathFinding;

const float MIN_DISTANCE 							= 300.0f;
const float MAX_DISTANCE_TARGET_FROM_FINAL_NODE 	= 650.0f;
const float AISpeed 								= 3.0f;

namespace ECS { namespace Systems { namespace AIController {


	struct AIControllerSystem* NewAIControllerSystem(struct EntityManager* Manager)
	{ 
		struct AIControllerSystem* System = new AIControllerSystem;
		System->Manager = Manager;
		return System;
	}

	// Updates Controller of AI it is attached to
	void Update(struct AIControllerSystem* System, eid32 Player)
	{
		struct EntityManager* Manager = System->Manager;
		const Component::Transform3D* Target = &Manager->TransformSystem->Transforms[Player];
		// Let's just make it go towards the player for testing
		for (eid32 ai = 0; ai < Manager->MaxAvailableID; ai++)
		{
			// Check to see if entity has ai controller
			// if ((Manager->Masks[ai] & (COMPONENT_AICONTROLLER | COMPONENT_TRANSFORM3D)) == (COMPONENT_AICONTROLLER | COMPONENT_TRANSFORM3D))
			// {
			// 	// Get AI behavior tree and blackboard
			// 	auto AC = &Manager->AIControllerSystem->AIControllers[ai];
			// 	auto Brain = AC->Brain;
			// 	auto BB = AC->BB;

			// 	// Run the tree
			// 	Brain->Run(BB);
			// }

			if ((Manager->Masks[ai] & (COMPONENT_AICONTROLLER | COMPONENT_TRANSFORM3D)) == (COMPONENT_AICONTROLLER | COMPONENT_TRANSFORM3D))
			{
				// Get AI behavior tree and blackboard
				auto AC = &Manager->AIControllerSystem->AIControllers[ai];
				auto TS = &Manager->TransformSystem;
				auto AICartesianPositon = &Manager->TransformSystem->Transforms[ai].CartesianPosition;
				auto PlayerCartesianPosition = &Manager->TransformSystem->Transforms[Manager->Player].CartesianPosition;
				auto PlayerPosition = Manager->TransformSystem->Transforms[Manager->Player].Position.XY();
				auto AIPosition = Manager->TransformSystem->Transforms[ai].Position.XY();
				auto AIVelocity = &Manager->TransformSystem->Transforms[ai].Velocity;
				auto AIVelocityGoal = &Manager->TransformSystem->Transforms[ai].VelocityGoal;
				auto PathFindingComponent = &Manager->AIControllerSystem->PathFindingComponents[ai];

				bool leave = false;

				// Find Path to player
				if (!PathFindingComponent->HasPath)
				{
					// Don't keep updating if you're already there
					float D = (PlayerPosition - AIPosition).Length();
					if (D <= MIN_DISTANCE) 
					{
						AIVelocity->x = 0.0f;
						AIVelocity->y = 0.0f;
						AIVelocityGoal->x = 0.0f;
						AIVelocityGoal->y = 0.0f;
						PathFindingComponent->HasPath = false;
						leave = true;
						std::cout << "Leaving!" << std::endl;
					}

					if (!leave)
					{
						PathFindingComponent->Path = PathFinding::FindPath(Manager->Grid, *AICartesianPositon, *PlayerCartesianPosition);
						PathFindingComponent->PathSize = PathFindingComponent->Path.size();

						if (PathFindingComponent->PathSize)
						{
							PathFindingComponent->HasPath = true;

							std::cout << "Getting path..." << std::endl; 
			
							// Get path information
							PathFindingComponent->CurrentPathIndex = 0;
							PathFindingComponent->TimeOnNode = 0.0f;
							auto Index = PathFindingComponent->Path.at(PathFindingComponent->CurrentPathIndex).Index;
							auto Coords = SpatialHash::FindGridCoordinatesFromIndex(Manager->Grid, Manager->Grid->cells.at(Index).ParentIndex);
							PathFindingComponent->CurrentCellDimensions = SpatialHash::GetCellDimensions(Manager->Grid, Coords);
							PathFindingComponent->CellPosition = EM::Vec2(PathFindingComponent->CurrentCellDimensions.x, PathFindingComponent->CurrentCellDimensions.y);
						}

						else PathFindingComponent->HasPath = false;
					}
				}

				if (PathFindingComponent->HasPath && !leave)
				{
					auto Distance = (PathFindingComponent->CellPosition - AIPosition).Length();

					if (Distance <= 40.0f)
					{
						PathFindingComponent->CurrentPathIndex ++;

						PathFindingComponent->TimeOnNode = 0.0f;

						if (PathFindingComponent->CurrentPathIndex >= PathFindingComponent->PathSize)
						{
							PathFindingComponent->HasPath = false;
							PathFindingComponent->CurrentPathIndex = 0;
							continue;
						}

						auto Index = PathFindingComponent->Path.at(PathFindingComponent->CurrentPathIndex).Index;

						// Get parent of index from spatial hash
						auto Coords = SpatialHash::FindGridCoordinatesFromIndex(Manager->Grid, Manager->Grid->cells.at(Index).ParentIndex);
						PathFindingComponent->CurrentCellDimensions = SpatialHash::GetCellDimensions(Manager->Grid, Coords);
						PathFindingComponent->CellPosition = EM::Vec2(PathFindingComponent->CurrentCellDimensions.x, PathFindingComponent->CurrentCellDimensions.y);
					}

					else
					{
						PathFindingComponent->TimeOnNode += 0.1f;

						// Stuck
						if (PathFindingComponent->TimeOnNode >= 4.0f)
						{
							std::cout << "Re-routing..." << std::endl;
							// Refind path next frame
							PathFindingComponent->HasPath = false;
						}

						// Look ahead and decide whether or not to reroute
						auto NextIndex = PathFindingComponent->Path.size() - PathFindingComponent->Path.size() / 2;
						auto AfterNextIndex = PathFindingComponent->Path.size() - (PathFindingComponent->Path.size() / 2) - 1;

						if (NextIndex < PathFindingComponent->PathSize && AfterNextIndex > 0)
						{
							// Reroute
							if (Manager->Grid->cells.at(PathFindingComponent->Path.at(NextIndex).Index).ObstructionValue >= 1.0f)
							{
								PathFindingComponent->HasPath = false;
							}
						}

						if (AfterNextIndex < PathFindingComponent->PathSize && AfterNextIndex > 0)
						{
							// Reroute
							if (Manager->Grid->cells.at(PathFindingComponent->Path.at(AfterNextIndex).Index).ObstructionValue >= 1.0f)
							{
								PathFindingComponent->HasPath = false;
							}
						}

						// Check to make sure that target has not completely left the path

						auto TargetIndex = PathFindingComponent->Path.at(PathFindingComponent->Path.size() - 1).Index;
						auto Coords = SpatialHash::FindGridCoordinatesFromIndex(Manager->Grid, Manager->Grid->cells.at(TargetIndex).ParentIndex);
						auto CellDims = SpatialHash::GetCellDimensions(Manager->Grid, Coords);
						auto CP = EM::Vec2(CellDims.x, CellDims.y);
						auto D = (PlayerPosition - CP).Length();
						if (D >= MAX_DISTANCE_TARGET_FROM_FINAL_NODE)
						{
							PathFindingComponent->HasPath = false;
						}

					}

					// Find vector
					auto Difference = EM::Vec2::Normalize(PathFindingComponent->CellPosition - AIPosition);

					AIVelocity->x = Difference.x * AISpeed;
					AIVelocity->y = Difference.y * AISpeed;

					if (AIVelocity->x < 0) 	Manager->TransformSystem->Transforms[ai].ViewVector.x = -1;
					else 					Manager->TransformSystem->Transforms[ai].ViewVector.x = 1;

				}

			}
		}
	}

	void Reset(struct EntityManager* Manager, eid32 Entity)
	{
		auto AIController = &Manager->AIControllerSystem->AIControllers[Entity];

		// Free memory / The behavior tree is managed by the BT Manager
		delete AIController->SO;
		delete AIController->BB;

		// Set to null
		AIController->Brain = nullptr;
		AIController->SO = nullptr;
		AIController->BB = nullptr;


		AIController->Entity = 0;
	}	
}}}




