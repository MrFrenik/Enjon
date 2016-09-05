/*-- Enjon includes --*/
#include "Enjon.h"
#include "Editor/BehaviorTreeEditor.h"
#include "BehaviorTree/BT.h"
#include "BehaviorTree/BehaviorTreeManager.h"
#include "IO/InputManager.h"
#include "IO/ResourceManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/FontManager.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/CursorManager.h"
#include "Graphics/Shapes.h"
#include "GUI/GUI.h"
#include "Physics/AABB.h"
#include "Utils/Errors.h"
#include "Defines.h"

/*-- IsoARPG includes --*/
#include "EnjonAnimation.h"
#include "AnimationManager.h"
#include "AnimManager.h"
#include "Level.h"

/*-- Standard Library includes --*/
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>

/*-- 3rd Party Includes --*/
#include <SDL2/SDL.h>

using namespace EA;
using json = nlohmann::json;

namespace Enjon { namespace GUI { 
	
	struct GUIBTNode : GUIElement<GUIBTNode>
	{
		GUIBTNode()
		{
			// Set up type
			this->Type = GUIType::SCENE_ELEMENT;

			// Set up states
			this->State = ButtonState::INACTIVE;
			this->HoverState = HoveredState::OFF_HOVER;
			this->Dimensions = EM::Vec2(200.0f, 100.0f);
			this->Position = EM::Vec2(0.0f);
			this->AABB.Min = this->Position;
			this->AABB.Max = this->AABB.Min + this->Dimensions;
			this->ParentNode = nullptr;
			this->BorderColor = EG::RGBA16_MidGrey();
			this->DrawingParentingLine = false;
			this->TargetPosition = EM::Vec2(-999999.0f);
			this->Targeting = false;

			// Set up GUIBTNode's on_hover signal
			this->on_hover.connect([&]()
			{
				this->HoverState = HoveredState::ON_HOVER;
			});

			// Set up GUIBTNode's off_hover signal
			this->off_hover.connect([&]()
			{
				this->HoverState = HoveredState::OFF_HOVER;
			});

			// Set up GUIBTNode's lose_focus signal
			this->lose_focus.connect([&]()
			{
				if (!Targeting) return;

				// Leave if not in group
				if (this->Group == nullptr) return;

				else
				{
					for (auto& C : Group->Children)
					{
						// Don't process this
						if (C == this) continue;

						if (EP::AABBvsPoint(&C->AABB, TargetPosition)) 
						{
							std::cout << "Intersected!" << std::endl;	

							// Add child
							this->AddChild(static_cast<GUIBTNode*>(C));
							return;
						}
					}
				}
			});

		}

		void Init() {}

		void Update()
		{
			// Update AABB
			this->AABB.Min = this->Position;
			this->AABB.Max = this->AABB.Min + this->Dimensions;
		}

		bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
		{
			static EM::Vec2 MouseFrameOffset(0.0f);
			static bool JustFocused = true;

		    SDL_Event event;
		    while (SDL_PollEvent(&event)) 
		    {
		        switch (event.type) 
		        {
					case SDL_KEYUP:
						Input->ReleaseKey(event.key.keysym.sym); 
						break;
					case SDL_KEYDOWN:
						Input->PressKey(event.key.keysym.sym);
						break;
					case SDL_MOUSEBUTTONDOWN:
						Input->PressKey(event.button.button);
						break;
					case SDL_MOUSEBUTTONUP:
						Input->ReleaseKey(event.button.button);
						break;
					case SDL_MOUSEMOTION:
						Input->SetMouseCoords((float)event.motion.x, (float)event.motion.y);
						break;
					default:
						break;
				}
		    }

		    auto MousePos = Input->GetMouseCoords();
		    Camera->ConvertScreenToWorld(MousePos);

    		if (Input->IsKeyDown(SDLK_LALT))
    		{
			    if (Input->IsKeyPressed(SDL_BUTTON_LEFT)) 
			    {
			    	if (this->ParentNode != nullptr) 
			    	{
				    	std::cout << "Removing..." << std::endl;
			    		this->ParentNode->RemoveChild(this);
			    		this->ParentNode = nullptr;
			    	}
			    }
    		}

		    else if (Input->IsKeyDown(SDL_BUTTON_LEFT))
		    {
				auto X = MousePos.x;
				auto Y = MousePos.y;

	    		if (JustFocused) 
	    		{
	    			MouseFrameOffset = EM::Vec2(MousePos.x - this->AABB.Min.x, MousePos.y - this->AABB.Min.y);
	    			JustFocused = false;

	    			// Just clicked
	    			this->on_click.emit();
	    		}

		    	
		    	if (Input->IsKeyDown(SDLK_LSHIFT))
		    	{
		    		// Draw line from Connection point to mouse
		    		DrawingParentingLine = true;

		    		TargetPosition = MousePos;

		    		Targeting = true;
		    	}

		    	else
		    	{
	    			// Change border color to active
	    			this->BorderColor = EG::RGBA16_Yellow();

					// Update Position
					this->Position = EM::Vec2(X - MouseFrameOffset.x, Y - MouseFrameOffset.y);

					// Emit that value has changed
					this->on_value_change.emit();
		    	}
		    }

	    	else 
	    	{
	    		this->lose_focus.emit();
    			this->BorderColor = EG::RGBA16_MidGrey();
    			this->DrawingParentingLine = false;
	    		JustFocused = true;
				Targeting = false;
	    		return true;
	    	}

			return false;
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			auto BorderThickness = 10.0f;

			Batch->Add(
							EM::Vec4(Position, Dimensions),
							EM::Vec4(0, 0, 1, 1),
							EI::ResourceManager::GetTexture("../Assets/Textures/Default.png").id,
							EG::RGBA16_DarkGrey(),
							Depth,
							EG::SpriteBatch::DrawOptions::BORDER | EG::SpriteBatch::DrawOptions::SHADOW,
							BorderColor,
							BorderThickness,
							EM::Vec2(2.0f, 2.0f),
							16.0f
						);

			auto Advance = 0.0f;
			auto Height = 0.0f;

			if (this->TextFont == nullptr) this->TextFont = EG::FontManager::GetFont("WeblySleek_10");

			for (auto& c : this->Name)
			{
				// Get advance
				Advance += EG::Fonts::GetAdvance(c, this->TextFont, 1.0f);

				// Get height
				Height += EG::Fonts::GetHeight(c, this->TextFont, 1.0f);
			}

			EG::Fonts::PrintText(		
									Position.x + Dimensions.x / 2.0f - Advance / 2.0f, 
									Position.y + Dimensions.y / 2.0f - Height / 2.0f, 
									1.0f, 
									this->Name, 
									this->TextFont, 
									*Batch, 
									EG::SetOpacity(Graphics::RGBA16_White(), 0.8f)
								);



			// Draw lines to children
			EM::Vec2 ParentCenter(this->Position.x + this->Dimensions.x / 2.0f, this->Position.y);
			for (auto c : Children)
			{
				// Get bottom center of this
				EM::Vec2 ChildCenter(c->Position.x + c->Dimensions.x / 2.0f, c->Position.y + c->Dimensions.y);
				EG::Shapes::DrawLine(Batch, EM::Vec4(ParentCenter, ChildCenter), 5.0f, EG::RGBA16_LightGrey(), Depth - 2.0f);
			}

			// If parenting line being drawn
			if (DrawingParentingLine)
			{
				EG::Shapes::DrawLine(Batch, EM::Vec4(ParentCenter, TargetPosition), 5.0f, EG::RGBA16_LightGrey(), Depth - 2.0f);
			}
		}

		void RemoveChild(GUIBTNode* Child)
		{
			this->Children.erase(std::remove(this->Children.begin(), this->Children.end(), Child), this->Children.end());
		}

		void AddChild(GUIBTNode* Child)
		{
			// Leave if this is our parent
			if (this->ParentNode != nullptr && Child == this->ParentNode) return;

			if (Children.size() < MaxChildren)
			{
				// Need to make sure child isn't present already in vector
				if (std::find(Children.begin(), Children.end(), Child) == Children.end())
				{
					// Make sure child doesn't already have a parent
					if (Child->ParentNode == nullptr)
					{
						// Push back
						Children.push_back(Child);
						// Set parent as this
						Child->ParentNode = this;

						return;
					}	
					// Otherwise, parent already set
					std::cout << "GUIBTNode::AddChild::GUIBTNode Child parent already assigned." << std::endl;
				}
				// Otherwise, we already contain that child
				std::cout << "GUIBTNode::AddChild::GUIBTNode Child already in vector." << std::endl;
			}
			// Otherwise, we have too many children as it is
			std::cout << "GUIBTNode::AddChild::Max children reached" << std::endl;
			std::cout << "Children size: " << Children.size() << std::endl;
			std::cout << "MaxChildren: " << MaxChildren << std::endl;

		}

