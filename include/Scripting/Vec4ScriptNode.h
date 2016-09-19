#ifndef ENJON_VEC4_SCRIPT_NODE_H
#define ENJON_VEC4_SCRIPT_NODE_H

#include <Scripting/ScriptNode.h>
#include <Math/Vec4.h>

namespace Enjon { namespace Scripting { 

	struct EVec4Node : public ConstantValueNode<EVec4Node, EM::Vec4>
	{
		EVec4Node()
		{
			this->Data = EM::Vec4(0.0f, 0.0f, 0.0f, 0.0f);
			this->InputA = nullptr;
			this->InputB = nullptr;
			this->InputC = nullptr;
			this->InputD = nullptr;
		}

		EVec4Node(const Enjon::f32& X, const Enjon::f32& Y, const Enjon::f32& Z, const Enjon::f32& W)
		{
			this->InputA = nullptr;
			this->InputB = nullptr;
			this->InputC = nullptr;
			this->InputD = nullptr;
			this->Data = EM::Vec4(X, Y, Z, W);
		}

		EVec4Node(const EM::Vec4& _Value)
		{
			this->InputA = nullptr;
			this->InputB = nullptr;
			this->InputC = nullptr;
			this->InputD = nullptr;
			this->Data = _Value;
		}

		void SetInputs(ScriptNodeBase* A = nullptr, ScriptNodeBase* B = nullptr, ScriptNodeBase* C = nullptr, ScriptNodeBase* D = nullptr)
		{
			this->InputA = A;
			this->InputB = B;
			this->InputC = C;
			this->InputD = D;
		}

		void Execute() 
		{
			if (InputA != nullptr)
			{
				InputA->Execute();
				this->Data.x = static_cast<Enjon::f32>(static_cast<AccessNode<Enjon::f32>*>(InputA)->Data);
			}
			if (InputB != nullptr)
			{
				InputB->Execute();
				this->Data.y = static_cast<Enjon::f32>(static_cast<AccessNode<Enjon::f32>*>(InputB)->Data);
			}
			if (InputC != nullptr)
			{
				InputC->Execute();
				this->Data.z = static_cast<Enjon::f32>(static_cast<AccessNode<Enjon::f32>*>(InputC)->Data);
			}
			if (InputD != nullptr)
			{
				InputD->Execute();
				this->Data.w = static_cast<Enjon::f32>(static_cast<AccessNode<Enjon::f32>*>(InputD)->Data);
			}
		}

		ScriptNodeBase* InputA;
		ScriptNodeBase* InputB;
		ScriptNodeBase* InputC;
		ScriptNodeBase* InputD;
	};

	struct Vec4IsEqualCompareBranchNode : public CompareBranchNode<Vec4IsEqualCompareBranchNode, EM::Vec4>
	{
		Vec4IsEqualCompareBranchNode()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value == B_Value;
			Branch(OutputA, OutputB, Data);
		}
	};

