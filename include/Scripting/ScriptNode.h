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
		Enjon::bool32 HasExecuted = false;
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
			static_cast<T*>(this)->HasExecuted = true;

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

		void Branch(ScriptNodeBase* A, ScriptNodeBase* B, Enjon::bool32 Data)
		{
			if (Data == true)
			{
				if (A != nullptr) A->Execute();
			} 	
			else
			{
				if (B != nullptr) B->Execute();
			} 				
		}

		void SetInputs(ScriptNodeBase* A, ScriptNodeBase* B)
		{
			this->InputA = A;
			this->InputB = B;
		}

		void SetOutputs(ScriptNodeBase* A, ScriptNodeBase* B)
		{
			this->OutputA = A;
			this->OutputB = B;
		}

		ScriptNodeBase* InputA;
		ScriptNodeBase* InputB;
		ScriptNodeBase* OutputA;
		ScriptNodeBase* OutputB;
		K A_Value;
		K B_Value;
	};

	template <typename T, typename K>
	struct ComponentScriptNode : public ScriptNode<ComponentScriptNode<T, K>, K>
	{
		ComponentScriptNode()
		{
			InputA = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ECS::eid32* AV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute children chain
			if (A != nullptr)
			{
				A->Execute();
				GetValue<ECS::eid32>(A, AV);
			}
		}

		void SetInputs(ScriptNodeBase* A)
		{
			this->InputA = A;
		}

		ScriptNodeBase* InputA;
		ECS::eid32 A_Value;
	};

}}

#endif