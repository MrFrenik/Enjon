#ifndef ENJON_SCRIPT_NODE_H
#define ENJON_SCRIPT_NODE_H

#include <System/Types.h>
#include <Defines.h>

namespace Enjon { namespace Scripting { 

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

	template <typename T>
	void GetValue(ScriptNodeBase* S, T* Value);

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
	struct AccessNode : public ScriptNode<AccessNode, Enjon::f32>
	{
		void Execute(){}
	};

	template <typename T, typename K>
	struct ConstantValueNode : public ScriptNode<ConstantValueNode<T, K>, K>
	{
		void Execute()
		{
			static_cast<T*>(this)->Execute();	
		} 
	};

	struct EFloatNode : public ConstantValueNode<EFloatNode, Enjon::f32>
	{
		EFloatNode()
		{
			this->Data = 1.0f;
		}

		EFloatNode(const Enjon::f32& _Value)
		{
			this->Data = _Value;
		}

		void Execute() {}
	};

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

	struct EVec3Node : public ConstantValueNode<EVec3Node, EM::Vec3>
	{
		EVec3Node()
		{
			this->Data = EM::Vec3(0.0f, 0.0f, 0.0f);
		}

		EVec3Node(const EM::Vec3& _Value)
		{
			this->Data = _Value;
		}

		void Execute() {}
	};

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

	template <typename T>
	struct ArithmeticNode : public ScriptNode<ArithmeticNode<T>, Enjon::f32>
	{
		ArithmeticNode()
		{
			InputA = nullptr;
			InputB = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ScriptNodeBase* B, Enjon::f32* AV, Enjon::f32* BV)
		{
			// Execute children chain
			if (A != nullptr)
			{
				A->Execute();
				GetValue<Enjon::f32>(A, AV);
			}
			if (B != nullptr) 
			{
				B->Execute();
				GetValue<Enjon::f32>(B, BV);
			}
		}

		void SetInputs(ScriptNodeBase* A, ScriptNodeBase* B)
		{
			this->InputA = A;
			this->InputB = B;
		}

		ScriptNodeBase* InputA;
		ScriptNodeBase* InputB;
		Enjon::f32 A_Value;
		Enjon::f32 B_Value;
	};

	struct MultiplyNode : public ArithmeticNode<MultiplyNode>
	{
		MultiplyNode()
		{
			this->A_Value = 1.0f;
			this->B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value * B_Value;
		}
	};

	struct SubtractionNode : public ArithmeticNode<SubtractionNode>
	{
		SubtractionNode()
		{
			this->A_Value = 1.0f;
			this->B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value - B_Value;
		}
	};

	struct AdditionNode : public ArithmeticNode<AdditionNode>
	{
		AdditionNode()
		{
			this->A_Value = 1.0f;
			this->B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value + B_Value;
		}
	};

	struct DivisionNode : public ArithmeticNode<DivisionNode>
	{
		DivisionNode()
		{
			this->A_Value = 1.0f;
			this->B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  B_Value == 0 ? A_Value / 0.00001f : A_Value / B_Value;
		}
	};

	template <typename T>
	void GetValue(ScriptNodeBase* S, T* Value)
	{
		*Value = static_cast<T>(static_cast<AccessNode*>(S)->Data);
	}


}}

#endif