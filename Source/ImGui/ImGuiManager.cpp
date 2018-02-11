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
#include "Physics/CollisionShape.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <fmt/printf.h> 

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

		// Clear all functions and docks
		mGuiFuncs.clear( );
		mWindows.clear( );
		mMainMenuOptions.clear( );
		mDockingLayouts.clear( );

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

#define MAP_KEY_PRIMITIVE( keyType, valType, ImGuiCastType, ImGuiFunc, object, prop )\
	{\
		const MetaPropertyHashMap< keyType, valType >* mapProp = prop->Cast< MetaPropertyHashMap< keyType, valType > >();\
		for ( auto iter = mapProp->Begin( object ); iter != mapProp->End( object ); ++iter )\
		{\
			valType val = mapProp->GetValueAs( object, iter );\
			Enjon::String label( "##" + propName + std::to_string( iter->first ) );\
			ImGui::Text( ( "Key: " + std::to_string( iter->first ) ).c_str( ) );\
			ImGui::SameLine( );\
			if ( ImGuiFunc( label.c_str( ), ( ImGuiCastType* )&val, mapProp->GetTraits( ).GetUIMin( ), mapProp->GetTraits( ).GetUIMax( ) ) )\
			{\
				mapProp->SetValueAt( object, iter, val );\
			}\
		}\
	}

