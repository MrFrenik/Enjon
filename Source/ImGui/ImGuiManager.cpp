#include "ImGui/ImGuiManager.h"
#include "ImGui/imgui_impl_sdl_gl3.h"
#include "Graphics/Camera.h"
#include "Graphics/Window.h"
#include "Asset/Asset.h"
#include "Graphics/Material.h"
#include "Graphics/Renderable.h"
#include "System/Types.h"
#include "Entity/EntityManager.h"
#include "Defines.h"
#include "Serialize/UUID.h" 
#include "Asset/AssetManager.h"
#include "Base/Object.h"
#include "Engine.h"

#include <algorithm>
#include <assert.h>
#include <iostream>

namespace Enjon
{
	std::vector<std::function<void()>> ImGuiManager::mGuiFuncs;
	std::vector<std::function<void()>> ImGuiManager::mWindows;
	std::unordered_map<std::string, std::vector<std::function<void()>>> ImGuiManager::mMainMenuOptions;
	std::vector<ImGui::DockingLayout> ImGuiManager::mDockingLayouts;

	//---------------------------------------------------
	void ImGuiManager::Init(SDL_Window* window)
	{
		assert(window != nullptr);

		// Init window
		ImGui_ImplSdlGL3_Init(window); 

		// Init style
		ImGuiStyles();

		// Initialize default windows/menus
		InitializeDefaults();
	}

	void ImGuiManager::ShutDown()
	{
		// Save dock
		// ImGui::SaveDock();

		// Shut down 
		ImGui_ImplSdlGL3_Shutdown();
	}

	//---------------------------------------------------
	void ImGuiManager::RegisterMenuOption(std::string name, std::function<void()> func)
	{
		// Will create the vector if not there
		mMainMenuOptions[name].push_back(func);
	}

	//---------------------------------------------------
	void ImGuiManager::Register(std::function<void()> func)
	{
		// TODO(): Search for function first before adding
		mGuiFuncs.push_back(func);
	}

	//---------------------------------------------------
	void ImGuiManager::RegisterWindow(std::function<void()> func)
	{
		mWindows.push_back(func);
	}

