#ifndef ENJON_SCRIPT_NODE_H
#define ENJON_SCRIPT_NODE_H

#include <System/Types.h>
#include <Defines.h>

namespace Enjon { namespace Scripting { 

	// Base script node
	struct ScriptNodeBase
	{
		ScriptNodeBase() 
		{ 
			HasExecuted = 0; 
		}

		virtual void Execute() = 0;

		ScriptNodeBase* EntryPoint;
		ScriptNodeBase* ExitPoint;
		Enjon::uint32 HasExecuted;
	};

	// CRTP script base
	template <typename T, typename K>
	struct ScriptNode : public ScriptNodeBase
	{
		K Data;

		void Execute()
		{
			static_cast<T*>(this)->Execute();	
		}

	};

	// Base accessor node for accessing data
	template <typename T>
	struct AccessNode : public ScriptNode<AccessNode<T>, T>
	{
		void Execute(){}
	};

	// Template for getting data of node
	template <typename T>
	void GetValue(ScriptNodeBase* S, T* Value)
	{
		*Value = static_cast<T>(static_cast<AccessNode<T>*>(S)->Data);
	}

	// Templated constant value node
	template <typename T, typename K>
	struct ConstantValueNode : public ScriptNode<ConstantValueNode<T, K>, K>
	{
		void Execute()
		{
			static_cast<T*>(this)->Execute();	
		} 
	};

	// Templated compare branch node
	template <typename T, typename K>
	struct CompareBranchNode : public ScriptNode<CompareBranchNode<T, K>, Enjon::bool32>
	{
		CompareBranchNode()
		{
			InputA = nullptr;
			InputB = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ScriptNodeBase* B, K* AV, K* BV)
		{
			// Execute children chain
			if (A != nullptr)
			{
				A->Execute();
				GetValue<K>(A, AV);
			}
			if (B != nullptr) 
			{
				B->Execute();
				GetValue<K>(B, BV);
			}
		}

		void SetInputs(ScriptNodeBase* A, ScriptNodeBase* B)
		{
			this->InputA = A;
			this->InputB = B;
		}

		ScriptNodeBase* InputA;
		ScriptNodeBase* InputB;
		K A_Value;
		K B_Value;
	};
}}

#endif