#ifndef ENJON_RENDER_COMPONENT_SCRIPT_NODE_H
#define ENJON_RENDER_COMPONENT_SCRIPT_NODE_H

#include <Scripting/ScriptNode.h>
#include <ECS/ComponentSystems.h>
#include <ECS/Renderer2DSystem.h>

namespace Enjon { namespace Scripting {

	struct RenderComponentGetComponentNode : public ComponentScriptNode<RenderComponentGetComponentNode, ECS::Component::Renderer2DComponent*>
	{
		RenderComponentGetComponentNode()
		{
			this->A_Value = 0;
		}

		void Execute()
		{
			FillData(InputA, &A_Value);
			this->Data = &ECSS::EntitySystem::World()->Renderer2DSystem->Renderers[A_Value];
		}
	};

	template <typename T, typename K>
	struct RenderComponentPropertyAccessorNode : public ScriptNode<RenderComponentPropertyAccessorNode<T, K>, K>
	{
		RenderComponentPropertyAccessorNode()
		{
			InputA = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ECS::Component::Renderer2DComponent* AV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute children chain
			if (A != nullptr)
			{
				A->Execute();
				auto S = static_cast<AccessNode<ECS::Component::Renderer2DComponent*>*>(A);
				if (S->Data != nullptr) A_Value = static_cast<ECS::Component::Renderer2DComponent*>(S->Data);
			}
		}

		void SetInputs(ScriptNodeBase* A)
		{
			this->InputA = A;
		}

		ScriptNodeBase* InputA;
		ECS::Component::Renderer2DComponent* A_Value;
	};

	struct RenderComponentGetColorNode : public RenderComponentPropertyAccessorNode<RenderComponentGetColorNode, EM::Vec4>
	{
		RenderComponentGetColorNode()
		{
			this->A_Value = nullptr;
		}

		void Execute()
		{
			FillData(InputA, A_Value);
			this->Data = EM::Vec4(A_Value->Color.r, A_Value->Color.g, A_Value->Color.b, A_Value->Color.a);
		}
	};

	template <typename T, typename K>
	struct RenderComponentPropertyMutatorNode : public ScriptNode<RenderComponentPropertyMutatorNode<T, K>, K>
	{
		RenderComponentPropertyMutatorNode()
		{
			InputA = nullptr;
		}

		void Execute()
		{
			static_cast<T*>(this)->Execute();
		}

		void FillData(ScriptNodeBase* A, ScriptNodeBase* B, ECS::Component::Renderer2DComponent* AV, K* BV)
		{
			static_cast<T*>(this)->HasExecuted = true;

			// Execute children chain
			if (A != nullptr)
			{
				A->Execute();
				auto S = static_cast<AccessNode<ECS::Component::Renderer2DComponent*>*>(A);
				if (S->Data != nullptr) A_Value = static_cast<ECS::Component::Renderer2DComponent*>(S->Data);
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
		ECS::Component::Renderer2DComponent* A_Value;
		K B_Value;
	};

	struct RenderComponentSetColorNode : public RenderComponentPropertyMutatorNode<RenderComponentSetColorNode, EM::Vec4>
	{
		RenderComponentSetColorNode()
		{
			this->A_Value = nullptr;
			this->B_Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		void Execute()
		{
			FillData(InputA, InputB, A_Value, &B_Value);
			if (A_Value != nullptr) 
			{
				A_Value->Color = EG::ColorRGBA16(B_Value);
				this->Data = B_Value;
			}
		}
	};
}}

#endif