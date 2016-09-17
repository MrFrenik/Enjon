#ifndef ENJON_CAST_SCRIPT_NODE_H
#define ENJON_CAST_SCRIPT_NODE_H

#include <Scripting/ScriptNode.h>

namespace Enjon { namespace Scripting {

	template <typename T, typename K>
	struct CastNode : public ScriptNode<CastNode<T, K>, K>
	{
		CastNode()
		{
			Input = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, K* Data)
		{
			if (A != nullptr)
			{
				A->Execute();
				GetValue<K>(A, Data);
			}
		}

		ScriptNodeBase* Input;
	};

	struct CastToIntNode : public CastNode<CastToIntNode, Enjon::int32>
	{
		CastToIntNode()
		{
			this->Data = 1;
		}

		void Execute()
		{
			FillData(Input, &Data);
		}
	};

}}

#endif