	void ImGuiManager::RenderGameUI(Window* window, f32* view, f32* projection)
	{
	    // Make a new window
		// ImGui_ImplSdlGL3_NewFrame(window);

		// Original screen coords
		auto dimensions = window->GetViewport();
		Enjon::Vec2 center = Enjon::Vec2((f32)dimensions.x / 2.0f, (f32)dimensions.y / 2.0f);

	    ImGuizmo::BeginFrame();
		static Mat4 model = Mat4::Identity();
		Enjon::Vec2 translate(0, 0);
    	ImGui::SliderFloat("Translate X", &translate.x, 0.0f, 1.0f);     // adjust display_format to decorate the value with a prefix or a suffix. Use power!=1.0 for logarithmic sliders
    	ImGui::SliderFloat("Translate Y", &translate.y, 0.0f, 1.0f);     // adjust display_format to decorate the value with a prefix or a suffix. Use power!=1.0 for logarithmic sliders

    	Vec3 scale(1, 1, 1);
    	ImGui::SliderFloat("Scale X", &scale.x, 0.01f, 1.0);     // adjust display_format to decorate the value with a prefix or a suffix. Use power!=1.0 for logarithmic sliders
    	ImGui::SliderFloat("Scale Y", &scale.y, 0.01f, 1.0);     // adjust display_format to decorate the value with a prefix or a suffix. Use power!=1.0 for logarithmic sliders
    	ImGui::SliderFloat("Scale Z", &scale.z, 0.01f, 1.0);     // adjust display_format to decorate the value with a prefix or a suffix. Use power!=1.0 for logarithmic sliders

    	model *= Mat4::Translate(Vec3(translate, 0.0f));
    	model *= Mat4::Scale(scale);

	    // Imguizmo 
	    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
	    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
	    if (ImGui::IsKeyPressed(90))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		if (ImGui::IsKeyPressed(69))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		if (ImGui::IsKeyPressed(82)) // r Key
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.elements, matrixTranslation, matrixRotation, matrixScale);
		ImGui::InputFloat3("Tr", matrixTranslation, 3);
		ImGui::InputFloat3("Rt", matrixRotation, 3);
		ImGui::InputFloat3("Sc", matrixScale, 3);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.elements);

		if (mCurrentGizmoOperation != ImGuizmo::SCALE)
		{
			if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
				mCurrentGizmoMode = ImGuizmo::LOCAL;
			ImGui::SameLine();
			if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
				mCurrentGizmoMode = ImGuizmo::WORLD;
		}
		static bool useSnap(false);
		if (ImGui::IsKeyPressed(83))
			useSnap = !useSnap;
		ImGui::Checkbox("", &useSnap);
		ImGui::SameLine();
		Enjon::Vec2 snap;

		/*
		switch (mCurrentGizmoOperation)
		{
		case ImGuizmo::TRANSLATE:
			snap = config.mSnapTranslation;
			ImGui::InputFloat3("Snap", &snap.x);
			break;
		case ImGuizmo::ROTATE:
			snap = config.mSnapRotation;
			ImGui::InputFloat("Angle Snap", &snap.x);
			break;
		case ImGuizmo::SCALE:
			snap = config.mSnapScale;
			ImGui::InputFloat("Scale Snap", &snap.x);
			break;
		}
		*/

		ImGuizmo::Manipulate(view, projection, mCurrentGizmoOperation, mCurrentGizmoMode, model.elements, NULL, useSnap ? &snap.x : NULL);
	}

	void DebugDumpProperty( Enjon::Object* object, Enjon::MetaProperty* prop )
	{
		Enjon::MetaClass* cls = const_cast< Enjon::MetaClass* >( object->Class( ) ); 
		Enjon::String name = prop->GetName( );

		switch ( prop->GetType( ) )
		{
			case Enjon::MetaPropertyType::U32:
			{
				u32 val = 0;
				cls->GetValue( object, prop, &val );
				Enjon::MetaPropertyTraits traits = prop->GetTraits( );
				ImGui::Text( Enjon::String( name + ":" ).c_str( ) );
				Enjon::String label( "##" + name );
				ImGui::SameLine( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderInt( label.c_str(), ( s32* )&val, ( s32 )traits.GetUIMin( ), ( s32 )traits.GetUIMax( ) ) )
					{
						cls->SetValue( object, prop, ( u32 )val );
					}
				}
				else
				{
					if ( ImGui::InputInt( label.c_str( ), ( s32* )&val, 0 ) )
					{
						cls->SetValue( object, prop, ( u32 )val );
					}
				}
			} break;

			case Enjon::MetaPropertyType::S32:
			{
				s32 val = 0;
				cls->GetValue( object, prop, &val );
				Enjon::MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderInt( name.c_str( ), ( s32* )&val, ( s32 )traits.GetUIMin( ), ( s32 )traits.GetUIMax( ) ) )
					{
						cls->SetValue( object, prop, ( s32 )val );
					}
				}
				else
				{
					if ( ImGui::InputInt( name.c_str( ), ( s32* )&val ) )
					{
						cls->SetValue( object, prop, ( s32 )val );
					}
				}
			} break;

			case Enjon::MetaPropertyType::F32:
			{
				float val = 0.0f;
				cls->GetValue( object, prop, &val );
				Enjon::MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderFloat( name.c_str( ), &val, traits.GetUIMin( ), traits.GetUIMax( ) ) )
					{
						cls->SetValue( object, prop, val );
					}
				}
				else
				{
					if ( ImGui::InputFloat( name.c_str( ), &val ) )
					{
						cls->SetValue( object, prop, val );
					}
				}
			} break;

			case Enjon::MetaPropertyType::Vec2:
			{
				Enjon::Vec2 val;
				cls->GetValue( object, prop, &val );
				Enjon::MetaPropertyTraits traits = prop->GetTraits( );
				f32 col[ 2 ] = { val.x, val.y };
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderFloat2( name.c_str( ), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						cls->SetValue( object, prop, val );
					}
				}
				else
				{
					if ( ImGui::InputFloat2( name.c_str( ), col ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						cls->SetValue( object, prop, val );
					}
				}
			} break;

			case Enjon::MetaPropertyType::Vec3:
			{
				Enjon::Vec3 val;
				cls->GetValue( object, prop, &val );
				f32 col[ 3 ] = { val.x, val.y, val.z };
				Enjon::MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderFloat3( name.c_str( ), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						val.z = col[ 2 ];
						cls->SetValue( object, prop, val );
					}
				}
				else
				{
					if ( ImGui::InputFloat3( name.c_str( ), col ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						val.z = col[ 2 ];
						cls->SetValue( object, prop, val );
					}
				}
			} break;

			case Enjon::MetaPropertyType::Vec4:
			{
				Enjon::Vec4 val;
				cls->GetValue( object, prop, &val );
				f32 col[ 4 ] = { val.x, val.y, val.z, val.w };
				Enjon::MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderFloat4( name.c_str( ), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						val.z = col[ 2 ];
						val.w = col[ 3 ];
						cls->SetValue( object, prop, val );
					}
				}
				else
				{
					if ( ImGui::InputFloat4( name.c_str( ), col ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						val.z = col[ 2 ];
						val.w = col[ 3 ];
						cls->SetValue( object, prop, val );
					}
				}
			} break;

			case Enjon::MetaPropertyType::ColorRGBA16:
			{
				Enjon::ColorRGBA16 val;
				cls->GetValue( object, prop, &val );
				f32 col[ 4 ] = { val.r, val.g, val.b, val.a };
				Enjon::MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderFloat4( name.c_str( ), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
					{
						val.r = col[ 0 ];
						val.g = col[ 1 ];
						val.b = col[ 2 ];
						val.a = col[ 3 ];
						cls->SetValue( object, prop, val );
					}
				}
				else
				{
					if ( ImGui::InputFloat4( name.c_str( ), col ) )
					{
						val.r = col[ 0 ];
						val.g = col[ 1 ];
						val.b = col[ 2 ];
						val.a = col[ 3 ];
						cls->SetValue( object, prop, val );
					}
				}
			} break;
				
			case Enjon::MetaPropertyType::String:
			{
				Enjon::String val;
				cls->GetValue( object, prop, &val );
				if ( ImGui::InputText( name.c_str( ), &val[ 0 ], val.size( ) ) )
				{
					cls->SetValue( object, prop, val ); 
				}
			} break;
				
			case Enjon::MetaPropertyType::UUID:
			{
				Enjon::UUID val;
				cls->GetValue( object, prop, &val );
				Enjon::String str = val.ToString( );
				ImGui::Text( name.c_str( ), &str[ 0 ], str.size( ) );
			} break;
		}
	}

	//---------------------------------------------------
			
	void ImGuiManager::DebugDumpObject( Enjon::Object* object )
	{
		if ( !object )
		{
			return;
		}

		Enjon::MetaClass* cls = const_cast< Enjon::MetaClass* >( object->Class( ) );

		ImGui::Text( ( "Type: " + std::string(object->GetTypeName( ) ) ).c_str( ) );

		Enjon::PropertyTable& pt = cls->GetProperties( );
		for ( auto& prop : pt )
		{
			Enjon::String name = prop.GetName( );

			switch ( prop.GetType( ) )
			{
				case Enjon::MetaPropertyType::U32: 
				case Enjon::MetaPropertyType::S32: 
				case Enjon::MetaPropertyType::F32: 
				case Enjon::MetaPropertyType::Vec2: 
				case Enjon::MetaPropertyType::Vec3: 
				case Enjon::MetaPropertyType::Vec4:
				case Enjon::MetaPropertyType::ColorRGBA16:
				case Enjon::MetaPropertyType::String:
				case Enjon::MetaPropertyType::UUID:
				{
					DebugDumpProperty( object, &prop );
				} break; 
				
				case Enjon::MetaPropertyType::Transform:
				{
					Enjon::Transform val;
					cls->GetValue( object, &prop, &val );
					Enjon::Vec3 pos = val.GetPosition( );
					Enjon::Quaternion rot = val.GetRotation( );
					Enjon::Vec3 scl = val.GetScale( );

					if ( ImGui::TreeNode( Enjon::String( prop.GetName( ) + "##" + std::to_string( (u32)object ) ).c_str( ) ) )
					{ 
						// Position
						{
							f32 col[ 3 ] = { pos.x, pos.y, pos.z };
							if ( ImGui::InputFloat3( Enjon::String( "Position##" + prop.GetName() ).c_str( ), col ) )
							{
								pos.x = col[ 0 ];
								pos.y = col[ 1 ];
								pos.z = col[ 2 ]; 
								val.SetPosition( pos );
								cls->SetValue( object, &prop, val );
							} 
						}
						
						// Rotation
						{
							f32 col[ 4 ] = { rot.x, rot.y, rot.z, rot.w };
							if ( ImGui::InputFloat4( Enjon::String( "Rotation##" + prop.GetName() ).c_str( ), col ) )
							{
								rot.x = col[ 0 ];
								rot.y = col[ 1 ];
								rot.z = col[ 2 ];
								val.SetRotation( rot );
								cls->SetValue( object, &prop, val );
							} 
						}
						
						// Scale
						{
							f32 col[ 3 ] = { scl.x, scl.y, scl.z };
							if ( ImGui::InputFloat3( Enjon::String( "Scale##" + prop.GetName() ).c_str( ), col ) )
							{
								scl.x = col[ 0 ];
								scl.y = col[ 1 ];
								scl.z = col[ 2 ];
								val.SetScale( scl );
								cls->SetValue( object, &prop, val );
							} 
						} 

						ImGui::TreePop( ); 
					} 

				} break;

				case Enjon::MetaPropertyType::AssetHandle:
				{
					Enjon::AssetHandle<Enjon::Asset> val;
					cls->GetValue( object, &prop, &val ); 
					if ( val )
					{
						Enjon::MetaClass* assetCls = const_cast< Enjon::MetaClass* >( val.GetAssetClass( ) );
						if ( assetCls )
						{ 
							Enjon::AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
							auto assets = am->GetAssets( assetCls ); 
							if ( ImGui::TreeNode( prop.GetName( ).c_str( ) ) )
							{
								if ( assets )
								{
									ImGui::ListBoxHeader( Enjon::String( "##" + prop.GetName( ) ).c_str( ) );
									{
										for ( auto& a : *assets )
										{
											if ( ImGui::Selectable( a.second->GetName( ).c_str( ) ) )
											{ 
												val.Set( a.second );
												cls->SetValue( object, &prop, val );
											}
										}
									} 
									ImGui::ListBoxFooter( ); 
								}
								ImGuiManager::DebugDumpObject( val.Get( ) ); 
								ImGui::TreePop( );
							}
						} 
					}

				} break;

				case Enjon::MetaPropertyType::Object:
				{
					Enjon::Object* obj = cls->GetValueAs< Enjon::Object >( object, &prop );
					if ( obj )
					{
						if ( ImGui::TreeNode( prop.GetName( ).c_str( ) ) )
						{
							ImGuiManager::DebugDumpObject( obj ); 
							ImGui::TreePop( ); 
						}
					}

				} break;

				case Enjon::MetaPropertyType::EntityHandle:
				{
					Enjon::EntityHandle handle;
					cls->GetValue( object, &prop, &handle );
					if ( handle.Get( ) )
					{
						if ( ImGui::TreeNode( prop.GetName( ).c_str( ) ) )
						{
							ImGuiManager::DebugDumpObject( handle.Get( ) );
							ImGui::TreePop( ); 
						}
					}

				} break;
			}
		} 
	}

	//---------------------------------------------------
	void ImGuiManager::Render(SDL_Window* window)
	{
	    // Make a new window
		ImGui_ImplSdlGL3_NewFrame(window);

		static bool show_scene1 = true;

		s32 menu_height = MainMenu();

	    if (ImGui::GetIO().DisplaySize.y > 0) 
		{
	        auto pos = ImVec2(0, menu_height);
	        auto size = ImGui::GetIO().DisplaySize;
	        size.y -= pos.y;
	        ImGui::RootDock(pos, ImVec2(size.x, size.y - 25.0f));

	        // Draw status bar (no docking)
	        ImGui::SetNextWindowSize(ImVec2(size.x, 25.0f), ImGuiSetCond_Always);
	        ImGui::SetNextWindowPos(ImVec2(0, size.y - 6.0f), ImGuiSetCond_Always);
	        ImGui::Begin("statusbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize);
	        ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
	        ImGui::End();
	    }

	    // Display all registered windows
	    for (auto& wind : mWindows)
	    {
	    	wind();
	    }
	}

	//---------------------------------------------------
	void ImGuiManager::RegisterDockingLayout(ImGui::DockingLayout& layout)
	{
		mDockingLayouts.push_back(layout);
	}

	//---------------------------------------------------
	void ImGuiManager::LateInit(SDL_Window* window)
	{
		Render(window);

		// Run through docking layouts here
    	for (auto& dl : mDockingLayouts)
    	{
    		ImGui::DockWith(dl.mChild, dl.mParent, dl.mSlotType, dl.mWeight);
    	}

    	// Clear docking layouts after to prevent from running through them again
    	mDockingLayouts.clear();
	}

	//---------------------------------------------------
	s32 ImGuiManager::MainMenu()
	{
		s32 menuHeight = 0;
		if (ImGui::BeginMainMenuBar())
		{
			// Display all menu options
			if (ImGui::BeginMenu("File"))
			{
				for (auto& sub : mMainMenuOptions["File"])
				{
					sub();		
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				for (auto& sub : mMainMenuOptions["View"])
				{
					sub();		
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				for (auto& sub : mMainMenuOptions["Help"])
				{
					sub();		
				}
				ImGui::EndMenu();
			}

			menuHeight = ImGui::GetWindowSize().y;

			ImGui::EndMainMenuBar();
		}

		return menuHeight;
	} 

	//------------------------------------------------------------------------------
	void ImGuiManager::ImGuiStyles()
	{
		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->Clear();
		io.Fonts->AddFontFromFileTTF("../Assets/Fonts/WeblySleek/weblysleekuisb.ttf", 16);
		io.Fonts->AddFontFromFileTTF("../Assets/Fonts/WeblySleek/weblysleekuisb.ttf", 14);
		io.Fonts->Build(); 

		// Grab reference to style
		ImGuiStyle& style = ImGui::GetStyle(); 

		style.WindowTitleAlign 		   = ImVec2(0.5f, 0.41f);
		style.ButtonTextAlign 		   = ImVec2(0.5f, 0.5f); 
		style.WindowPadding = ImVec2(6, 3);
		style.WindowRounding = 2.0f;
		style.FramePadding = ImVec2(6, 2);
		style.FrameRounding = 2.0f;
		style.ItemSpacing = ImVec2(8, 3);
		style.ItemInnerSpacing = ImVec2(2, 2);
		style.IndentSpacing = 21.0f;
		style.ScrollbarSize = 15.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabMinSize = 3.0f;
		style.GrabRounding = 3.0f;
		style.Alpha = 1.0f;
		style.FrameRounding = 3.0f; 

		style.Colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 0.61f);
		style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.40f, 0.40f, 0.40f, 0.81f);
		style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.09f, 0.11f, 0.13f, 0.31f);
		style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		style.Colors[ImGuiCol_Border]                = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.39f, 0.61f, 1.00f, 0.18f);
		style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.06f, 0.06f, 0.08f, 0.67f);
		style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
		style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.04f, 0.04f, 0.04f, 0.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.18f, 0.18f, 0.18f, 0.00f);
		style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.04f, 0.05f, 0.06f, 0.99f);
		style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.06f, 0.06f, 0.06f, 0.87f);
		style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.14f, 0.14f, 0.14f, 0.85f);
		style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.20f, 0.20f, 0.52f);
		style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.24f, 0.24f, 0.24f, 0.51f);
		style.Colors[ImGuiCol_Header]                = ImVec4(0.06f, 0.06f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.59f, 1.00f, 0.52f);
		style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Column]                = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.93f);
		style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
		style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		// Load dock
		// ImGui::LoadDock();
	}

	//--------------------------------------------------
	void ImGuiManager::InitializeDefaults()
	{
		mMainMenuOptions["File"].push_back([&]()
		{
			static bool on = false;
	    	ImGui::MenuItem("Save##file", NULL, &on);
		});
	}
}