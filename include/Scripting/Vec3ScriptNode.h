#ifndef ENJON_VEC3_SCRIPT_NODE_H
#define ENJON_VEC3_SCRIPT_NODE_H

#include <Scripting/ScriptNode.h>
#include <Math/Vec3.h>

namespace Enjon { namespace Scripting { 

	struct EVec3Node : public ConstantValueNode<EVec3Node, EM::Vec3>
	{
		EVec3Node()
		{
			this->Data = EM::Vec3(0.0f, 0.0f, 0.0f);
			this->InputA = nullptr;
			this->InputB = nullptr;
			this->InputC = nullptr;
		}

		EVec3Node(const Enjon::f32& X, const Enjon::f32& Y, const Enjon::f32& Z)
		{
			this->InputA = nullptr;
			this->InputB = nullptr;
			this->InputC = nullptr;
			this->Data = EM::Vec3(X, Y, Z);
		}

		EVec3Node(const EM::Vec3& _Value)
		{
			this->InputA = nullptr;
			this->InputB = nullptr;
			this->InputC = nullptr;
			this->Data = _Value;
		}

		void SetInputs(ScriptNodeBase* A = nullptr, ScriptNodeBase* B = nullptr, ScriptNodeBase* C = nullptr)
		{
			this->InputA = A;
			this->InputB = B;
			this->InputC = C;
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
		}

		ScriptNodeBase* InputA;
		ScriptNodeBase* InputB;
		ScriptNodeBase* InputC;
	};

	struct Vec3IsEqualCompareBranchNode : public CompareBranchNode<Vec3IsEqualCompareBranchNode, EM::Vec3>
	{
		Vec3IsEqualCompareBranchNode()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value == B_Value;
			Branch(OutputA, OutputB, Data);
		}
	};

