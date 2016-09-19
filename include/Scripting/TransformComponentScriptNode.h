#ifndef ENJON_TRANSFORM_COMPONENT_SCRIPT_NODE_H
#define ENJON_TRANSFORM_COMPONENT_SCRIPT_NODE_H

#include <Scripting/ScriptNode.h>
#include <ECS/ComponentSystems.h>
#include <ECS/Transform3DSystem.h>

namespace Enjon { namespace Scripting {

	struct TransformComponentGetComponentNode : public ComponentScriptNode<TransformComponentGetComponentNode, ECS::Component::Transform3D*>
	{
		TransformComponentGetComponentNode()
		{
			this->A_Value = 0;
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = &ECSS::EntitySystem::World()->TransformSystem->Transforms[A_Value];
		}
	};

	template <typename T, typename K>
	struct TransformComponentPropertyAccessorNode : public ScriptNode<TransformComponentPropertyAccessorNode<T, K>, K>
	{
		TransformComponentPropertyAccessorNode()
		{
			InputA = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ECS::Component::Transform3D* AV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute children chain
			if (A != nullptr)
			{
				A->Execute();
				auto S = static_cast<AccessNode<ECS::Component::Transform3D*>*>(A);
				if (S->Data != nullptr) A_Value = static_cast<ECS::Component::Transform3D*>(S->Data);
			}
		}

		void SetInputs(ScriptNodeBase* A)
		{
			this->InputA = A;
		}

		ScriptNodeBase* InputA;
		ECS::Component::Transform3D* A_Value;
	};

	struct TransformComponentGetPositionNode : public TransformComponentPropertyAccessorNode<TransformComponentGetPositionNode, EM::Vec3>
	{
		TransformComponentGetPositionNode()
		{
			this->A_Value = nullptr;
		}

		void Execute()
		{
			FillData(InputA, A_Value);
			this->Data = A_Value->Position;
		}
	};

	template <typename T, typename K>
	struct TransformComponentPropertyMutatorNode : public ScriptNode<TransformComponentPropertyMutatorNode<T, K>, K>
	{
		TransformComponentPropertyMutatorNode()
		{
			InputA = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ScriptNodeBase* B, ECS::Component::Transform3D* AV, K* BV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute children chain
			if (A != nullptr)
			{
				A->Execute();
				auto S = static_cast<AccessNode<ECS::Component::Transform3D*>*>(A);
				if (S->Data != nullptr) A_Value = static_cast<ECS::Component::Transform3D*>(S->Data);
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
		ECS::Component::Transform3D* A_Value;
		K B_Value;
	};

	struct TransformComponentSetPositionNode : public TransformComponentPropertyMutatorNode<TransformComponentSetPositionNode, EM::Vec3>
	{
		TransformComponentSetPositionNode()
		{
			this->A_Value = nullptr;
			this->B_Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, A_Value, &B_Value);
			if (A_Value != nullptr) 
			{
				A_Value->Position = B_Value;
				this->Data = B_Value;
			}
		}
	};
}}

#endif