	struct Vec4IsNotEqualCompareBranchNode : public CompareBranchNode<Vec4IsNotEqualCompareBranchNode, EM::Vec4>
	{
		Vec4IsNotEqualCompareBranchNode()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value != B_Value;
			Branch(OutputA, OutputB, Data);
		}
	};

	template <typename T>
	struct Vec4ArithmeticNode : public ScriptNode<Vec4ArithmeticNode<T>, EM::Vec4>
	{
		Vec4ArithmeticNode()
		{
			InputA = nullptr;
			InputB = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ScriptNodeBase* B, EM::Vec4* AV, EM::Vec4* BV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute children chain
			if (A != nullptr)
			{
				A->Execute();
				GetValue<EM::Vec4>(A, AV);
			}
			if (B != nullptr) 
			{
				B->Execute();
				GetValue<EM::Vec4>(B, BV);
			}
		}

		void SetInputs(ScriptNodeBase* A, ScriptNodeBase* B)
		{
			this->InputA = A;
			this->InputB = B;
		}

		ScriptNodeBase* InputA;
		ScriptNodeBase* InputB;
		EM::Vec4 A_Value;
		EM::Vec4 B_Value;
	};

	template <typename T>
	struct Vec4FloatOperationNode : public ScriptNode<Vec4FloatOperationNode<T>, Enjon::f32>
	{
		Vec4FloatOperationNode()
		{
			InputA = nullptr;
			InputB = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ScriptNodeBase* B, EM::Vec4* AV, EM::Vec4* BV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute children chain
			if (A != nullptr)
			{
				A->Execute();
				GetValue<EM::Vec4>(A, AV);
			}
			if (B != nullptr) 
			{
				B->Execute();
				GetValue<EM::Vec4>(B, BV);
			}
		}

		void SetInputs(ScriptNodeBase* A, ScriptNodeBase* B)
		{
			this->InputA = A;
			this->InputB = B;
		}

		ScriptNodeBase* InputA;
		ScriptNodeBase* InputB;
		EM::Vec4 A_Value;
		EM::Vec4 B_Value;
	};

	template <typename T, typename K>
	struct Vec4UnaryOperationNode : public ScriptNode<Vec4UnaryOperationNode<T, K>, K>
	{
		Vec4UnaryOperationNode()
		{
			InputA = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, EM::Vec4* AV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute child
			if (A != nullptr)
			{
				A->Execute();
				GetValue<EM::Vec4>(A, AV);
			}
		}

		void SetInputs(ScriptNodeBase* A)
		{
			this->InputA = A;
		}

		ScriptNodeBase* InputA;
		EM::Vec4 A_Value;
	};

	template <typename T, typename K>
	struct Vec4FloatBinaryOperationNode : public ScriptNode<Vec4FloatBinaryOperationNode<T, K>, EM::Vec4>
	{
		Vec4FloatBinaryOperationNode()
		{
			InputA = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ScriptNodeBase* B, EM::Vec4* AV, Enjon::f32* BV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute child
			if (A != nullptr)
			{
				A->Execute();
				GetValue<EM::Vec4>(A, AV);
			}

			// Execute child
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
		EM::Vec4 A_Value;
		Enjon::f32 B_Value;
	};

	struct Vec4MultiplicationFloatNode : public Vec4FloatBinaryOperationNode<Vec4MultiplicationFloatNode, EM::Vec4>
	{
		Vec4MultiplicationFloatNode()
		{
			A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
			B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data = A_Value * B_Value;
		}
	};

	struct Vec4LengthNode : Vec4UnaryOperationNode<Vec4LengthNode, Enjon::f32>
	{
		Vec4LengthNode()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data =  A_Value.Length();
		}
	};

	struct Vec4NormalizeNode : Vec4UnaryOperationNode<Vec4NormalizeNode, EM::Vec4>
	{
		Vec4NormalizeNode()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = EM::Vec4::Normalize(A_Value);
		}
	};

	struct Vec4DotProductNode : public Vec4FloatOperationNode<Vec4DotProductNode>
	{
		Vec4DotProductNode()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value.Dot(B_Value);
		}
	};

	struct Vec4SubtractionVec4Node : public Vec4ArithmeticNode<Vec4SubtractionVec4Node>
	{
		Vec4SubtractionVec4Node()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value - B_Value;
		}
	};

	struct Vec4AdditionVec4Node : public Vec4ArithmeticNode<Vec4AdditionVec4Node>
	{
		Vec4AdditionVec4Node()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value + B_Value;
		}
	};

	struct Vec4MultiplicationVec4Node : public Vec4ArithmeticNode<Vec4MultiplicationVec4Node>
	{
		Vec4MultiplicationVec4Node()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value * B_Value;
		}
	};

	struct Vec4DivisionVec4Node : public Vec4ArithmeticNode<Vec4DivisionVec4Node>
	{
		Vec4DivisionVec4Node()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);

			// Check for division by 0
			if (B_Value.x == 0 || B_Value.y == 0 || B_Value.z == 0 || B_Value.w == 0) 	this->Data = A_Value;
			else 																		this->Data =  A_Value / B_Value;
		}
	};

	struct Vec4GetYComponentNode : public Vec4UnaryOperationNode<Vec4GetYComponentNode, Enjon::f32>
	{
		Vec4GetYComponentNode()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}	

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = A_Value.y;
		}
	};

	struct Vec4GetXComponentNode : public Vec4UnaryOperationNode<Vec4GetXComponentNode, Enjon::f32>
	{
		Vec4GetXComponentNode()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}	

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = A_Value.x;
		}
	};

	struct Vec4GetZComponentNode : public Vec4UnaryOperationNode<Vec4GetZComponentNode, Enjon::f32>
	{
		Vec4GetZComponentNode()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}	

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = A_Value.z;
		}
	};

	struct Vec4GetWComponentNode : public Vec4UnaryOperationNode<Vec4GetWComponentNode, Enjon::f32>
	{
		Vec4GetWComponentNode()
		{
			this->A_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}	

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = A_Value.w;
		}
	};
}}

#endif