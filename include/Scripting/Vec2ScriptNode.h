#ifndef ENJON_VEC2_SCRIPT_NODE_H
#define ENJON_VEC2_SCRIPT_NODE_H

#include <Scripting/ScriptNode.h>
#include <Math/Vec2.h>

namespace Enjon { namespace Scripting {

	struct EVec2Node : public ConstantValueNode<EVec2Node, EM::Vec2>
	{
		EVec2Node()
		{
			this->Data = EM::Vec2(0.0f, 0.0f);
		}

		EVec2Node(const EM::Vec2& _Value)
		{
			this->Data = _Value;
		}

		void Execute() {}
	};

}}


#endif