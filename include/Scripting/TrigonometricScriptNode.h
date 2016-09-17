#ifndef ENJON_TRIG_SCRIPT_NODE_H
#define ENJON_TRIG_SCRIPT_NODE_H

#include <Scripting/ScriptNode.h>
#include <Math/Common.h>

namespace Enjon { namespace Scripting {

	template <typename T>
	struct TrigonometricOperationNode : public ScriptNode<TrigonometricOperationNode<T>, Enjon::f32>
	{
		TrigonometricOperationNode()
		{
			InputA = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, Enjon::f32* AV)
		{
			// Execute childr
			if (A != nullptr)
			{
				A->Execute();
				GetValue<Enjon::f32>(A, AV);
			}
		}

		void SetInputs(ScriptNodeBase* A)
		{
			this->InputA = A;
		}

		ScriptNodeBase* InputA;
		Enjon::f32 A_Value;
	};

	struct CastToRadiansNode : public TrigonometricOperationNode<CastToRadiansNode>
	{
		CastToRadiansNode()
		{
			this->A_Value = 0.0f;
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = EM::ToRadians(A_Value);
		}
	};

	struct CastToDegreesNode : public TrigonometricOperationNode<CastToDegreesNode>
	{
		CastToDegreesNode()
		{
			this->A_Value = 0.0f;
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = EM::ToDegrees(A_Value);
		}
	};

	struct CosineNode : public TrigonometricOperationNode<CosineNode>
	{
		CosineNode()
		{
			this->A_Value = 0.0f;
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = std::cos(A_Value);
		}
	};

	struct InverseCosineNode : public TrigonometricOperationNode<InverseCosineNode>
	{
		InverseCosineNode()
		{
			this->A_Value = 0.0f;
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = std::acos(A_Value);
		}
	};

	struct SinNode : public TrigonometricOperationNode<SinNode>
	{
		SinNode()
		{
			this->A_Value = 0.0f;
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = std::sin(A_Value);
		}
	};

	struct InverseSinNode : public TrigonometricOperationNode<InverseSinNode>
	{
		InverseSinNode()
		{
			this->A_Value = 0.0f;
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = std::asin(A_Value);
		}
	};

	struct TanNode : public TrigonometricOperationNode<TanNode>
	{
		TanNode()
		{
			this->A_Value = 0.0f;
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = std::tan(A_Value);
		}
	};

	struct InverseTanNode : public TrigonometricOperationNode<InverseTanNode>
	{
		InverseTanNode()
		{
			this->A_Value = 0.0f;
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = std::atan(A_Value);
		}
	};


}}

#endif