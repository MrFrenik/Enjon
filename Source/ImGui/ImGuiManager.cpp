#include "ImGui/ImGuiManager.h" 
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
#include <fmt/format.h>

#include <algorithm>
#include <assert.h>
#include <iostream>

namespace Enjon
{
	//=============================================================

	Result ImGuiManager::Initialize( )
	{
		return Result::SUCCESS;
	}

	void ImGuiManager::Update( const f32 dT )
	{ 
	} 

	void ImGuiManager::Init(SDL_Window* window)
	{
		assert(window != nullptr);

		// Init window
		ImGui_ImplSdlGL3_Init(window); 

		// Init style
		ImGuiStyles();

		// Initialize default windows/menus
		InitializeDefaults(); 

		// Cache context
		mContext = ImGui_ImplSdlGL3_GetContext( );
	}

	Result ImGuiManager::Shutdown()
	{ 
		// Clear all functions and docks
		mGuiFuncs.clear( );
		mWindows.clear( );
		mMainMenuOptions.clear( );
		mDockingLayouts.clear( );

		// Shut down 
		ImGui_ImplSdlGL3_Shutdown();

		return Result::SUCCESS;
	}

	//==============================================================================================

	bool ImGuiManager::HasWindow( const String& windowName )
	{
		return ( mWindows.find( windowName ) != mWindows.end( ) );
	}

	//==============================================================================================

	bool ImGuiManager::HasMenuOption( const String& menu, const String& menuOptionName )
	{
		if ( mMainMenuOptions.find( menu ) != mMainMenuOptions.end( ) )
		{
			return ( mMainMenuOptions[menu].find( menuOptionName ) != mMainMenuOptions[menu].end( ) );
		}

		return false;
	}

	//---------------------------------------------------

	void ImGuiManager::RegisterMenuOption(const String& menuName, const String& optionName, std::function<void()> func)
	{
		// Will create the vector if not there
		mMainMenuOptions[menuName][optionName] = func;
	}

	//---------------------------------------------------
	void ImGuiManager::Register(std::function<void()> func)
	{
		// TODO(): Search for function first before adding
		mGuiFuncs.push_back(func);
	}

	//---------------------------------------------------
	void ImGuiManager::RegisterWindow(const String& windowName, std::function<void()> func)
	{
		if ( !HasWindow( windowName ) )
		{
			mWindows[windowName] = func; 
		}
	}