		EGUI::Signal<> on_value_change;
		BT::BehaviorNodeType BehaviorType;
		Enjon::uint32 MaxChildren;
		std::vector<GUIBTNode*> Children;
		EG::ColorRGBA16 BorderColor;
		GUIBTNode* ParentNode;
		EG::Fonts::Font* TextFont;
		Enjon::uint32 DrawingParentingLine;
		EM::Vec2 TargetPosition;
		GUISceneGroup* Group;
		Enjon::uint32 Targeting;
	};
}}

namespace Enjon { namespace BehaviorTreeEditor {

	using namespace GUI;
	using namespace Graphics;

	/*-- Function Delcarations --*/
	bool ProcessInput(EI::InputManager* Input);

	EI::InputManager* Input = nullptr;

	////////////////////////////////////
	// BEHAVIOR TREE EDITOR ////////////
	
	float SCREENWIDTH;
	float SCREENHEIGHT;

	bool IsRunning = true;

	// Shaders	
	EG::GLSLProgram* BasicShader 	= nullptr;
	EG::GLSLProgram* TextShader 	= nullptr;

	// SpriteBatches
	EG::SpriteBatch UIBatch;
	EG::SpriteBatch SceneBatch;
	
	// Cameras	
	EG::Camera2D Camera;
	EG::Camera2D HUDCamera;

	EM::Vec2 CameraScaleVelocity;

	// GUI Elements
	GUISceneGroup 		BehaviorNodeSceneGroup;
	GUIBTNode 			SequenceNode;
	GUIBTNode 			SelectorNode;
	GUIBTNode 			LeafNode;
	GUIBTNode 			DecoratorNode;


	/*-- Function Definitions --*/
	bool Init(EI::InputManager* _Input, float SW, float SH)
	{
		// Shader for frame buffer
		BasicShader			= EG::ShaderManager::GetShader("Text");
		TextShader			= EG::ShaderManager::GetShader("Text");

		// UI Batch
		UIBatch.Init();
		SceneBatch.Init();

		SCREENWIDTH = SW;
		SCREENHEIGHT = SH;

		// Create Camera
		Camera.Init(SCREENWIDTH, SCREENHEIGHT);

		// Create HUDCamera
		HUDCamera.Init(SCREENWIDTH, SCREENHEIGHT);

		// Init spritebatches
		// TODO(John): Put in a debug statement that says whether or not a spritebatch is initialized or not
		// Or, if it's not initialized, then do so when begin is called
		UIBatch.Init();
		SceneBatch.Init();

		// Set input
		Input = _Input;

		// Set up Sequence as root
		SequenceNode.MaxChildren = 10;
		SequenceNode.BehaviorType = BT::BehaviorNodeType::COMPOSITE;
		SequenceNode.Name = "Sequence";

		// Set up Sequence as root
		SelectorNode.MaxChildren = 10;
		SelectorNode.BehaviorType = BT::BehaviorNodeType::COMPOSITE;
		SelectorNode.Name = "Selector";


		// Set up Sequence as root
		DecoratorNode.MaxChildren = 1;
		DecoratorNode.BehaviorType = BT::BehaviorNodeType::DECORATOR;
		DecoratorNode.Name = "Decorator";


		// Set up Sequence as root
		LeafNode.MaxChildren = 0;
		LeafNode.BehaviorType = BT::BehaviorNodeType::LEAF;
		LeafNode.Name = "Leaf";


		// Add SceneELement to group
		BehaviorNodeSceneGroup.AddToGroup(&SequenceNode, 	"Sequence");
		BehaviorNodeSceneGroup.AddToGroup(&DecoratorNode, 	"Decorator");
		BehaviorNodeSceneGroup.AddToGroup(&LeafNode, 		"Leaf");
		BehaviorNodeSceneGroup.AddToGroup(&SelectorNode, 	"Selector");
		SequenceNode.Group = &BehaviorNodeSceneGroup;
		DecoratorNode.Group = &BehaviorNodeSceneGroup;
		LeafNode.Group = &BehaviorNodeSceneGroup;
		SelectorNode.Group = &BehaviorNodeSceneGroup;


		return true;	
	}

	bool Update()
	{
		// Check for quit condition
		IsRunning = ProcessInput(Input);

		// Update input
		Input->Update();

		// Update cameras
		Camera.Update();
		HUDCamera.Update();

		// Update scene group
		BehaviorNodeSceneGroup.Update();

		return IsRunning;
	}		

