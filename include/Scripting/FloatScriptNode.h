#ifndef ENJON_FLOAT_SCRIPT_NODE_H
#define ENJON_FLOAT_SCRIPT_NODE_H

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

		void Execute() {}
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

	struct MultiplyNode : public FloatArithmeticNode<MultiplyNode>
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

	struct SubtractionNode : public FloatArithmeticNode<SubtractionNode>
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

	struct AdditionNode : public FloatArithmeticNode<AdditionNode>
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

	struct DivisionNode : public FloatArithmeticNode<DivisionNode>
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


}}

#endif