	void ImGuiManager::RenderGameUI(Window* window, f32* view, f32* projection)
	{
	    // Make a new window
		// ImGui_ImplSdlGL3_NewFrame(window);

		// Original screen coords
		auto dimensions = window->GetViewport();
		Enjon::Vec2 center = Enjon::Vec2((f32)dimensions.x / 2.0f, (f32)dimensions.y / 2.0f);
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
		if ( !prop || !object )
		{
			return;
		} 

		const Enjon::MetaClass* cls = object->Class( ); 
		Enjon::String name = prop->GetName( );

		f32 startCursorX = ImGui::GetCursorPosX( );
		f32 windowWidth = ImGui::GetWindowWidth( );

		if ( prop->GetType( ) != MetaPropertyType::Transform )
		{
			ImGui::Text( name.c_str( ) ); 
			ImGui::SameLine( );
			ImGui::SetCursorPosX( windowWidth * 0.4f );
			ImGui::PushItemWidth( windowWidth / 2.0f ); 
		}

		switch ( prop->GetType( ) )
		{
			case Enjon::MetaPropertyType::Bool:
			{
				bool val = 0;
				cls->GetValue( object, prop, &val );
				if ( ImGui::Checkbox( fmt::format("##{}", name).c_str(), &val ) )
				{
					cls->SetValue( object, prop, val );
				}

			} break;

			case Enjon::MetaPropertyType::U32:
			{
				u32 val = 0;
				cls->GetValue( object, prop, &val );
				Enjon::MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderInt( fmt::format("##{}", name).c_str(), ( s32* )&val, ( s32 )traits.GetUIMin( ), ( s32 )traits.GetUIMax( ) ) )
					{
						cls->SetValue( object, prop, ( u32 )val );
					}
				}
				else
				{
					if ( ImGui::DragInt( fmt::format("##{}", name).c_str(), ( s32* )&val ) )
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
					if ( ImGui::SliderInt( fmt::format("##{}", name).c_str( ), ( s32* )&val, ( s32 )traits.GetUIMin( ), ( s32 )traits.GetUIMax( ) ) )
					{
						cls->SetValue( object, prop, ( s32 )val );
					}
				}
				else
				{
					if ( ImGui::DragInt( fmt::format("##{}", name).c_str( ), ( s32* )&val ) )
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
					if ( ImGui::SliderFloat( fmt::format("##{}", name).c_str(), &val, traits.GetUIMin( ), traits.GetUIMax( ) ) )
					{
						cls->SetValue( object, prop, val );
					}
				}
				else
				{
					if ( ImGui::DragFloat( fmt::format("##{}", name).c_str(), &val ) )
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
					if ( ImGui::SliderFloat2( fmt::format("##{}", name).c_str(), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						cls->SetValue( object, prop, val );
					}
				}
				else
				{
					if ( ImGui::DragFloat2( fmt::format("##{}", name).c_str(), col ) )
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

			case Enjon::MetaPropertyType::iVec3:
			{
				Enjon::iVec3 val;
				cls->GetValue( object, prop, &val );
				s32 col[ 3 ] = { val.x, val.y, val.z };
				Enjon::MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderInt3( ( "##" + name ).c_str(), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						val.z = col[ 2 ];
						cls->SetValue( object, prop, val );
					}
				}
				else
				{ 
					if ( ImGui::DragInt3( ( "##" + name ).c_str(), col ) )
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
					if ( ImGui::SliderFloat4( fmt::format("##{}", name).c_str(), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
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
					if ( ImGui::DragFloat4( fmt::format("##{}", name).c_str(), col ) )
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
					if ( ImGui::SliderFloat4( fmt::format("##{}", name).c_str( ), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
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
					if ( ImGui::DragFloat4( fmt::format("##{}", name).c_str( ), col ) )
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
				if ( ImGui::InputText( fmt::format("##{}", name).c_str( ), buffer, 256 ) )
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
				ImGui::Text( fmt::format( "{}", str ).c_str( ) );
			} break;

			// Type is transform
			case Enjon::MetaPropertyType::Transform:
			{
				Enjon::Transform val;
				cls->GetValue( object, prop, &val );
				Enjon::Vec3 pos = val.GetPosition( );
				Enjon::Quaternion rot = val.GetRotation( );
				Enjon::Vec3 scl = val.GetScale( );

				// Position 
				ImGui::Text( fmt::format( "Position", prop->GetName( ) ).c_str( ) );
				ImGui::SameLine( );
				ImGui::SetCursorPosX( windowWidth * 0.4f );
				ImGui::PushItemWidth( windowWidth / 2.0f ); 
				{
					f32 col[ 3 ] = { pos.x, pos.y, pos.z };
					if ( ImGui::DragFloat3( Enjon::String( "##position" + prop->GetName() ).c_str( ), col ) )
					{
						pos.x = col[ 0 ];
						pos.y = col[ 1 ];
						pos.z = col[ 2 ]; 
						val.SetPosition( pos );
						cls->SetValue( object, prop, val );
					} 
				}
				ImGui::PopItemWidth( );
				
				// Rotation
				ImGui::Text( fmt::format( "Rotation", prop->GetName( ) ).c_str( ) );
				ImGui::SameLine( );
				ImGui::SetCursorPosX( windowWidth * 0.4f );
				ImGui::PushItemWidth( windowWidth / 2.0f ); 
				{ 
					f32 col[ 4 ] = { rot.x, rot.y, rot.z, rot.w };
					if ( ImGui::DragFloat4( Enjon::String( "##rotation" + prop->GetName() ).c_str( ), col ) )
					{
						rot.x = col[ 0 ];
						rot.y = col[ 1 ];
						rot.z = col[ 2 ];
						rot.w = col[ 3 ];

						val.SetRotation( rot );
						cls->SetValue( object, prop, val );
					} 
				}
				ImGui::PopItemWidth( );
				
				// Scale
				ImGui::Text( fmt::format( "Scale", prop->GetName( ) ).c_str( ) );
				ImGui::SameLine( );
				ImGui::SetCursorPosX( windowWidth * 0.4f );
				ImGui::PushItemWidth( windowWidth / 2.0f ); 
				{
					f32 col[ 3 ] = { scl.x, scl.y, scl.z };
					if ( ImGui::DragFloat3( Enjon::String( "##scale" + prop->GetName() ).c_str( ), col ) )
					{
						scl.x = col[ 0 ];
						scl.y = col[ 1 ];
						scl.z = col[ 2 ];
						val.SetScale( scl );
						cls->SetValue( object, prop, val );
					} 
				} 
				ImGui::PopItemWidth( );

			} break;

			// Enum type
			case Enjon::MetaPropertyType::Enum:
			{
				// Property is enum prop, so need to convert it
				const MetaPropertyEnum* enumProp = prop->Cast< MetaPropertyEnum >( ); 

				s32 enumInt = *cls->GetValueAs<s32>( object, prop ); 

				if ( ImGui::BeginCombo( "##enumProps", enumProp->GetEnumName().c_str() ) )
				{ 
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

						if ( ImGui::Selectable( e.Identifier( ).c_str() ) )
						{
							cls->SetValue( object, prop, e.Value( ) );
						} 

						if ( pushedColor )
						{
							ImGui::PopStyleColor( );
						}

					} 

					ImGui::EndCombo( );
				}

			} break;
		}
	} 

	ImGuiContext* ImGuiManager::GetContext( )
	{
		return mContext;
	}

	void ImGuiManager::InspectObject( const Object* object )
	{ 
		Result res = const_cast< Object* >( object )->OnEditorUI( );
		if ( res == Result::INCOMPLETE )
		{
			DebugDumpObject( object );
		}
	}
			
	void ImGuiManager::DebugDumpObject( const Enjon::Object* object )
	{ 
		if ( !object )
		{
			return;
		}

		const Enjon::MetaClass* cls = object->Class( ); 

		for ( usize i = 0; i < cls->GetPropertyCount( ); ++i )
		{
			// Grab property from class
			const MetaProperty* prop = cls ->GetProperty( i );

			// If not valid, then continue
			if ( !prop )
			{
				continue;
			}

			// If not visible in editor, then skip
			MetaPropertyTraits traits = prop->GetTraits( );
			if ( !traits.IsVisible( ) )
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
				case Enjon::MetaPropertyType::iVec3: 
				case Enjon::MetaPropertyType::Vec3: 
				case Enjon::MetaPropertyType::Vec4:
				case Enjon::MetaPropertyType::ColorRGBA32:
				case Enjon::MetaPropertyType::String:
				case Enjon::MetaPropertyType::UUID: 
				case Enjon::MetaPropertyType::Transform:
				case Enjon::MetaPropertyType::Bool:
				case Enjon::MetaPropertyType::Enum:
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
						AssetManager* am = EngineSubsystem( AssetManager );
						auto assets = am->GetAssets( assetCls ); 
						if ( ImGui::TreeNode( prop->GetName( ).c_str( ) ) )
						{
							if ( assets )
							{
								String label = val ? val->GetName( ) : assetCls->GetName( );
								if ( ImGui::BeginCombo( fmt::format("##{}", prop->GetName() ).c_str(), label.c_str() ) )
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
									ImGui::EndCombo( );
								} 
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
	        ImGui::Text("Frame: %.5f ms", 1000.0f / (f32)ImGui::GetIO().Framerate);
	        ImGui::End();
	    }

	    // Display all registered windows
	    for (auto& wind : mWindows)
	    {
			wind.second( );
	    }
	}

	//============================================================================================

	void ImGuiManager::ProcessEvent( SDL_Event* event )
	{
		ImGui_ImplSdlGL3_ProcessEvent( event ); 
	} 

	//============================================================================================

	void ImGuiManager::RegisterDockingLayout(const GUIDockingLayout& layout)
	{
		mDockingLayouts.push_back(layout);
	}

	//============================================================================================

	void ImGuiManager::LateInit(SDL_Window* window)
	{
		Render(window);

		// Run through docking layouts here
    	for (auto& dl : mDockingLayouts)
    	{
    		ImGui::DockWith(dl.mChild, dl.mParent, (ImGui::DockSlotType)(u32)dl.mSlotType, dl.mWeight);
    	}

    	// Clear docking layouts after to prevent from running through them again
    	mDockingLayouts.clear();
	}

	//============================================================================================

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
					sub.second( );
				}
				ImGui::EndMenu();
			}

			if ( ImGui::BeginMenu( "Create" ) )
			{
				for ( auto& sub : mMainMenuOptions[ "Create" ] )
				{
					sub.second( );
				}
				ImGui::EndMenu( );
			}

			if (ImGui::BeginMenu("View"))
			{
				for (auto& sub : mMainMenuOptions["View"])
				{
					sub.second( );
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				for (auto& sub : mMainMenuOptions["Help"])
				{
					sub.second();		
				}
				ImGui::EndMenu();
			}

			menuHeight = ImGui::GetWindowSize().y;

			ImGui::EndMainMenuBar();
		}

		return menuHeight;
	} 

	//============================================================================================

	ImFont* ImGuiManager::GetFont( const String& name )
	{
		auto query = mFonts.find( name );
		if ( query != mFonts.end( ) )
		{
			return mFonts[ name ];
		}

		return nullptr;
	}

	//============================================================================================

	void ImGuiManager::ImGuiStyles()
	{
		String rootPath = Engine::GetInstance()->GetConfig( ).GetRoot( );
		String fp = rootPath + "/Assets/Fonts/";

		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->Clear();
		mFonts["WeblySleek_10"] = io.Fonts->AddFontFromFileTTF(( fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 10);
		mFonts["WeblySleek_14"] = io.Fonts->AddFontFromFileTTF(( fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 14);
		mFonts["WeblySleek_16"] = io.Fonts->AddFontFromFileTTF(( fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 16);
		mFonts["WeblySleek_20"] = io.Fonts->AddFontFromFileTTF(( fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 20);
		mFonts["Roboto-MediumItalic_14"] = io.Fonts->AddFontFromFileTTF(( fp + "Roboto/Roboto-MediumItalic.ttf").c_str(), 14);
		mFonts["Roboto-MediumItalic_12"] = io.Fonts->AddFontFromFileTTF(( fp + "Roboto/Roboto-MediumItalic.ttf").c_str(), 12);
		io.Fonts->Build(); 

		// Grab reference to style
		ImGuiStyle& style = ImGui::GetStyle(); 

		// Set default font
		io.FontDefault = mFonts[ "WeblySleek_16" ];

		style.WindowTitleAlign 		= ImVec2(0.5f, 0.41f);
		style.ButtonTextAlign 		= ImVec2(0.5f, 0.5f); 
		style.WindowPadding			= ImVec2(10, 8);
		style.WindowRounding		= 0.0f;
		style.FramePadding			= ImVec2(10, 3);
		style.FrameRounding			= 2.0f;
		style.ItemSpacing			= ImVec2(9, 3);
		style.ItemInnerSpacing		= ImVec2(2, 3);
		style.IndentSpacing			= 20.0f;
		style.ScrollbarSize			= 14.0f;
		style.ScrollbarRounding		= 0.0f;
		style.GrabMinSize			= 5.0f;
		style.GrabRounding			= 2.0f;
		style.Alpha					= 1.0f;
		style.FrameBorderSize		= 0.0f;
		style.WindowBorderSize		= 1.0f;

		ImVec4* colors = ImGui::GetStyle( ).Colors;
		colors[ ImGuiCol_Text ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
		colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.50f, 0.50f, 0.50f, 0.57f );
		colors[ ImGuiCol_WindowBg ] = ImVec4( 0.12f, 0.12f, 0.12f, 1.00f );
		colors[ ImGuiCol_ChildBg ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
		colors[ ImGuiCol_PopupBg ] = ImVec4( 0.05f, 0.05f, 0.05f, 1.00f );
		colors[ ImGuiCol_Border ] = ImVec4( 0.12f, 0.12f, 0.12f, 0.45f );
		colors[ ImGuiCol_BorderShadow ] = ImVec4( 0.00f, 0.00f, 0.00f, 1.00f );
		colors[ ImGuiCol_FrameBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 0.61f );
		colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );
		colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.21f, 0.21f, 0.21f, 1.00f );
		colors[ ImGuiCol_TitleBg ] = ImVec4( 0.04f, 0.04f, 0.04f, 1.00f );
		colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.10f, 0.57f, 0.35f, 1.00f );
		colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
		colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
		colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.40f );
		colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.31f, 0.31f, 0.31f, 1.00f );
		colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
		colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
		colors[ ImGuiCol_CheckMark ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
		colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.07f, 0.39f, 0.71f, 1.00f );
		colors[ ImGuiCol_Button ] = ImVec4( 0.21f, 0.21f, 0.21f, 1.00f );
		colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
		colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );
		colors[ ImGuiCol_Header ] = ImVec4( 0.06f, 0.06f, 0.06f, 0.31f );
		colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_Separator ] = ImVec4( 0.29f, 0.29f, 0.29f, 0.50f );
		colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.13f, 0.77f, 1.00f, 0.25f );
		colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.10f, 0.40f, 0.75f, 1.00f );
		colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.08f, 0.41f, 0.74f, 1.00f );
		colors[ ImGuiCol_CloseButton ] = ImVec4( 0.41f, 0.41f, 0.41f, 0.50f );
		colors[ ImGuiCol_CloseButtonHovered ] = ImVec4( 0.98f, 0.39f, 0.36f, 1.00f );
		colors[ ImGuiCol_CloseButtonActive ] = ImVec4( 0.98f, 0.39f, 0.36f, 1.00f );
		colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
		colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
		colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
		colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
		colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
		colors[ ImGuiCol_ModalWindowDarkening ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.80f );
		colors[ ImGuiCol_DragDropTarget ] = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
		colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
		colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
		colors[ ImGuiCol_SelectableHovered ] = ImVec4( 0.14f, 0.23f, 0.32f, 1.00f );
		colors[ ImGuiCol_SelectableActive ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_Selectable ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_ComboBox ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
		colors[ ImGuiCol_ComboBoxHovered ] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );

		// Load dock
		// ImGui::LoadDock();
	}

	//=====================================================================

	void ImGuiManager::InitializeDefaults()
	{
		mMainMenuOptions["File"]["Save"] = ([&]()
		{
			static bool on = false;
	    	ImGui::MenuItem("Save##file", NULL, &on);
		});
	}

	//=====================================================================

	void ImGuiManager::BindContext( )
	{
		ImGui::SetCurrentContext( mContext );
	}

	//=====================================================================

	void ImGuiManager::Text( const String& text )
	{
		ImGui::Text( text.c_str( ) );
	}

	//=====================================================================

	void ImGuiManager::Separator( )
	{
		ImGui::Separator( );
	}
	
	//=====================================================================

	void ImGuiManager::PushFont( const String& fontName )
	{
		ImGui::PushFont( GetFont( fontName ) );
	}
	
	//=====================================================================

	void ImGuiManager::PopFont( )
	{
		ImGui::PopFont( );
	}
	
	//=====================================================================

	bool ImGuiManager::DragFloat2( const String& label, Vec2* vec, f32 speed, f32 min, f32 max )
	{
		return ImGui::DragFloat2( label.c_str( ), (float*)vec, speed, min, max );
	}

	//=====================================================================

	GUIWidget::GUIWidget( const String& label )
		: mLabel( label )
	{ 
	}

	//===================================================================== 

	GUIWidget::GUIWidget( const String& label, const Vec2& position, const Vec2& size )
		: mLabel( label ), mPosition( position ), mSize( size )
	{ 
	}

	//===================================================================== 

	void GUIWidget::SetSize( const Vec2& size )
	{ 
		mSize = size;
		mAutoCalculateSize = false;
	}

	//===================================================================== 

	Vec2 GUIWidget::GetSize( )
	{
		return mSize;
	}

	//===================================================================== 

	Vec2 GUIWidget::GetPosition( )
	{ 
		return mPosition;
	}

	//===================================================================== 

	PopupWindow::PopupWindow( const String& label, const Vec2& position, const Vec2& size )
		: GUIWidget( label, position, size )
	{ 
	}

	//===================================================================== 

	void PopupWindow::operator+=( const GUICallbackFunc& func )
	{
		this->RegisterCallback( func );
	}

	//===================================================================== 

	void PopupWindow::RegisterCallback( const GUICallbackFunc& func )
	{
		mCallbacks.push_back( func );
	}

	//===================================================================== 

	void PopupWindow::SetFadeInSpeed( const f32& speed )
	{
		mFadeInSpeed = speed;
	}

	//===================================================================== 

	void PopupWindow::SetFadeOutSpeed( const f32& speed )
	{
		mFadeOutSpeed = speed;
	}

	//===================================================================== 

	void PopupWindow::Activate( const Vec2& position )
	{
		mPosition = position; 
		mEnabled = true;
		mFadeTimer = 0.0f;
	}

	//===================================================================== 

	void PopupWindow::Deactivate( )
	{
		mBeginDisable = true;
		mFadeTimer = 0.0f;
	}

	//===================================================================== 

	void PopupWindow::DoWidget( )
	{
		if ( !mEnabled )
		{
			return;
		}

		// Get display size
		ImVec2 dispSize = ImGui::GetIO( ).DisplaySize; 

		ImVec2 position = ImVec2( mPosition.x, mPosition.y );

		// Calculate max position
		ImVec2 popupMaxPos = ImVec2( position.x + mSize.x, position.y + mSize.y ); 
 
		// Clamp menu position to inside of window
		if ( popupMaxPos.x > dispSize.x )
		{ 
			position.x = dispSize.x - mSize.x;
		} 

		// Clamp to being inside of the window and not negative
		position.x = Max( position.x, 0.0f ); 

		// If below half-way ( offset by size of window )
		if ( position.y > dispSize.y / 2.0f )
		{
			position.y -= mSize.y;
		}

		// Calculate popupbg color using fade
		ImVec4 bgColor = ImColor( ImGui::GetColorU32( ImGuiCol_WindowBg ) );
		f32 bgAlpha = 1.0f;
		if ( !mBeginDisable )
		{
			// Increment fade timer by delta time
			mFadeTimer += mFadeInSpeed * Engine::GetInstance( )->GetWorldTime( ).GetDeltaTime( );
			
			// Calculate fade amount for animation 
			bgAlpha =  Clamp( mFadeTimer / 1.0f, 0.0f, 1.0f ); 
		}
		else
		{
			// Increment fade timer by delta time
			mFadeOutTimer -= mFadeOutSpeed * Engine::GetInstance( )->GetWorldTime( ).GetDeltaTime( ); 
			mFadeOutTimer = Clamp( mFadeOutTimer, 0.0f, 1.0f ); 
			bgAlpha = mFadeOutTimer;
		}

		// Push background color
		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( bgColor.x, bgColor.y, bgColor.z, bgAlpha ) ); 

		// Set window position
		ImGui::SetNextWindowPos( position );

		// Set window size
		if ( !mAutoCalculateSize )
		{
			ImGui::SetNextWindowSize( ImVec2( mSize.x, mSize.y ) );
		}

		// Open popup window
		const char* label = fmt::format( "{}##{}", mLabel, (u32)this ).c_str();
		//ImGui::OpenPopup( label );
		ImGui::Begin( label, &mEnabled, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar );
		{
			// Capture hovered state
			ImVec2 pa = position;
			ImVec2 pb = ImVec2( position.x + mSize.x, position.y + mSize.y );
			mHovered = ImGui::IsMouseHoveringRect( pa, pb ); 

			// Do all callbacks
			for ( auto& f : mCallbacks )
			{
				f( );
			}

			// If any of the callbacks deactivated the popup, then close it
			if ( mBeginDisable )
			{
				if ( mFadeOutTimer <= 0.0f )
				{ 
					Reset( );
				}
			}

			// If need to calculate size, then do so
			if ( mAutoCalculateSize )
			{
				// Reset size based on popup content ( questionable... )
				mSize = Vec2( ImGui::GetWindowSize( ).x, ImGui::GetWindowSize( ).y ); 
			}
		} 
		ImGui::End( );

		// Pop color vars
		ImGui::PopStyleColor( );
	}

	//===================================================================== 

	void PopupWindow::Reset( )
	{ 
		mBeginDisable = false;
		mEnabled = false;
		mFadeTimer = 0.0f;
		mFadeInSpeed = 10.0f;
		mFadeOutSpeed = 20.0f;
		mFadeOutTimer = 1.0f;
	}

	//===================================================================== 

	bool PopupWindow::Enabled( )
	{
		return mEnabled;
	}

	//===================================================================== 

	bool PopupWindow::Hovered( )
	{
		return mHovered;
	}

	//===================================================================== 

	DockingWindow::DockingWindow( const String& label, const Vec2& position, const Vec2& size, u32 viewFlags )
		: GUIWidget( label, position, size ), mViewFlags( viewFlags )
	{
		// Register window on construction
		ImGuiManager* igm = EngineSubsystem( ImGuiManager ); 

		igm->RegisterMenuOption( "View", mLabel, [ & ] ( )
		{
			ImGui::MenuItem( fmt::format( "{}##options", mLabel ).c_str( ), NULL, &mEnabled );
		});

		// Register individual window with docking system
		igm->RegisterWindow( mLabel, [ & ] ( )
		{
			// Docking windows
			if ( ImGui::BeginDock( mLabel.c_str(), &mEnabled, mViewFlags ) )
			{
				DoWidget( );
			}
			ImGui::EndDock( ); 
		}); 
	}

	//===================================================================== 

	DockingWindow::~DockingWindow( )
	{ 
		// Register window on construction
		ImGuiManager* igm = EngineSubsystem( ImGuiManager ); 
	}

	//===================================================================== 
}