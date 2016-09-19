#ifndef ENJON_UINT_SCRIPT_NODE_H
#define ENJON_UINT_SCRIPT_NODE_H

#include <Scripting/ScriptNode.h>

namespace Enjon { namespace Scripting { 

	struct EUintNode : public ConstantValueNode<EUintNode, Enjon::u32>
	{
		EUintNode()
		{
			this->Data = 1;
		}

		EUintNode(const Enjon::u32& _Value)
		{
			this->Data = _Value;
		}

		void Execute() 
		{
		}
	};

}}

#endif