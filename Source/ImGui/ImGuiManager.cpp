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
#include "Serialize/ObjectArchiver.h"
#include "Utils/FileUtils.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include "Graphics/GraphicsSubsystem.h"
#include "SubsystemCatalog.h"

#include <algorithm>
#include <assert.h>
#include <iostream>

namespace Enjon
{
	//=============================================================

	bool GUIContext::HasMainMenu( const String& menu )
	{
		return ( mMainMenuOptions.find( menu ) != mMainMenuOptions.end( ) );
	}

	//=============================================================

	bool GUIContext::HasWindow( const String& windowName )
	{
		return ( mWindows.find( windowName ) != mWindows.end( ) );
	}

	//==============================================================================================

	bool GUIContext::HasMainMenuOption( const String& menu, const String& menuOptionName )
	{
		if ( mMainMenuOptions.find( menu ) != mMainMenuOptions.end( ) )
		{
			return ( mMainMenuOptions[menu].find( menuOptionName ) != mMainMenuOptions[menu].end( ) );
		}

		return false;
	}

	//===================================================================================================

	void GUIContext::CreateMainMenu( const String& menuName )
	{
		mMainMenuOptions[ menuName ] = HashMap< String, GUICallbackFunc >( );
		mMainMenuLayout.push_back( menuName );
	}

	//===================================================================================================

	void GUIContext::RegisterMenuOption(const String& menuName, const String& optionName, const GUICallbackFunc& func)
	{
		// Create main menu if not already available
		if ( !HasMainMenu( menuName ) )
		{
			CreateMainMenu( menuName ); 
		}

		// Will create the vector if not there
		mMainMenuOptions[menuName][optionName] = func; 
	} 

	//===================================================================================================

	void GUIContext::Register( const GUICallbackFunc& func)
	{
		// TODO(): Search for function first before adding
		mGuiFuncs.push_back(func);
	}

	//===================================================================================================

	void GUIContext::RegisterDockingWindow( const String& dockName, const GUICallbackFunc& windowFunc )
	{
		auto dockFunc = [ & ] ( )
		{
			if ( ImGui::BeginDock( dockName.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize ) )
			{
				windowFunc();
				ImGui::EndDock( ); 
			}
		};

		RegisterWindow( dockName, dockFunc );
	}

	//===================================================================================================

	void GUIContext::RegisterWindow(const String& windowName, const GUICallbackFunc& func)
	{
		if ( !HasWindow( windowName ) )
		{
			mWindows[windowName] = func; 
		}
	}

	//===================================================================================================

	void GUIContext::RegisterDockingLayout(const GUIDockingLayout& layout)
	{
		mDockingLayouts.push_back(layout);
	} 

	//===================================================================================================

	void GUIContext::RegisterMainMenu( const String& menuName )
	{
		if ( !HasMainMenu( menuName ) )
		{
			CreateMainMenu( menuName );
		}
	}

	//===================================================================================================

	void GUIContext::SetActiveDock( const char* label )
	{
		mActiveDock = label;
	}

	//===================================================================================================

	void GUIContext::Finalize( )
	{
		LateInit( );
	} 

	//===================================================================================================

	void GUIContext::SetGUIContextParams( const GUIContextParams& params )
	{
		mParams = params;
	} 

	//===================================================================================================

	void GUIContext::SetUIStyleConfig( const AssetHandle< UIStyleConfig >& style )
	{
		mParams.mUIStyle = style;
	}

	//===================================================================================================

	void GUIContext::ClearContext( )
	{ 
		// Clear docking layout with context
		ImGui::RemoveDockingContext( mContext ); 

		mGuiFuncs.clear();
		mWindows.clear();
		mMainMenuOptions.clear();
		mDockingLayouts.clear(); 
		mMainMenuLayout.clear( ); 
	}

	//===================================================================================================

	void GUIContext::LateInit( )
	{
		// Set current context
		ImGui::SetCurrentContext( GetContext( ) );

		// Run through docking layouts here
    	for (auto& dl : mDockingLayouts)
    	{
    		ImGui::DockWith(dl.mChild, dl.mParent, (ImGui::DockSlotType)(u32)dl.mSlotType, dl.mWeight);
    	}
 
		// Create callbacks for docks
		auto onEnterHorizontalSplitHover = [ & ] ( ) -> void
		{ 
			Window::SetWindowCursor( CursorType::SizeWE );
		};

		auto onEnterVerticalSplitHover = [ & ] ( ) -> void
		{ 
			Window::SetWindowCursor( CursorType::SizeNS );
		};

		auto onExitSplitHover = [ & ] ( ) -> void 
		{ 
			Window::SetWindowCursor( CursorType::Arrow );
		}; 

		if ( mActiveDock )
		{
			ImGui::SetDockActive( mActiveDock );
		}

		// Set these callbacks
		ImGui::SetEventCallback( onEnterHorizontalSplitHover, ImGui::CallBackEventType::OnEnterHorizontalSplitHover );
		ImGui::SetEventCallback( onEnterVerticalSplitHover, ImGui::CallBackEventType::OnEnterVerticalSplitHover );
		ImGui::SetEventCallback( onExitSplitHover, ImGui::CallBackEventType::OnExitSplitHover ); 
	}

	//===================================================================================================

	void GUIContext::RootDock( )
	{ 
		ImGui::SetCurrentContext( mContext );
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
	} 

	void GUIContext::Render( )
	{ 
	    // Make a new ImGui window frame
		ImGui_ImplSdlGL3_NewFrame( mWindow->GetSDLWindow(), GetContext( ) ); 

		// Set current context for ImGui manager and ImGui
		ImGuiManager* manager = EngineSubsystem( ImGuiManager );
		manager->SetContextByWindow( mWindow ); 

		// Set style for context
		manager->LoadStyle( mParams.mUIStyle, this );

		if ( mParams.mUseRootDock )
		{
			RootDock( ); 
		}

		// Display all registered windows
		for (auto& wind : mWindows)
		{
			wind.second( );
		}
	}
	
	//=================================================================================================== 