	struct Vec3IsNotEqualCompareBranchNode : public CompareBranchNode<Vec3IsNotEqualCompareBranchNode, EM::Vec3>
	{
		Vec3IsNotEqualCompareBranchNode()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value != B_Value;
			Branch(OutputA, OutputB, Data);
		}
	};

	template <typename T>
	struct Vec3ArithmeticNode : public ScriptNode<Vec3ArithmeticNode<T>, EM::Vec3>
	{
		Vec3ArithmeticNode()
		{
			InputA = nullptr;
			InputB = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ScriptNodeBase* B, EM::Vec3* AV, EM::Vec3* BV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute children chain
			if (A != nullptr)
			{
				A->Execute();
				GetValue<EM::Vec3>(A, AV);
			}
			if (B != nullptr) 
			{
				B->Execute();
				GetValue<EM::Vec3>(B, BV);
			}
		}

		void SetInputs(ScriptNodeBase* A, ScriptNodeBase* B)
		{
			this->InputA = A;
			this->InputB = B;
		}

		ScriptNodeBase* InputA;
		ScriptNodeBase* InputB;
		EM::Vec3 A_Value;
		EM::Vec3 B_Value;
	};

	template <typename T>
	struct Vec3FloatOperationNode : public ScriptNode<Vec3FloatOperationNode<T>, Enjon::f32>
	{
		Vec3FloatOperationNode()
		{
			InputA = nullptr;
			InputB = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ScriptNodeBase* B, EM::Vec3* AV, EM::Vec3* BV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute children chain
			if (A != nullptr)
			{
				A->Execute();
				GetValue<EM::Vec3>(A, AV);
			}
			if (B != nullptr) 
			{
				B->Execute();
				GetValue<EM::Vec3>(B, BV);
			}
		}

		void SetInputs(ScriptNodeBase* A, ScriptNodeBase* B)
		{
			this->InputA = A;
			this->InputB = B;
		}

		ScriptNodeBase* InputA;
		ScriptNodeBase* InputB;
		EM::Vec3 A_Value;
		EM::Vec3 B_Value;
	};

	template <typename T, typename K>
	struct Vec3UnaryOperationNode : public ScriptNode<Vec3UnaryOperationNode<T, K>, K>
	{
		Vec3UnaryOperationNode()
		{
			InputA = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, EM::Vec3* AV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute child
			if (A != nullptr)
			{
				A->Execute();
				GetValue<EM::Vec3>(A, AV);
			}
		}

		void SetInputs(ScriptNodeBase* A)
		{
			this->InputA = A;
		}

		ScriptNodeBase* InputA;
		EM::Vec3 A_Value;
	};

	template <typename T, typename K>
	struct Vec3FloatBinaryOperationNode : public ScriptNode<Vec3FloatBinaryOperationNode<T, K>, EM::Vec3>
	{
		Vec3FloatBinaryOperationNode()
		{
			InputA = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ScriptNodeBase* B, EM::Vec3* AV, Enjon::f32* BV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute child
			if (A != nullptr)
			{
				A->Execute();
				GetValue<EM::Vec3>(A, AV);
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
		EM::Vec3 A_Value;
		Enjon::f32 B_Value;
	};

	struct Vec3MultiplicationFloatNode : public Vec3FloatBinaryOperationNode<Vec3MultiplicationFloatNode, EM::Vec3>
	{
		Vec3MultiplicationFloatNode()
		{
			A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
			B_Value = 1.0f;
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data = A_Value * B_Value;
		}
	};

	struct Vec3LengthNode : Vec3UnaryOperationNode<Vec3LengthNode, Enjon::f32>
	{
		Vec3LengthNode()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data =  A_Value.Length();
		}
	};

	struct Vec3NormalizeNode : Vec3UnaryOperationNode<Vec3NormalizeNode, EM::Vec3>
	{
		Vec3NormalizeNode()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = EM::Vec3::Normalize(A_Value);
		}
	};

	struct Vec3DotProductNode : public Vec3FloatOperationNode<Vec3DotProductNode>
	{
		Vec3DotProductNode()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value.Dot(B_Value);
		}
	};

	struct Vec3SubtractionVec3Node : public Vec3ArithmeticNode<Vec3SubtractionVec3Node>
	{
		Vec3SubtractionVec3Node()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value - B_Value;
		}
	};

	struct Vec3AdditionVec3Node : public Vec3ArithmeticNode<Vec3AdditionVec3Node>
	{
		Vec3AdditionVec3Node()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value + B_Value;
		}
	};

	struct Vec3MultiplicationVec3Node : public Vec3ArithmeticNode<Vec3MultiplicationVec3Node>
	{
		Vec3MultiplicationVec3Node()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value * B_Value;
		}
	};

	struct Vec3DivisionVec3Node : public Vec3ArithmeticNode<Vec3DivisionVec3Node>
	{
		Vec3DivisionVec3Node()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);

			// Check for division by 0
			if (B_Value.x == 0 || B_Value.y == 0 || B_Value.z == 0) this->Data = A_Value;
			else 													this->Data =  A_Value / B_Value;
		}
	};

	struct Vec3CrossProductNode : public Vec3ArithmeticNode<Vec3CrossProductNode>
	{
		Vec3CrossProductNode()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
			this->B_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, &A_Value, &B_Value);
			this->Data =  A_Value.Cross(B_Value);
		}
	};

	struct Vec3GetYComponentNode : public Vec3UnaryOperationNode<Vec3GetYComponentNode, Enjon::f32>
	{
		Vec3GetYComponentNode()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}	

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = A_Value.y;
		}
	};

	struct Vec3GetXComponentNode : public Vec3UnaryOperationNode<Vec3GetXComponentNode, Enjon::f32>
	{
		Vec3GetXComponentNode()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}	

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = A_Value.x;
		}
	};

	struct Vec3GetZComponentNode : public Vec3UnaryOperationNode<Vec3GetZComponentNode, Enjon::f32>
	{
		Vec3GetZComponentNode()
		{
			this->A_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}	

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = A_Value.z;
		}
	};
}}

#endif