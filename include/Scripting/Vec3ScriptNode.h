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
		}

		EVec3Node(const Enjon::f32& X, const Enjon::f32& Y, const Enjon::f32& Z)
		{
			this->Data = EM::Vec3(X, Y, Z);
		}

		EVec3Node(const EM::Vec3& _Value)
		{
			this->Data = _Value;
		}

		void Execute() {}
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

	struct Vec3SubtractionNode : public Vec3ArithmeticNode<Vec3SubtractionNode>
	{
		Vec3SubtractionNode()
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

	struct Vec3AdditionNode : public Vec3ArithmeticNode<Vec3AdditionNode>
	{
		Vec3AdditionNode()
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

	struct Vec3MultiplicationNode : public Vec3ArithmeticNode<Vec3MultiplicationNode>
	{
		Vec3MultiplicationNode()
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

	struct Vec3DivisionNode : public Vec3ArithmeticNode<Vec3DivisionNode>
	{
		Vec3DivisionNode()
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