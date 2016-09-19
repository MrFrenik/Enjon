#ifndef ENJON_WORLD_SCRIPT_NODE_H
#define ENJON_WORLD_SCRIPT_NODE_H

#include <Scripting/ScriptNode.h>
#include <ECS/ComponentSystems.h>

namespace Enjon { namespace Scripting {

	struct WorldTimeNode : public ConstantValueNode<WorldTimeNode, Enjon::f32>
	{
		WorldTimeNode()
		{
			this->Data = 0.0f;
		}

		void Execute()
		{
			// Get world time from world
			this->Data = ECSS::EntitySystem::WorldTime();
		}
	};

}}

#endif