#define MAP_KEY_STRING( valType, ImGuiCastType, ImGuiFunc, object, prop )\
	{\
		const MetaPropertyHashMap< String, valType >* mapProp = prop->Cast< MetaPropertyHashMap< String, valType > >( );\
		for ( auto iter = mapProp->Begin( object ); iter != mapProp->End( object ); ++iter )\
		{\
			valType val = mapProp->GetValueAs( object, iter );\
			Enjon::String label( "##" + propName + iter->first );\
			ImGui::Text( ( "Key: " + iter->first ).c_str( ) );\
			ImGui::SameLine( );\
			if ( ImGuiFunc( label.c_str( ), ( ImGuiCastType* )&val, mapProp->GetTraits( ).GetUIMin( ), mapProp->GetTraits( ).GetUIMax( ) ) )\
			{\
				mapProp->SetValueAt( object, iter, val );\
			}\
		}\
	} 

	void ImGuiManager::DebugDumpHashMapProperty( const Enjon::Object* object, const Enjon::MetaPropertyHashMapBase* prop )
	{
		const MetaClass* cls = object->Class( );
		String propName = prop->GetName( );

		switch ( prop->GetKeyType( ) )
		{
			case MetaPropertyType::F32:
			{ 
				switch ( prop->GetValueType() )
				{
					case MetaPropertyType::U32:		MAP_KEY_PRIMITIVE( f32, u32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::S32:		MAP_KEY_PRIMITIVE( f32, s32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::F32:		MAP_KEY_PRIMITIVE( f32, f32, f32, ImGui::InputFloat, object, prop )	break;
				} 
			} break;

			case MetaPropertyType::S32:
			{ 
				switch ( prop->GetValueType() )
				{
					case MetaPropertyType::U32:		MAP_KEY_PRIMITIVE( s32, u32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::S32:		MAP_KEY_PRIMITIVE( s32, s32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::F32:		MAP_KEY_PRIMITIVE( s32, f32, f32, ImGui::InputFloat, object, prop )	break;
				} 
			} break;

			case MetaPropertyType::U32:
			{ 
				switch ( prop->GetValueType() )
				{
					case MetaPropertyType::U32:		MAP_KEY_PRIMITIVE( u32, u32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::S32:		MAP_KEY_PRIMITIVE( u32, s32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::F32:		MAP_KEY_PRIMITIVE( u32, f32, f32, ImGui::InputFloat, object, prop )	break;
				} 
			} break;

			case MetaPropertyType::String:
			{
				switch ( prop->GetValueType( ) )
				{
					case MetaPropertyType::U32:		MAP_KEY_STRING( u32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::S32:		MAP_KEY_STRING( s32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::F32:		MAP_KEY_STRING( f32, f32, ImGui::InputFloat, object, prop ) break;
					case MetaPropertyType::Object: 
					{
						const MetaPropertyHashMap< String, Object* >* mapProp = prop->Cast< MetaPropertyHashMap< String, Object* > >( );
						for ( auto iter = mapProp->Begin( object ); iter != mapProp->End( object ); ++iter )
						{
							Object* val = mapProp->GetValueAs( object, iter );
							Enjon::String label( "##" + propName + iter->first );
							ImGui::Text( ( "Key: " + iter->first ).c_str( ) );
							if ( ImGui::TreeNode( ( iter->first ).c_str( ) ) )
							{
								DebugDumpObject( iter->second );
								ImGui::TreePop( );
							}
						}
					} break;
				}
			} break;
		}
	}

#define ARRAY_PROP( prop, propName, type, object, ImGuiCastType, ImGuiFunction, min, max )\
	{\
		const MetaPropertyArray< type >* arrayProp = prop->Cast< MetaPropertyArray< type > >();\
		for ( usize i = 0; i < arrayProp->GetSize( object ); ++i )\
		{\
			Enjon::String label( "##" + propName + std::to_string(i) );\
			type val = arrayProp->GetValueAs( object, i );\
			if ( ImGuiFunction( label.c_str(), (ImGuiCastType*)&val, min, max ) )\
			{\
				arrayProp->SetValueAt( object, i, val );\
			}\
		}\
	}

	void ImGuiManager::DebugDumpArrayProperty( const Enjon::Object* object, const Enjon::MetaPropertyArrayBase* prop )
	{ 
		const MetaClass* cls = object->Class( );
		String propName = prop->GetName( ); 

		switch ( prop->GetArrayType( ) )
		{
			case MetaPropertyType::U32:	ARRAY_PROP( prop, propName, u32, object, s32, ImGui::InputInt, prop->GetTraits().GetUIMin(), prop->GetTraits().GetUIMax() )		break; 
			case MetaPropertyType::S32: ARRAY_PROP( prop, propName, s32, object, s32, ImGui::InputInt, prop->GetTraits().GetUIMin(), prop->GetTraits().GetUIMax() )		break; 
			case MetaPropertyType::F32: ARRAY_PROP( prop, propName, f32, object, f32, ImGui::InputFloat, prop->GetTraits().GetUIMin(), prop->GetTraits().GetUIMax() )	break; 

			case MetaPropertyType::String:
			{ 
				const MetaPropertyArray< String >* arrayProp = prop->Cast< MetaPropertyArray< String > >( );
				for ( usize i = 0; i < arrayProp->GetSize( object ); ++i ) 
				{
					String val = arrayProp->GetValueAs( object, i );
					char buffer[ 256 ];
					std::strncpy( buffer, val.c_str( ), 256 );
					String label( "##" + propName + std::to_string( i ) );
					if ( ImGui::InputText( label.c_str(), buffer, 256 ) )
					{
						arrayProp->SetValueAt( object, i, String( buffer ) );
					}
				} 
			} break;

			case MetaPropertyType::Bool:
			{ 
			} break;

			case MetaPropertyType::Object:
			{
				const MetaPropertyArray< Object* >* arrayProp = static_cast< const MetaPropertyArray< Object* >* >( prop );
				if ( arrayProp )
				{
					for ( usize i = 0; i < arrayProp->GetSize( object ); ++i )
					{
						const Object* arrObj = arrayProp->GetValueAs( object, i );
						const MetaClass* arrPropCls = arrObj->Class( ); 

						if ( ImGui::TreeNode( Enjon::String( arrPropCls->GetName() + "##" + std::to_string(u32(arrayProp->GetValueAs( object, i ) ) ) ).c_str( ) ) )
						{
							DebugDumpObject( arrayProp->GetValueAs( object, i ) );
							ImGui::TreePop( );
						}
					}
				}
			} break;

			case MetaPropertyType::EntityHandle:
			{
				const MetaPropertyArray< EntityHandle >* arrayProp = static_cast< const MetaPropertyArray< EntityHandle >* >( prop );
				if ( arrayProp )
				{
					for ( usize i = 0; i < arrayProp->GetSize( object ); ++i )
					{
						EntityHandle arrObj = arrayProp->GetValueAs( object, i );
						if ( arrObj.Get( ) )
						{
							const MetaClass* arrPropCls = arrObj.Get( )->Class( );
							if ( ImGui::TreeNode( Enjon::String( arrPropCls->GetName( ) + "##" + std::to_string( u32( arrObj.GetID() ) ) ).c_str( ) ) )
							{
								DebugDumpObject( arrObj.Get( ) );
								ImGui::TreePop( );
							}
						}
					}
				}
			} break;

			case MetaPropertyType::AssetHandle:
			{
				MetaArrayPropertyProxy proxy = prop->GetProxy( ); 
				const MetaPropertyTemplateBase* base = static_cast<const MetaPropertyTemplateBase*> ( proxy.mArrayPropertyTypeBase );
				const MetaClass* assetCls = const_cast<Enjon::MetaClass*>( base->GetClassOfTemplatedArgument( ) ); 
				
				// Property is of type MetaPropertyAssetHandle
				const MetaPropertyArray< AssetHandle< Asset > >* arrayProp = static_cast<const MetaPropertyArray< AssetHandle< Asset > > * >( prop ); 
				if ( assetCls )
				{ 
					for ( usize i = 0; i < arrayProp->GetSize( object ); ++i )
					{
						Enjon::AssetHandle<Enjon::Asset> val; 
						arrayProp->GetValueAt( object, i, &val );
						const Enjon::AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
						auto assets = am->GetAssets( assetCls ); 
						if ( ImGui::TreeNode( Enjon::String( std::to_string( i ) + "##" + prop->GetName( ) + std::to_string(u32(arrayProp) ) ).c_str( ) ) )
						{
							if ( assets )
							{
								ImGui::ListBoxHeader( Enjon::String( "##" + std::to_string( i ) + prop->GetName( ) ).c_str( ) );
								{
									u32 assetIndex = 0;
									// All asset record info struct
									for ( auto& a : *assets )
									{
										if ( ImGui::Selectable( String( std::to_string( assetIndex ) + ": " +  a.second.GetAssetName() ).c_str( ) ) )
										{ 
											val.Set( const_cast< Asset* >( a.second.GetAsset() ) );
											arrayProp->SetValueAt( object, i, val );
										}
										assetIndex++;
									}
								} 
								ImGui::ListBoxFooter( ); 
							}

							if ( val )
							{
								ImGuiManager::DebugDumpObject( val.Get( ) );
							}

							ImGui::TreePop( );
						}
					} 
				} 

			} break;
		}
	}

	void ImGuiManager::DebugDumpProperty( const Enjon::Object* object, const Enjon::MetaProperty* prop )
	{
		const Enjon::MetaClass* cls = object->Class( ); 
		Enjon::String name = prop->GetName( );

		switch ( prop->GetType( ) )
		{
			case Enjon::MetaPropertyType::U32:
			{
				u32 val = 0;
				cls->GetValue( object, prop, &val );
				Enjon::MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderInt( name.c_str(), ( s32* )&val, ( s32 )traits.GetUIMin( ), ( s32 )traits.GetUIMax( ) ) )
					{
						cls->SetValue( object, prop, ( u32 )val );
					}
				}
				else
				{
					if ( ImGui::DragInt( name.c_str( ), ( s32* )&val ) )
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
					if ( ImGui::DragInt( name.c_str( ), ( s32* )&val ) )
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
					if ( ImGui::DragFloat( name.c_str( ), &val ) )
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
					if ( ImGui::DragFloat2( name.c_str( ), col ) )
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
				ImGui::Text( name.c_str( ) ); ImGui::SameLine( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderFloat3( ( "##" + name ).c_str(), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						val.z = col[ 2 ];
						cls->SetValue( object, prop, val );
					}
				}
				else
				{ 
					if ( ImGui::DragFloat3( ( "##" + name ).c_str(), col ) )
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
					if ( ImGui::DragFloat4( name.c_str( ), col ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						val.z = col[ 2 ];
						val.w = col[ 3 ];
						cls->SetValue( object, prop, val );
					}
				}
			} break;

			case Enjon::MetaPropertyType::ColorRGBA32:
			{
				Enjon::ColorRGBA32 val;
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
					if ( ImGui::DragFloat4( name.c_str( ), col ) )
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
				char buffer[ 256 ];
				strncpy_s( buffer, &val[0], 256 );
				if ( ImGui::InputText( name.c_str( ), buffer, 256 ) )
				{
					// Reset string
					cls->SetValue( object, prop, String( buffer ) ); 
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
			
	void ImGuiManager::DebugDumpObject( const Enjon::Object* object )
	{
		if ( !object )
		{
			return;
		}

		const Enjon::MetaClass* cls = object->Class( );

		ImGui::Text( ( "Type: " + std::string(cls->GetName( ) ) ).c_str( ) );

		for ( usize i = 0; i < cls->GetPropertyCount( ); ++i )
		{
			// Grab property from class
			const MetaProperty* prop = cls ->GetProperty( i );

			if ( !prop )
			{
				continue;
			}

			// Get property name
			Enjon::String name = prop->GetName( );

			switch ( prop->GetType( ) )
			{
				// Primitive types
				case Enjon::MetaPropertyType::U32: 
				case Enjon::MetaPropertyType::S32: 
				case Enjon::MetaPropertyType::F32: 
				case Enjon::MetaPropertyType::Vec2: 
				case Enjon::MetaPropertyType::Vec3: 
				case Enjon::MetaPropertyType::Vec4:
				case Enjon::MetaPropertyType::ColorRGBA32:
				case Enjon::MetaPropertyType::String:
				case Enjon::MetaPropertyType::UUID: 
				{
					DebugDumpProperty( object, prop );
				} break; 

				// Array type
				case Enjon::MetaPropertyType::Array:
				{
					if ( ImGui::TreeNode( Enjon::String( prop->GetName( ) + "##" + std::to_string( (u32)object ) ).c_str( ) ) )
					{
						const MetaPropertyArrayBase* arrayProp = prop->Cast< MetaPropertyArrayBase >( );
						DebugDumpArrayProperty( object, arrayProp );
						ImGui::TreePop( );
					}
				} break;

				case Enjon::MetaPropertyType::HashMap: 
				{

					if ( ImGui::TreeNode( Enjon::String( prop->GetName() + "##" + std::to_string( (u32)object ) ).c_str() ) )
					{
						const MetaPropertyHashMapBase* mapProp = prop->Cast< MetaPropertyHashMapBase >();
						DebugDumpHashMapProperty( object, mapProp );
						ImGui::TreePop( );
					}

				} break;

				// Enum type
				case Enjon::MetaPropertyType::Enum:
				{
					// Property is enum prop, so need to convert it
					const MetaPropertyEnum* enumProp = prop->Cast< MetaPropertyEnum >( ); 

					if ( ImGui::TreeNode( prop->GetName().c_str() ) )
					{
						ImGui::ListBoxHeader( "##enumProps" );
						{
							s32 enumInt = *cls->GetValueAs<s32>( object, prop ); 

							// For each element in the enum
							for ( auto& e : enumProp->GetElements( ) )
							{ 
								// Has this value, so need to display it differently
								bool pushedColor = false;
								if ( e.Value( ) == enumInt )
								{
									ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.8f, 0.3f, 0.1f, 1.0f ) );
									pushedColor = true;
								}

								if ( ImGui::Selectable( e.Identifier( ).c_str( ) ) )
								{
									cls->SetValue( object, prop, e.Value( ) );
								} 

								if ( pushedColor )
								{
									ImGui::PopStyleColor( );
								}

							} 
						}
						ImGui::ListBoxFooter( ); 
						ImGui::TreePop( );
					}

				} break;
				
				// Type is transform
				case Enjon::MetaPropertyType::Transform:
				{
					Enjon::Transform val;
					cls->GetValue( object, prop, &val );
					Enjon::Vec3 pos = val.GetPosition( );
					Enjon::Quaternion rot = val.GetRotation( );
					Enjon::Vec3 scl = val.GetScale( );

					if ( ImGui::TreeNode( Enjon::String( prop->GetName( ) + "##" + std::to_string( (u32)object ) ).c_str( ) ) )
					{ 
						// Position
						{
							f32 col[ 3 ] = { pos.x, pos.y, pos.z };
							if ( ImGui::DragFloat3( Enjon::String( "Position##" + prop->GetName() ).c_str( ), col ) )
							{
								pos.x = col[ 0 ];
								pos.y = col[ 1 ];
								pos.z = col[ 2 ]; 
								val.SetPosition( pos );
								cls->SetValue( object, prop, val );
							} 
						}
						
						// Rotation
						{
							f32 col[ 4 ] = { rot.x, rot.y, rot.z, rot.w };
							if ( ImGui::DragFloat4( Enjon::String( "Rotation##" + prop->GetName() ).c_str( ), col ) )
							{
								rot.x = col[ 0 ];
								rot.y = col[ 1 ];
								rot.z = col[ 2 ];
								val.SetRotation( rot );
								cls->SetValue( object, prop, val );
							} 
						}
						
						// Scale
						{
							f32 col[ 3 ] = { scl.x, scl.y, scl.z };
							if ( ImGui::DragFloat3( Enjon::String( "Scale##" + prop->GetName() ).c_str( ), col ) )
							{
								scl.x = col[ 0 ];
								scl.y = col[ 1 ];
								scl.z = col[ 2 ];
								val.SetScale( scl );
								cls->SetValue( object, prop, val );
							} 
						} 

						ImGui::TreePop( ); 
					} 

				} break;

				// AssetHandle type
				case Enjon::MetaPropertyType::AssetHandle:
				{
					// Property is of type MetaPropertyAssetHandle
					const MetaPropertyTemplateBase* base = prop->Cast< MetaPropertyTemplateBase >( );
					const MetaClass* assetCls = base->GetClassOfTemplatedArgument( );

					if ( assetCls )
					{ 
						Enjon::AssetHandle<Enjon::Asset> val; 
						cls->GetValue( object, prop, &val );
						const Enjon::AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
						auto assets = am->GetAssets( assetCls ); 
						if ( ImGui::TreeNode( prop->GetName( ).c_str( ) ) )
						{
							if ( assets )
							{
								ImGui::ListBoxHeader( Enjon::String( "##" + prop->GetName( ) ).c_str( ) );
								{
									// For each record in assets
									for ( auto& a : *assets )
									{
										if ( ImGui::Selectable( a.second.GetAssetName().c_str( ) ) )
										{ 
											val.Set( const_cast< Asset* > ( a.second.GetAsset() ) );
											cls->SetValue( object, prop, val );
										}
									}
								} 
								ImGui::ListBoxFooter( ); 
							}
							if ( val )
							{
								ImGuiManager::DebugDumpObject( val.Get( ) ); 
							}
							ImGui::TreePop( );
						}
					} 

				} break;

				// Object type
				case Enjon::MetaPropertyType::Object:
				{ 
					if ( prop->GetTraits( ).IsPointer( ) )
					{
						const MetaPropertyPointerBase* base = prop->Cast< MetaPropertyPointerBase >( );
						const Enjon::Object* obj = base->GetValueAsObject( object );
						if ( obj )
						{
							if ( ImGui::TreeNode( prop->GetName( ).c_str( ) ) )
							{
								ImGuiManager::DebugDumpObject( obj ); 
								ImGui::TreePop( );
							}
						}
					}
					else
					{
						const Enjon::Object* obj = cls->GetValueAs< Enjon::Object >( object, prop );
						if ( obj )
						{
							if ( ImGui::TreeNode( prop->GetName( ).c_str( ) ) )
							{
								ImGuiManager::DebugDumpObject( obj ); 
								ImGui::TreePop( ); 
							}
						} 
					}

				} break;

				// Entity handle type
				case Enjon::MetaPropertyType::EntityHandle:
				{
					Enjon::EntityHandle handle;
					cls->GetValue( object, prop, &handle );
					if ( handle.Get( ) )
					{
						if ( ImGui::TreeNode( prop->GetName( ).c_str( ) ) )
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

		// TODO(): Need to organize this in a much better manner...
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

			if ( ImGui::BeginMenu( "Create" ) )
			{
				for ( auto& sub : mMainMenuOptions[ "Create" ] )
				{
					sub( );
				}
				ImGui::EndMenu( );
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
		String rootPath = Engine::GetInstance()->GetConfig( ).GetRoot( );
		String fp = rootPath + "/Assets/Fonts/";

		ImGuiIO& io = ImGui::GetIO();

		//io.Fonts->Clear();
		io.Fonts->AddFontFromFileTTF(( fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 16);
		io.Fonts->AddFontFromFileTTF(( fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 14);
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