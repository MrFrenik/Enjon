#ifndef ENJON_VEC4_SCRIPT_NODE_H
#define ENJON_VEC4_SCRIPT_NODE_H

#include <Math/Vec4.h>
#include <Scripting/ScriptNode.h>

namespace Enjon { namespace Scripting { 

	struct EVec4Node : public ConstantValueNode<EVec4Node, EM::Vec4>
	{
		EVec4Node()
		{
			this->Data = EM::Vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}

		EVec4Node(const EM::Vec4& _Value)
		{
			this->Data = _Value;
		}

		void Execute() {}
	};

}}

#endif