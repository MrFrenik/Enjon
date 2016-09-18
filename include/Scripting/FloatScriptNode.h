#ifndef ENJON_FLOAT_SCRIPT_NODE_H
#define ENJON_FLOAT_SCRIPT_NODE_H

#include <Scripting/ScriptNode.h>

namespace Enjon { namespace Scripting { 

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

		void Execute() 
		{
		}
	};

	////////////////////////////////////////////////////
	// Float Arithmetic ////////////////////////////////
	////////////////////////////////////////////////////

	template <typename T>
	struct FloatArithmeticNode : public ScriptNode<FloatArithmeticNode<T>, Enjon::f32>
	{
		FloatArithmeticNode()
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
			static_cast<T*>(this)->HasExecuted = true;

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

	struct FloatMultiplicationNode : public FloatArithmeticNode<FloatMultiplicationNode>
	{
		FloatMultiplicationNode()
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

	struct FloatSubtractionNode : public FloatArithmeticNode<FloatSubtractionNode>
	{
		FloatSubtractionNode()
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

	struct FloatAdditionNode : public FloatArithmeticNode<FloatAdditionNode>
	{
		FloatAdditionNode()
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

	struct FloatDivision : public FloatArithmeticNode<FloatDivision>
	{
		FloatDivision()
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

	/////////////////////////////////////////////////
	// Float Compare ////////////////////////////////
	/////////////////////////////////////////////////

	struct FloatIsEqualCompareBranchNode : public CompareBranchNode<FloatIsEqualCompareBranchNode, Enjon::f32>
	{
		FloatIsEqualCompareBranchNode()
		{
			this->A_Value = 1.0f;
			this->B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value == B_Value;
			Branch(OutputA, OutputB, Data);
		}
	};

	struct FloatIsNotEqualCompareBranchNode : public CompareBranchNode<FloatIsNotEqualCompareBranchNode, Enjon::f32>
	{
		FloatIsNotEqualCompareBranchNode()
		{
			this->A_Value = 1.0f;
			this->B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value != B_Value;
			Branch(OutputA, OutputB, Data);
		}
	};

	struct FloatIsGreaterThanCompareBranchNode : public CompareBranchNode<FloatIsGreaterThanCompareBranchNode, Enjon::f32>
	{
		FloatIsGreaterThanCompareBranchNode()
		{
			this->A_Value = 1.0f;
			this->B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value > B_Value;
			Branch(OutputA, OutputB, Data);
		}
	};

	struct FloatIsLessThanCompareBranchNode : public CompareBranchNode<FloatIsLessThanCompareBranchNode, Enjon::f32>
	{
		FloatIsLessThanCompareBranchNode()
		{
			this->A_Value = 1.0f;
			this->B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value < B_Value;
			Branch(OutputA, OutputB, Data);
		}
	};

	struct FloatIsLessOrEqualCompareBranchNode : public CompareBranchNode<FloatIsLessOrEqualCompareBranchNode, Enjon::f32>
	{
		FloatIsLessOrEqualCompareBranchNode()
		{
			this->A_Value = 1.0f;
			this->B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value <= B_Value;
			Branch(OutputA, OutputB, Data);
		}
	};

	struct FloatIsGreaterOrEqualCompareBranchNode : public CompareBranchNode<FloatIsGreaterOrEqualCompareBranchNode, Enjon::f32>
	{
		FloatIsGreaterOrEqualCompareBranchNode()
		{
			this->A_Value = 1.0f;
			this->B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value >= B_Value;
			Branch(OutputA, OutputB, Data);
		}
	};

}}

#endif