	s32 GUIContext::MainMenu()
	{
		s32 menuHeight = 0;

		if ( ImGui::BeginMainMenuBar( ) )
		{ 
			for ( auto& menu : mMainMenuLayout )
			{
				if ( HasMainMenu( menu ) )
				{
					if ( ImGui::BeginMenu( menu.c_str() ) )
					{
						for ( auto& sub : mMainMenuOptions[ menu ] )
						{
							sub.second( );
						} 

						ImGui::EndMenu( );
					} 
				}
			} 

			menuHeight = ImGui::GetWindowSize().y;

			ImGui::EndMainMenuBar();
		}

		return menuHeight;

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

	//======================================================================================== 

	Result ImGuiManager::Initialize( )
	{
		return Result::SUCCESS;
	}

	void ImGuiManager::Update( const f32 dT )
	{ 
	} 

	void ImGuiManager::RemoveWindowFromContextMap( SDL_Window* window )
	{
		mImGuiContextMap.erase( window );
	}

	void ImGuiManager::AddWindowToContextMap( SDL_Window* window, ImGuiContext* ctx )
	{
		auto query = mImGuiContextMap.find( window );
		if ( query == mImGuiContextMap.end( ) )
		{
			mImGuiContextMap[ window ] = ctx;
		}
	} 

	void ImGuiManager::SetContextByWindow( Window* window )
	{
		GUIContext* guiCtx = window->GetGUIContext( );
		ImGuiContext* ctx = guiCtx->GetContext( );
		if ( ctx )
		{
			ImGui::SetCurrentContext( ctx );
		}
		mContext = ctx;
	}

	ImGuiContext* ImGuiManager::GetContextByWindow( Window* window )
	{
		if ( mImGuiContextMap.find( window->GetSDLWindow() ) != mImGuiContextMap.end() )
		{
			return mImGuiContextMap[window->GetSDLWindow()];
		}
		return nullptr;
	}

	ImGuiContext* ImGuiManager::Init(Window* window)
	{
		assert( window != nullptr );

		// Cache current context
		ImGuiContext* curCtx = ImGui::GetCurrentContext( );

		// Init window
		ImGuiContext* ctx = ImGui_ImplSdlGL3_Init( window->GetSDLWindow( ) );

		// Init style
		ImGuiStyles();

		// Initialize default windows/menus
		//InitializeDefaults(); 

		// Set imgui context
		AddWindowToContextMap( window->GetSDLWindow(), ctx ); 

		// Reset context
		ImGui::SetCurrentContext( curCtx );

		return ctx;
	} 

	Result ImGuiManager::Shutdown()
	{ 
		// Clear all functions and docks
		mGuiFuncs.clear( );
		mWindows.clear( );
		mMainMenuOptions.clear( );
		mDockingLayouts.clear( ); 

		// Destroy all contexts ( if existing )
		//for ( auto& w : mImGuiContextMap )
		//{ 
		//	if ( w.second )
		//	{
		//		// Set context
		//		ImGui::SetCurrentContext( w.second );

		//		// Destroy device data
		//		ImGui_ImplSdlGL3_InvalidateDeviceObjects( );

		//		// Destroy context
		//		ImGui::DestroyContext( w.second );
		//	}
		//	w.second = nullptr;
		//}

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
			ImGui::Text( "%s", ( "Key: " + std::to_string( iter->first ) ).c_str( ) );\
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
			ImGui::Text( "%s", ( "Key: " + iter->first ).c_str( ) );\
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

		// If you hit a "plus" button, then want to add a new element

		switch ( prop->GetKeyType( ) )
		{
			default: break;

			case MetaPropertyType::F32:
			{ 
				switch ( prop->GetValueType() )
				{
					default: break;
					case MetaPropertyType::U32:		MAP_KEY_PRIMITIVE( f32, u32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::S32:		MAP_KEY_PRIMITIVE( f32, s32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::F32:		MAP_KEY_PRIMITIVE( f32, f32, f32, ImGui::InputFloat, object, prop )	break;
				} 
			} break;

			case MetaPropertyType::S32:
			{ 
				switch ( prop->GetValueType() )
				{
					default: break;
					case MetaPropertyType::U32:		MAP_KEY_PRIMITIVE( s32, u32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::S32:		MAP_KEY_PRIMITIVE( s32, s32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::F32:		MAP_KEY_PRIMITIVE( s32, f32, f32, ImGui::InputFloat, object, prop )	break;
				} 
			} break;

			case MetaPropertyType::U32:
			{ 
				switch ( prop->GetValueType() )
				{
					default: break;
					case MetaPropertyType::U32:		MAP_KEY_PRIMITIVE( u32, u32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::S32:		MAP_KEY_PRIMITIVE( u32, s32, s32, ImGui::InputInt, object, prop )	break;
					case MetaPropertyType::F32:		MAP_KEY_PRIMITIVE( u32, f32, f32, ImGui::InputFloat, object, prop )	break;
				} 
			} break;

			case MetaPropertyType::String:
			{
				// Add a new element into the thing...with a unique key, of course
				if ( ImGui::Button( "+ Add New Element" ) )
				{ 
					switch ( prop->GetValueType() )
					{
						default: break;
						case MetaPropertyType::Object:
						{ 
							// Construct new function should be called for the map. That's all that should happen;
							const MetaClass* cls = prop->GetValueMetaClass();
							if ( cls )
							{
								Object* obj = cls->Construct();
								String key = "New_" + cls->GetName();
								u32 i = 0;
								const MetaPropertyHashMap< String, Object* >* mapProp = prop->Cast< MetaPropertyHashMap< String, Object* > >( );
								while ( true ) {
									if ( !mapProp->KeyExists( object, key ) ) {
										break;
									}
									key = "New_" + cls->GetName() + std::to_string( i++ );
								}
								mapProp->SetValueAt( object, key, obj );
							}
						}
					}
				}

				switch ( prop->GetValueType( ) )
				{
					default: break;
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
							ImGui::Text( "%s", ( "Key: " + iter->first ).c_str( ) );
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
			default: break;
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
				const MetaPropertyArrayBase* arrBase = prop->Cast< MetaPropertyArrayBase >( );
				const MetaProperty* arrPropTypeBase = arrBase->GetProxy( ).mArrayPropertyTypeBase; 

				// If not pointer, then stacked based allocated object
				if ( !arrPropTypeBase->GetTraits( ).IsPointer( ) )
				{
					// Not working for now...
				} 

				else
				{
					const MetaPropertyArray< Object* >* arrayProp = static_cast< const MetaPropertyArray< Object* >* >( prop ); 
					if ( arrayProp )
					{
						for ( usize i = 0; i < arrayProp->GetSize( object ); ++i )
						{
							const Object* arrObj = arrayProp->GetValueAs( object, i );

							if ( !arrObj )
							{
								continue;
							}

							const MetaClass* arrPropCls = arrObj->Class( ); 

							if ( ImGui::TreeNode( Enjon::String( arrPropCls->GetName() + "##" + std::to_string( (u32)(usize)( arrObj ) ) ).c_str( ) ) )
							{
								DebugDumpObject( arrayProp->GetValueAs( object, i ) );
								ImGui::TreePop( );
							}
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
							if ( ImGui::TreeNode( Enjon::String( arrPropCls->GetName( ) + "##" + std::to_string( (u32)(usize)( arrObj.GetID() ) ) ).c_str( ) ) )
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
					for ( u32 i = 0; i < (u32)arrayProp->GetSize( object ); ++i )
					{
						Enjon::AssetHandle<Enjon::Asset> val; 
						arrayProp->GetValueAt( object, i, &val );
						const Enjon::AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
						auto assets = am->GetAssets( assetCls ); 
						if ( ImGui::TreeNode( Enjon::String( std::to_string( i ) + "##" + prop->GetName( ) + std::to_string((u32)(usize)(arrayProp) ) ).c_str( ) ) )
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

							// Crashes here...
							if ( val )
							{
								// ImGuiManager::DebugDumpObject( val.Get( ) );
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

		// Grab draw list 
		ImDrawList* drawList = ImGui::GetWindowDrawList( );

		f32 startCursorX = ImGui::GetCursorPosX( );
		f32 windowWidth = ImGui::GetWindowWidth( );

		// Get whether or not this particular property has an override for this object
		bool hasPropertyOverride = prop->HasOverride( object ); 

		// If property has override, then need to change text color
		ImColor labelColor = hasPropertyOverride ? ImColor( ImGui::GetColorU32( ImGuiCol_SeparatorHovered ) ) : ImColor( ImGui::GetColorU32( ImGuiCol_Text ) ); 

		// Push color for label
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( labelColor ) );

		// Size of button for reverting property
		f32 buttonSize = 10.0f;

		ImVec2 padding = ImVec2( 5.0f, 0.0f );

		// Formatting
		if ( prop->GetType( ) != MetaPropertyType::Transform )
		{
			// Property override button
			if ( hasPropertyOverride )
			{ 
				ImVec2 bA = ImVec2( ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + buttonSize / 3.0f );
				ImVec2 bB = ImVec2( bA.x + buttonSize, bA.y + buttonSize );

				bool buttonHovered = ImGui::IsMouseHoveringRect( bA, bB );
				bool buttonActive = buttonHovered && ImGui::IsMouseDown( 0 );

				ImColor buttonActiveColor = ImColor( ImGui::GetColorU32( ImGuiCol_SeparatorHovered ) );
				buttonActiveColor.Value.w *= 0.8f;
				ImColor buttonHoveredColor = ImColor( ImGui::GetColorU32( ImGuiCol_SeparatorHovered ) );
				ImColor buttonColor = ImColor( ImColor( ImGui::GetColorU32( ImGuiCol_SeparatorHovered ) ) );
				buttonColor.Value.w *= 0.5f;

				drawList->AddRectFilled( bA, bB, buttonActive ? buttonActiveColor : buttonHovered ? buttonHoveredColor : buttonColor, 2.0f );

				if ( buttonActive )
				{
					// Revert property
					ObjectArchiver::RevertProperty( object->ConstCast< Object >(), const_cast< MetaProperty* >( prop ) );
				} 

				// Display the source object's property in new window
				else if ( buttonHovered )
				{
					ImVec2 windowPos = ImGui::GetMousePos( );
					windowPos.x += 10.0f;
					windowPos.y += 10.0f;
					ImGui::SetNextWindowPos( windowPos );
					Vec2 aspect = Vec2( 300.0f, 30.0f );
					ImColor windowBG = ImGui::GetColorU32( ImGuiCol_WindowBg );
					windowBG.Value.x *= 1.8f;
					windowBG.Value.y *= 1.8f;
					windowBG.Value.z *= 1.8f;
					ImGui::SetNextWindowSize( ImVec2( aspect.x, aspect.y ) );
					ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
					ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( windowBG ) );
					ImGui::Begin( "##SourceObjectPropertyView", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar );
					{
						// BG frame for window
						ImVec2 framePaddingSize( 15.0f, 15.0f );
						ImVec2 bgA( ImGui::GetWindowPos().x + framePaddingSize.x, ImGui::GetWindowPos().y + framePaddingSize.y );
						ImVec2 bgB( ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - framePaddingSize.x , ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - framePaddingSize.y );
						drawList->AddRectFilled( bgA, bgB, ImColor( 0.4f, 0.4f, 0.4f, 1.0f ) );

						// Set cursor position for property drawing
						ImGui::SetCursorScreenPos( ImVec2( ImGui::GetCursorScreenPos( ).x, ImGui::GetWindowPos( ).y + ImGui::GetWindowSize( ).y / 2.0f - 10.0f ) );

						// Grab source object
						const Object* sourceObject = const_cast< MetaProperty* >( prop )->GetSourceObject( object );
						if ( sourceObject )
						{
							DebugDumpProperty( sourceObject, prop );
						} 
					}
					ImGui::End( );
					ImGui::PopStyleColor( );
					ImGui::PopStyleColor( );
				}
			}

			ImGui::SetCursorPosX( startCursorX + buttonSize + padding.x );

			ImGui::Text( "%s", name.c_str( ) ); 
			ImGui::SameLine( ); 

			ImGui::SetCursorPosX( windowWidth * 0.4f );
			ImGui::PushItemWidth( windowWidth / 2.0f ); 
		}

		switch ( prop->GetType( ) )
		{
			default: break;

			case MetaPropertyType::Bool:
			{
				bool val = 0;
				cls->GetValue( object, prop, &val );
				if ( ImGui::Checkbox( Utils::format("##%s", name.c_str()).c_str(), &val ) )
				{
					cls->SetValue( object, prop, val );
				}

			} break;

			case MetaPropertyType::U32:
			{
				u32 val = 0;
				cls->GetValue( object, prop, &val );
				MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderInt( Utils::format("##%s", name.c_str()).c_str(), ( s32* )&val, ( s32 )traits.GetUIMin( ), ( s32 )traits.GetUIMax( ) ) )
					{
						cls->SetValue( object, prop, ( u32 )val );
					}
				}
				else
				{
					if ( ImGui::DragInt( Utils::format("##%s", name.c_str()).c_str(), ( s32* )&val ) )
					{
						cls->SetValue( object, prop, ( u32 )val );
					}
				}
			} break;

			case MetaPropertyType::S32:
			{
				s32 val = 0;
				cls->GetValue( object, prop, &val );
				MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderInt( Utils::format("##%s", name.c_str()).c_str( ), ( s32* )&val, ( s32 )traits.GetUIMin( ), ( s32 )traits.GetUIMax( ) ) )
					{
						cls->SetValue( object, prop, ( s32 )val );
					}
				}
				else
				{
					if ( ImGui::DragInt( Utils::format("##%s", name.c_str()).c_str( ), ( s32* )&val ) )
					{
						cls->SetValue( object, prop, ( s32 )val ); 
					}
				}
			} break;

			case MetaPropertyType::F32:
			{
				float val = 0.0f;
				cls->GetValue( object, prop, &val );
				MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderFloat( Utils::format("##%s", name.c_str()).c_str(), &val, traits.GetUIMin( ), traits.GetUIMax( ) ) )
					{
						cls->SetValue( object, prop, val );
					}
				}
				else
				{
					if ( ImGui::DragFloat( Utils::format("##%s", name.c_str()).c_str(), &val ) )
					{
						cls->SetValue( object, prop, val );
					}
				}
			} break;

			case MetaPropertyType::Vec2:
			{
				Vec2 val;
				cls->GetValue( object, prop, &val );
				MetaPropertyTraits traits = prop->GetTraits( );
				f32 col[ 2 ] = { val.x, val.y };
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderFloat2( Utils::format("##%s", name.c_str()).c_str(), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						cls->SetValue( object, prop, val );
					}
				}
				else
				{
					if ( ImGui::DragFloat2( Utils::format("##%s", name.c_str()).c_str(), col ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						cls->SetValue( object, prop, val );
					}
				}
			} break;

			case MetaPropertyType::Vec3:
			{
				Vec3 val;
				cls->GetValue( object, prop, &val );
				f32 col[ 3 ] = { val.x, val.y, val.z };
				MetaPropertyTraits traits = prop->GetTraits( );
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

			case MetaPropertyType::iVec3:
			{
				iVec3 val;
				cls->GetValue( object, prop, &val );
				s32 col[ 3 ] = { val.x, val.y, val.z };
				MetaPropertyTraits traits = prop->GetTraits( );
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

			case MetaPropertyType::Vec4:
			{
				Vec4 val;
				cls->GetValue( object, prop, &val );
				f32 col[ 4 ] = { val.x, val.y, val.z, val.w };
				MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderFloat4( Utils::format("##%s", name.c_str()).c_str(), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
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
					if ( ImGui::DragFloat4( Utils::format("##%s", name.c_str()).c_str(), col ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						val.z = col[ 2 ];
						val.w = col[ 3 ];
						cls->SetValue( object, prop, val );
					}
				}
			} break;

			case MetaPropertyType::Quat:
			{
				Quaternion val;
				cls->GetValue( object, prop, &val );
				f32 col[ 4 ] = { val.x, val.y, val.z, val.w };
				MetaPropertyTraits traits = prop->GetTraits( );
				// Make a change to a file in the engine.
				// Watch how fast the reflection generation occurs.
				// Already done :)
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::SliderFloat4( Utils::format("##%s", name.c_str()).c_str(), col, traits.GetUIMin( ), traits.GetUIMax( ) ) )
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
					if ( ImGui::DragFloat4( Utils::format("##%s", name.c_str()).c_str(), col ) )
					{
						val.x = col[ 0 ];
						val.y = col[ 1 ];
						val.z = col[ 2 ];
						val.w = col[ 3 ];
						cls->SetValue( object, prop, val );
					}
				} 
			} break;

			case MetaPropertyType::ColorRGBA32:
			{
				ColorRGBA32 val;
				cls->GetValue( object, prop, &val );
				f32 col[ 4 ] = { val.r, val.g, val.b, val.a };
				MetaPropertyTraits traits = prop->GetTraits( );
				if ( traits.UseSlider( ) )
				{
					if ( ImGui::ColorEdit4( Utils::format( "##%s", name.c_str() ).c_str( ), col ) )
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
					if ( ImGui::ColorEdit4( Utils::format( "##%s", name.c_str() ).c_str( ), col ) )
					{ 
						val.r = col[ 0 ];
						val.g = col[ 1 ];
						val.b = col[ 2 ];
						val.a = col[ 3 ];
						cls->SetValue( object, prop, val );
					}
				}
			} break;
				
			case MetaPropertyType::String:
			{
				String val;
				cls->GetValue( object, prop, &val );
				char buffer[ 256 ];
				strncpy( buffer, &val[0], 256 );
				if ( ImGui::InputText( Utils::format("##%s", name.c_str()).c_str( ), buffer, 256 ) )
				{
					// Reset string
					cls->SetValue( object, prop, String( buffer ) ); 
				}
			} break;
				
			case MetaPropertyType::UUID:
			{
				UUID val;
				cls->GetValue( object, prop, &val );
				String str = val.ToString( );
				ImGui::Text( "%s", str.c_str( ) );
			} break;

			// Type is transform
			case MetaPropertyType::Transform:
			{
				InspectObject( cls->GetValueAs< Transform >( object, prop ) );
			} break;

			// Enum type
			case MetaPropertyType::Enum:
			{
				// Property is enum prop, so need to convert it
				const MetaPropertyEnum* enumProp = prop->Cast< MetaPropertyEnum >( ); 

				s32 enumInt = *cls->GetValueAs<s32>( object, prop ); 

				Utils::TempBuffer buffer = Utils::TransientBuffer( "##%s_enum_props_%d_%s", enumProp->GetEnumName().c_str(), (usize)(intptr_t)(object), prop->GetName().c_str() );
				if ( ImGui::BeginCombo( buffer.buffer, enumProp->GetEnumName().c_str() ) )
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
			
			// AssetHandle type
			case MetaPropertyType::AssetHandle:
			{
				// Property is of type MetaPropertyAssetHandle
				const MetaPropertyTemplateBase* base = prop->Cast< MetaPropertyTemplateBase >( );
				const MetaClass* assetCls = base->GetClassOfTemplatedArgument( );

				if ( assetCls )
				{
					AssetHandle<Enjon::Asset> val;
					cls->GetValue( object, prop, &val );
					AssetManager* am = EngineSubsystem( AssetManager );
					auto assets = am->GetAssets( assetCls );
					{
						if ( assets )
						{
							static ImGuiTextFilter filter;
							static String filterString = "";
							String label = val ? val->GetName( ) : assetCls->GetName( );
							if ( ImGui::BeginCombo( Utils::format( "##%s", prop->GetName().c_str() ).c_str( ), label.c_str( ) ) )
							{
								// For each record in assets
								for ( auto& a : *assets )
								{
									char buffer[ 256 ];
									strncpy( buffer, filterString.c_str( ), 256 );
									if ( filter.PassFilter( buffer ) )
									{
										if ( ImGui::Selectable( a.second.GetAssetName( ).c_str( ) ) )
										{
											val.Set( const_cast< Asset* > ( a.second.GetAsset( ) ) );
											cls->SetValue( object, prop, val );
										}
									}
								}
								ImGui::EndCombo( );
							}
						}
						if ( val )
						{
							//ImGuiManager::DebugDumpObject( val.Get( ) );
						}
					}
				}
			} break;
		}

		// Pop color for label
		ImGui::PopStyleColor( );
	} 

	ImGuiContext* ImGuiManager::GetContext( )
	{
		return mContext;
	}

	void ImGuiManager::InspectObject( const Object* object )
	{ 
		// Need to make sure that the correct context is set? 
		ImGui::SetCurrentContext( mContext );
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
				default: break;

				// Primitive types
				case Enjon::MetaPropertyType::U32: 
				case Enjon::MetaPropertyType::S32: 
				case Enjon::MetaPropertyType::F32: 
				case Enjon::MetaPropertyType::Vec2: 
				case Enjon::MetaPropertyType::iVec3: 
				case Enjon::MetaPropertyType::Vec3: 
				case Enjon::MetaPropertyType::Vec4:
				case Enjon::MetaPropertyType::Quat:
				case Enjon::MetaPropertyType::ColorRGBA32:
				case Enjon::MetaPropertyType::String:
				case Enjon::MetaPropertyType::UUID: 
				case Enjon::MetaPropertyType::Transform:
				case Enjon::MetaPropertyType::Bool:
				case Enjon::MetaPropertyType::Enum:
				case Enjon::MetaPropertyType::AssetHandle:
				{
					DebugDumpProperty( object, prop );
				} break; 

				// Array type
				case Enjon::MetaPropertyType::Array:
				{
					if ( ImGui::TreeNode( Enjon::String( prop->GetName( ) + "##" + std::to_string( (u32)(usize)object ) ).c_str( ) ) )
					{
						const MetaPropertyArrayBase* arrayProp = prop->Cast< MetaPropertyArrayBase >( );
						DebugDumpArrayProperty( object, arrayProp );
						ImGui::TreePop( );
					}
				} break;

				case Enjon::MetaPropertyType::HashMap: 
				{ 
					// Hate the way these are displayed now. Need a better way of handling this...
					if ( ImGui::TreeNode( Enjon::String( prop->GetName() + "##" + std::to_string( (u32)(usize)object ) ).c_str() ) )
					{
						const MetaPropertyHashMapBase* mapProp = prop->Cast< MetaPropertyHashMapBase >();
						DebugDumpHashMapProperty( object, mapProp );
						ImGui::TreePop( );
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
	void ImGuiManager::Render(Window* window)
	{ 
		// Render gui context
		window->GetGUIContext( )->Render( );
		ImGui_ImplSdlGL3_UpdateViewports(window->GetGUIContext()->GetContext());
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

	void ImGuiManager::LateInit( Window* window )
	{
		for ( auto& w : EngineSubsystem( GraphicsSubsystem )->GetWindows( ) )
		{
			Render( w ); 
		}

		for ( auto& w : EngineSubsystem( GraphicsSubsystem )->GetWindows( ) )
		{
			w->GetGUIContext( )->LateInit( );
		}

		// Run through docking layouts here
    	for (auto& dl : mDockingLayouts)
    	{
    		ImGui::DockWith(dl.mChild, dl.mParent, (ImGui::DockSlotType)(u32)dl.mSlotType, dl.mWeight);
    	}
 
		// Create callbacks for docks
		auto onEnterHorizontalSplitHover = [ & ] ( ) -> void
		{ 
			Window::SetWindowCursor( CursorType::SizeWE );
		};

		auto onEnterVerticalSplitHover = [ & ] ( ) -> void
		{ 
			Window::SetWindowCursor( CursorType::SizeNS );
		};

		auto onExitSplitHover = [ & ] ( ) -> void 
		{ 
			Window::SetWindowCursor( CursorType::Arrow );
		}; 

		// Set these callbacks
		ImGui::SetEventCallback( onEnterHorizontalSplitHover, ImGui::CallBackEventType::OnEnterHorizontalSplitHover );
		ImGui::SetEventCallback( onEnterVerticalSplitHover, ImGui::CallBackEventType::OnEnterVerticalSplitHover );
		ImGui::SetEventCallback( onExitSplitHover, ImGui::CallBackEventType::OnExitSplitHover );
 
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

#define Vec2ToImVec2( v )\
	ImVec2( (v).x, (v).y )

#define Col32ToImVec4( c )\
	ImVec4( (c).r, (c).g, (c).b, (c).a )

	void ImGuiManager::LoadStyle( const AssetHandle< UIStyleConfig >& handle, GUIContext* ctx )
	{ 
		const UIStyleConfig* asset = handle.Get();
		if ( !asset ) {
			return;
		}

		// Load the styles from the asset
		// Here's the issue - need a way to set multiple font sizes for any given font and then rebuild a texture upon request
		// Cannot rebuild the texture DURING a frame, therefore it will be deferred to the next. Damn. 
		// Load a style for a particular context
		ImGuiContext* prevContext = ImGui::GetCurrentContext(); 
		if ( ctx ) {
			ImGui::SetCurrentContext( ctx->GetContext( ) );
		}

		// Set styles
		{	
			// Grab reference to style
			ImGuiStyle& style = ImGui::GetStyle(); 
			ImGuiIO& io = ImGui::GetIO(); 

			style.WindowTitleAlign 		= Vec2ToImVec2( asset->WindowTitleAlign );
			style.ButtonTextAlign 		= Vec2ToImVec2( asset->ButtonTextAlign ); 
			style.WindowPadding			= Vec2ToImVec2( asset->WindowPadding );
			style.WindowRounding		= asset->WindowRounding;
			style.FramePadding			= Vec2ToImVec2( asset->FramePadding );
			style.FrameRounding			= asset->FrameRounding;
			style.ItemSpacing			= Vec2ToImVec2( asset->ItemSpacing );
			style.ItemInnerSpacing		= Vec2ToImVec2( asset->ItemInnerSpacing );
			style.IndentSpacing			= asset->IndentSpacing;
			style.ScrollbarSize			= asset->ScrollbarSize;
			style.ScrollbarRounding		= asset->ScrollbarRounding;
			style.GrabMinSize			= asset->GrabMinSize;
			style.GrabRounding			= asset->GrabRounding;
			style.Alpha					= Math::Clamp( asset->GrabRounding, 0.f, 1.f );
			style.FrameBorderSize		= asset->FrameBorderSize;
			style.WindowBorderSize		= asset->WindowBorderSize; 

			ImVec4* colors = ImGui::GetStyle( ).Colors;
			colors[ ImGuiCol_Text ] = Col32ToImVec4( asset->Col_Text );
			colors[ ImGuiCol_TextDisabled ] = Col32ToImVec4( asset->Col_TextDisabled );
			colors[ ImGuiCol_WindowBg ] = Col32ToImVec4( asset->Col_WindowBg );
			colors[ ImGuiCol_ChildBg ] = Col32ToImVec4( asset->Col_ChildBg );
			colors[ ImGuiCol_PopupBg ] = Col32ToImVec4( asset->Col_PopupBg );
			colors[ ImGuiCol_Border ] = Col32ToImVec4( asset->Col_Border );
			colors[ ImGuiCol_BorderShadow ] = Col32ToImVec4( asset->Col_BorderShadow );
			colors[ ImGuiCol_FrameBg ] = Col32ToImVec4( asset->Col_FrameBg );
			colors[ ImGuiCol_FrameBgHovered ] = Col32ToImVec4( asset->Col_FrameBgHovered );
			colors[ ImGuiCol_FrameBgActive ] = Col32ToImVec4( asset->Col_FrameBgActive );
			colors[ ImGuiCol_TitleBg ] = Col32ToImVec4( asset->Col_TitleBg );
			colors[ ImGuiCol_TitleBgActive ] = Col32ToImVec4( asset->Col_TitleBgActive );
			colors[ ImGuiCol_TitleBgCollapsed ] = Col32ToImVec4( asset->Col_TitleBgCollapsed );
			colors[ ImGuiCol_MenuBarBg ] = Col32ToImVec4( asset->Col_MenuBarBg );
			colors[ ImGuiCol_ScrollbarBg ] = Col32ToImVec4( asset->Col_ScrollbarBg );
			colors[ ImGuiCol_ScrollbarGrab ] = Col32ToImVec4( asset->Col_ScrollbarGrab );
			colors[ ImGuiCol_ScrollbarGrabHovered ] = Col32ToImVec4( asset->Col_ScrollbarGrabHovered );
			colors[ ImGuiCol_ScrollbarGrabActive ] = Col32ToImVec4( asset->Col_ScrollbarGrabActive );
			colors[ ImGuiCol_CheckMark ] = Col32ToImVec4( asset->Col_CheckMark );
			colors[ ImGuiCol_SliderGrab ] = Col32ToImVec4( asset->Col_SliderGrab );
			colors[ ImGuiCol_SliderGrabActive ] = Col32ToImVec4( asset->Col_SliderGrabActive );
			colors[ ImGuiCol_Button ] = Col32ToImVec4( asset->Col_Button );
			colors[ ImGuiCol_ButtonHovered ] = Col32ToImVec4( asset->Col_ButtonHovered );
			colors[ ImGuiCol_ButtonActive ] = Col32ToImVec4( asset->Col_ButtonActive );
			colors[ ImGuiCol_Header ] = Col32ToImVec4( asset->Col_Header );
			colors[ ImGuiCol_HeaderHovered ] = Col32ToImVec4( asset->Col_HeaderHovered );
			colors[ ImGuiCol_HeaderActive ] = Col32ToImVec4( asset->Col_HeaderActive );
			colors[ ImGuiCol_Separator ] = Col32ToImVec4( asset->Col_Separator );
			colors[ ImGuiCol_SeparatorHovered ] = Col32ToImVec4( asset->Col_SeparatorHovered );
			colors[ ImGuiCol_SeparatorActive ] = Col32ToImVec4( asset->Col_SeparatorActive );
			colors[ ImGuiCol_ResizeGrip ] = Col32ToImVec4( asset->Col_ResizeGrip );
			colors[ ImGuiCol_ResizeGripHovered ] = Col32ToImVec4( asset->Col_ResizeGripHovered );
			colors[ ImGuiCol_ResizeGripActive ] = Col32ToImVec4( asset->Col_ResizeGripActive );
			colors[ ImGuiCol_CloseButton ] = Col32ToImVec4( asset->Col_CloseButton );
			colors[ ImGuiCol_CloseButtonHovered ] = Col32ToImVec4( asset->Col_CloseButtonHovered );
			colors[ ImGuiCol_CloseButtonActive ] = Col32ToImVec4( asset->Col_CloseButtonActive );
			colors[ ImGuiCol_PlotLines ] = Col32ToImVec4( asset->Col_PlotLines );
			colors[ ImGuiCol_PlotLinesHovered ] = Col32ToImVec4( asset->Col_PlotLinesHovered );
			colors[ ImGuiCol_PlotHistogram ] = Col32ToImVec4( asset->Col_PlotHistogram );
			colors[ ImGuiCol_PlotHistogramHovered ] = Col32ToImVec4( asset->Col_PlotHistogramHovered );
			colors[ ImGuiCol_TextSelectedBg ] = Col32ToImVec4( asset->Col_TextSelectedBg );
			colors[ ImGuiCol_ModalWindowDarkening ] = Col32ToImVec4( asset->Col_ModalWindowDarkening );
			colors[ ImGuiCol_DragDropTarget ] = Col32ToImVec4( asset->Col_DragDropTarget );
			colors[ ImGuiCol_NavHighlight ] = Col32ToImVec4( asset->Col_NavHighlight );
			colors[ ImGuiCol_NavWindowingHighlight ] = Col32ToImVec4( asset->Col_NavWindowingHighlight );
			colors[ ImGuiCol_SelectableHovered ] = Col32ToImVec4( asset->Col_SelectableHovered );
			colors[ ImGuiCol_SelectableActive ] = Col32ToImVec4( asset->Col_SelectableActive );
			colors[ ImGuiCol_Selectable ] = Col32ToImVec4( asset->Col_Selectable );
			colors[ ImGuiCol_ComboBox ] = Col32ToImVec4( asset->Col_ComboBox );
			colors[ ImGuiCol_ComboBoxHovered ] = Col32ToImVec4( asset->Col_ComboBoxHovered );
			colors[ ImGuiCol_ListSelection ] = Col32ToImVec4( asset->Col_ListSelection );
			colors[ ImGuiCol_ListSelectionHovered ] = Col32ToImVec4( asset->Col_ListSelectionHovered );
			colors[ ImGuiCol_ListSelectionActive ] = Col32ToImVec4( asset->Col_ListSelectionActive );
			colors[ ImGuiCol_ListSelectionRenamed ] = Col32ToImVec4( asset->Col_ListSelectionRenamed ); 
		} 
		ImGui::SetCurrentContext( prevContext );
	}

	//============================================================================================ 

	void ImGuiManager::AddFont( const String& filePath, const u32& size, GUIContext* ctx, const char* fontName )
	{ 
		// Cache previous context, set context
		ImGuiContext* prevContext = ImGui::GetCurrentContext(); 
		ImGui::SetCurrentContext( ctx->GetContext() );

		String rootPath = Engine::GetInstance()->GetConfig( ).GetRoot( );
		String fp = rootPath + "/Assets/Fonts/";
		ImGuiIO& io = ImGui::GetIO(); 
		ImFontConfig fontCfg;
	    fontCfg.FontDataOwnedByAtlas = true;
	    //fontCfg.OversampleH = 7;
	    //fontCfg.OversampleV = 7; 
	    int fs = 1; 

		io.Fonts->Clear();
		mFonts.clear();
			mFonts["WeblySleek_10"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 10 * fs, &fontCfg );
			mFonts["WeblySleek_14"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 14 * fs, &fontCfg );
			mFonts["WeblySleek_16"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 16 * fs, &fontCfg );
			mFonts["WeblySleek_20"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 20 * fs, &fontCfg );
			mFonts["WeblySleek_24"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 24 * fs, &fontCfg );
			mFonts["WeblySleek_32"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 32 * fs, &fontCfg );
			mFonts["WeblySleek_100"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 100 * fs, &fontCfg );
			mFonts["Roboto-MediumItalic_14"] = io.Fonts->AddFontFromFileTTF( (fp + "Roboto/Roboto-MediumItalic.ttf").c_str(), 14 * fs, &fontCfg );
			mFonts["Roboto-MediumItalic_12"] = io.Fonts->AddFontFromFileTTF( (fp + "Roboto/Roboto-MediumItalic.ttf").c_str(), 12 * fs, &fontCfg );
			char buffer[1024];
			snprintf( buffer, 1024, "%s_%d", fontName, (s32)size );
			mFonts[ buffer ] = io.Fonts->AddFontFromFileTTF( filePath.c_str(), (f32)size, &fontCfg );
		io.Fonts->Build();

		// Recreate font texture
		ImGui_ImplSdlGL3_CreateFontsTexture( ctx->GetContext() );

		ImGuiStyle& style = ImGui::GetStyle(); 

		// Set default font
		io.FontDefault = mFonts[ "WeblySleek_16" ];

		// Restore context
		ImGui::SetCurrentContext( prevContext );
	}

	//============================================================================================

	void ImGuiManager::ImGuiStyles()
	{
		String rootPath = Engine::GetInstance()->GetConfig( ).GetRoot( );
		String fp = rootPath + "/Assets/Fonts/";

		ImGuiIO& io = ImGui::GetIO();
		ImFontConfig fontCfg;
	    fontCfg.FontDataOwnedByAtlas = true;
	    fontCfg.OversampleH = 6;
	    fontCfg.OversampleV = 6;
	    // fontCfg.RasterizerMultiply = 1.5f; 

	    // Font scale
	    int fs = 1; 

		io.Fonts->Clear();

		mFonts["WeblySleek_10"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 10 * fs, &fontCfg );
		mFonts["WeblySleek_14"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 14 * fs, &fontCfg );
		mFonts["WeblySleek_16"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 16 * fs, &fontCfg );
		mFonts["WeblySleek_20"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 20 * fs, &fontCfg );
		mFonts["WeblySleek_24"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 24 * fs, &fontCfg );
		mFonts["WeblySleek_32"] = io.Fonts->AddFontFromFileTTF( (fp + "WeblySleek/weblysleekuisb.ttf").c_str(), 32 * fs, &fontCfg );
		mFonts["Roboto-MediumItalic_14"] = io.Fonts->AddFontFromFileTTF( (fp + "Roboto/Roboto-MediumItalic.ttf").c_str(), 14 * fs, &fontCfg );
		mFonts["Roboto-MediumItalic_12"] = io.Fonts->AddFontFromFileTTF( (fp + "Roboto/Roboto-MediumItalic.ttf").c_str(), 12 * fs, &fontCfg ); 
		io.Fonts->Build();

		// io.DisplayFramebufferScale = { 8, 8 };
		// io.FontGlobalScale = 0.25f;

		// Grab reference to style
		ImGuiStyle& style = ImGui::GetStyle(); 

		// Set default font
		io.FontDefault = mFonts[ "WeblySleek_16" ];

		style.WindowTitleAlign 		= ImVec2(0.5f, 0.41f);
		style.ButtonTextAlign 		= ImVec2(0.5f, 0.5f); 
		style.WindowPadding			= ImVec2(10, 6);
		style.WindowRounding		= 0.0f;
		style.FramePadding			= ImVec2(6, 3);
		style.FrameRounding			= 3.0f;
		style.ItemSpacing			= ImVec2(8, 2);
		style.ItemInnerSpacing		= ImVec2(2, 3);
		style.IndentSpacing			= 20.0f;
		style.ScrollbarSize			= 14.0f;
		style.ScrollbarRounding		= 0.0f;
		style.GrabMinSize			= 5.0f;
		style.GrabRounding			= 2.0f;
		style.Alpha					= 1.0f;
		style.FrameBorderSize		= 1.0f;
		style.WindowBorderSize		= 1.0f; 

		ImVec4* colors = ImGui::GetStyle( ).Colors;
		colors[ ImGuiCol_Text ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
		colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.50f, 0.50f, 0.50f, 0.57f );
		colors[ ImGuiCol_WindowBg ] = ImVec4( 0.13f, 0.13f, 0.13f, 1.00f );
		colors[ ImGuiCol_ChildBg ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
		colors[ ImGuiCol_PopupBg ] = ImVec4( 0.08f, 0.08f, 0.08f, 1.00f );
		colors[ ImGuiCol_Border ] = ImVec4( 0.08f, 0.08f, 0.08f, 0.28f );
		colors[ ImGuiCol_BorderShadow ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.05f );
		colors[ ImGuiCol_FrameBg ] = ImVec4( 0.03f, 0.03f, 0.03f, 0.39f );
		colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );
		colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.16f, 0.16f, 0.16f, 1.00f );
		colors[ ImGuiCol_TitleBg ] = ImVec4( 0.04f, 0.04f, 0.04f, 1.00f );
		colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.15f, 0.53f, 0.61f, 1.00f );
		colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
		colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.17f, 0.17f, 0.17f, 1.00f );
		colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
		colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.31f, 0.31f, 0.31f, 1.00f );
		colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
		colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
		colors[ ImGuiCol_CheckMark ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
		colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.07f, 0.39f, 0.71f, 1.00f );
		colors[ ImGuiCol_Button ] = ImVec4( 0.24f, 0.24f, 0.24f, 1.00f );
		colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.27f, 0.27f, 0.27f, 1.00f );
		colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.17f, 0.17f, 0.17f, 1.00f );
		colors[ ImGuiCol_Header ] = ImVec4( 0.20f, 0.20f, 0.20f, 0.31f );
		colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_Separator ] = ImVec4( 0.29f, 0.29f, 0.29f, 0.50f );
		colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.13f, 0.77f, 1.00f, 1.00f );
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
		colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.29f, 0.43f, 0.58f, 1.00f );
		colors[ ImGuiCol_ModalWindowDarkening ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.80f );
		colors[ ImGuiCol_DragDropTarget ] = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
		colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
		colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
		colors[ ImGuiCol_SelectableHovered ] = ImVec4( 0.14f, 0.23f, 0.32f, 1.00f );
		colors[ ImGuiCol_SelectableActive ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_Selectable ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_ComboBox ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
		colors[ ImGuiCol_ComboBoxHovered ] = ImVec4( 0.16f, 0.16f, 0.16f, 1.00f );
		colors[ ImGuiCol_ListSelection ] = ImVec4( 0.06f, 0.48f, 0.89f, 1.00f );
		colors[ ImGuiCol_ListSelectionHovered ] = ImVec4( 0.88f, 0.40f, 0.18f, 1.00f );
		colors[ ImGuiCol_ListSelectionActive ] = ImVec4( 0.88f, 0.40f, 0.18f, 1.00f );
		colors[ ImGuiCol_ListSelectionRenamed ] = ImVec4( 0.88f, 0.40f, 0.18f, 1.00f );
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
		BindContext( );
		ImGui::Text( "%s", text.c_str( ) );
	}

	//=====================================================================

	void ImGuiManager::SameLine( )
	{
		ImGui::SameLine( );
	}

	//=====================================================================

	bool ImGuiManager::CheckBox( const String& text, bool* option )
	{
		return ImGui::Checkbox( text.c_str( ), option );
	}

	//=====================================================================

	bool ImGuiManager::Button( const String& text )
	{
		return ImGui::Button( text.c_str( ) );
	}

	//=====================================================================

	bool ImGuiManager::Selectable( const String& text )
	{
		return ImGui::Selectable( text.c_str( ) );
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

	bool ImGuiManager::InputText( const char* label, char* buffer, usize buffSize )
	{
		BindContext( );
		return ImGui::InputText( label, buffer, buffSize );
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
 
		// Math::Clamp menu position to inside of window
		if ( popupMaxPos.x > dispSize.x )
		{ 
			position.x = dispSize.x - mSize.x;
		} 

		// Math::Clamp to being inside of the window and not negative
		position.x = Math::Max( position.x, 0.0f ); 

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
			bgAlpha =  Math::Clamp( mFadeTimer / 1.0f, 0.0f, 1.0f ); 
		}
		else
		{
			// Increment fade timer by delta time
			mFadeOutTimer -= mFadeOutSpeed * Engine::GetInstance( )->GetWorldTime( ).GetDeltaTime( ); 
			mFadeOutTimer = Math::Clamp( mFadeOutTimer, 0.0f, 1.0f ); 
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
		const char* label = Utils::format( "%s##%zu", mLabel.c_str(), (u32)(usize)this ).c_str();
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
			ImGui::MenuItem( Utils::format( "%s##options", mLabel.c_str() ).c_str( ), NULL, &mEnabled );
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
	}

	//===================================================================== 
}