	bool Draw()
	{
		// Set up necessary matricies
    	auto Model 		= EM::Mat4::Identity();	
    	auto View 		= Camera.GetCameraMatrix();
    	auto Projection = EM::Mat4::Identity();

		TextShader->Use();
		{
			TextShader->SetUniformMat4("model", Model);
			TextShader->SetUniformMat4("projection", Projection);
			TextShader->SetUniformMat4("view", View);

	    		SceneBatch.Begin(EG::GlyphSortType::FRONT_TO_BACK);
		    	{
		    		BehaviorNodeSceneGroup.Draw(&SceneBatch);
		    	}
		    	SceneBatch.End();
		    	SceneBatch.RenderBatch();
		}
		TextShader->Unuse();

		return true;
	}

	bool ProcessInput(EI::InputManager* Input)
	{
	    SDL_Event event;
	    while (SDL_PollEvent(&event)) 
	    {
	        switch (event.type) 
	        {
	            case SDL_QUIT:
	                return false;
	                break;
				case SDL_KEYUP:
					Input->ReleaseKey(event.key.keysym.sym); 
					break;
				case SDL_KEYDOWN:
					Input->PressKey(event.key.keysym.sym);
					break;
				case SDL_MOUSEBUTTONDOWN:
					Input->PressKey(event.button.button);
					break;
				case SDL_MOUSEBUTTONUP:
					Input->ReleaseKey(event.button.button);
					break;
				case SDL_MOUSEMOTION:
					Input->SetMouseCoords((float)event.motion.x, (float)event.motion.y);
					break;
				case SDL_MOUSEWHEEL:
				default:
					break;
			}
	
			static GUIElementBase* E = nullptr;
			static GUIElementBase* MouseFocus = nullptr;
			static bool PrintedDebugNoChild = false;
			bool ProcessMouse = false;

			auto MouseCoords = Input->GetMouseCoords();
			Camera.ConvertScreenToWorld(MouseCoords);
			bool ChildFound = false;
			auto G = &BehaviorNodeSceneGroup;

			if (MouseFocus)
			{
				// Check if mouse focus is done processing itself
				if (MouseFocus->Type == GUIType::SCENE_ELEMENT)
				{
					ProcessMouse = MouseFocus->ProcessInput(Input, &Camera);
			
					// If done, then return from function
					if (ProcessMouse && !Input->IsKeyDown(SDL_BUTTON_LEFT)) 
					{
						// std::cout << "Losing focus after processing complete..." << std::endl;
						MouseFocus->lose_focus.emit();
						MouseFocus = nullptr;
						return true;
					}
					else return true;	
				}

				// else
				// {
				// 	ProcessMouse = MouseFocus->ProcessInput(Input, &HUDCamera);
		
				// 	// If done, then return from function
				// 	if (ProcessMouse) 
				// 	{
				// 		std::cout << "Losing focus after processing complete..." << std::endl;
				// 		MouseFocus->lose_focus.emit();
				// 		MouseFocus = nullptr;
				// 		return true;
				// 	}
				// }
			}

			if (G->Visibility)
			{
				for (auto C : G->Children)
				{
					auto AABB = &C->AABB;

					C->check_children.emit(Input, &Camera);

					if (EP::AABBvsPoint(AABB, MouseCoords))
					{
						// Set to true
						ChildFound = true;

						// Set on hover
						C->on_hover.emit();

						// Set the hovered element
						G->HoveredElement = C;

						// If clicked while hovering
						if (Input->IsKeyPressed(SDL_BUTTON_LEFT))
						{
							C->on_click.emit();

							if (MouseFocus == nullptr) 
							{
								std::cout << "Gaining Focus: " << C->Name << std::endl;
								MouseFocus = C;
								MouseFocus->ProcessInput(Input, &Camera);
							}

							// If not mouse focus
							else if (MouseFocus != C && C->Depth >= MouseFocus->Depth && !ProcessMouse) 
							{
								std::cout << "Switching Focus: " << MouseFocus->Name << " to " << C->Name << std::endl;

								// Lose focus
								MouseFocus->lose_focus.emit();
			
								// Set focus	
								MouseFocus = C;
								MouseFocus->ProcessInput(Input, &Camera);
							}
						}
					}

					else
					{
						C->off_hover.emit();
					}
				}
			}
	    }

	    if (Input->IsKeyPressed(SDLK_ESCAPE))
	    {
	    	return false;
	    }

		return true;
	}
}}
