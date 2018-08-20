
#include "Base/Object.h"
#include "Serialize/ByteBuffer.h"
#include "Serialize/ObjectArchiver.h"
#include "Serialize/AssetArchiver.h"
#include "Entity/EntityManager.h"
#include "ImGui/ImGuiManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"

namespace Enjon
{ 
	//=========================================================================

	Result Object::SerializeData( ByteBuffer* buffer ) const
	{ 
		return Result::INCOMPLETE;
	}

	//=========================================================================

	Result Object::DeserializeData( ByteBuffer* buffer )
	{
		return Result::INCOMPLETE; 
	}

	//=========================================================================

	Result Object::DeserializeLateInit( )
	{
		return Result::INCOMPLETE;
	}

	//=========================================================================

	bool MetaClass::InstanceOf( const MetaClass* cls ) const
	{
		return ( cls && cls == this );
	}

	//=========================================================================

	void MetaProperty::AddOverride( const Object* obj, const Object* source )
	{
		if ( mPropertyOverrides.find( obj ) == mPropertyOverrides.end( ) )
		{
			mPropertyOverrides.insert( obj ); 
			mPropertyOverrideSourceMap[ obj ] = source;
		}
	}

	//=========================================================================

	const Object* MetaProperty::GetSourceObject( const Object* key )
	{
		if ( mPropertyOverrideSourceMap.find( key ) != mPropertyOverrideSourceMap.end() )
		{
			return mPropertyOverrideSourceMap[ key ];
		}

		return nullptr;
	}

	//=========================================================================

	void MetaProperty::RemoveOverride( const Object* obj )
	{ 
		mPropertyOverrides.erase( obj );
		mPropertyOverrideSourceMap.erase( obj );
	}

	//=========================================================================

	bool MetaProperty::HasOverride( const Object* obj ) const
	{
		return ( mPropertyOverrides.find( obj ) != mPropertyOverrides.end( ) );
	}

	//=========================================================================

	void MetaClassRegistry::UnregisterMetaClass( const MetaClass* cls )
	{ 
		// If available, then return
		if ( HasMetaClass( cls->GetName() ) )
		{
			// If component, then must unregister the component from the entity manager first
			switch ( cls->GetMetaClassType( ) )
			{
				case MetaClassType::Component:
				{
					EntityManager* entities = Engine::GetInstance( )->GetSubsystem( Object::GetClass< EntityManager >( ) )->ConstCast< EntityManager >( );
					if ( entities )
					{
						entities->UnregisterComponent( cls );
					}

				} break;
				case MetaClassType::Object:
				case MetaClassType::Application:
				{ 
					// Do nothing for now
				} break;
			}

			// Delete metaclass from registry
			u32 id = cls->GetTypeId( );
			MetaClass* cls = mRegistry[ id ];
			mRegistry.erase( id );
			mRegistryByClassName.erase( cls->GetName( ) );
			delete cls;
			cls = nullptr;
		} 
	} 

	//======================================================================================

	void MetaClassRegistry::RegisterMetaClassLate( const MetaClass* cls )
	{
		switch ( cls->GetMetaClassType( ) )
		{
			case MetaClassType::Component:
			{
				EntityManager* entities = Engine::GetInstance( )->GetSubsystem( Object::GetClass< EntityManager >( ) )->ConstCast< EntityManager >( );
				if ( entities )
				{
					if ( !entities->ComponentBaseExists( cls->GetTypeId( ) ) )
					{
						entities->RegisterComponent( cls ); 
					}
				} 
			} break;
		} 
	}

	//======================================================================================

	const MetaClass* Object::GetClass( const u32& typeId )
	{
		MetaClassRegistry* mr = const_cast<MetaClassRegistry*> ( Engine::GetInstance( )->GetMetaClassRegistry( ) );
		return mr->GetClassById( typeId ); 
	}

	//======================================================================================

	const MetaClass* MetaClassRegistry::GetClassById( const u32& typeId )
	{
		if ( HasMetaClass( typeId ) )
		{
			return mRegistry[typeId];
		}

		return nullptr;
	}

	//======================================================================================

	Result Object::OnEditorUI( )
	{
		return Result::INCOMPLETE;
	}

	//======================================================================================

	Result Object::MergeWith( Object* other, MergeType mergeType )
	{
		return Result::INCOMPLETE;
	}

	//====================================================================================== 

	Result Object::HasPropertyOverrides( bool& result ) const
	{
		return Result::INCOMPLETE;
	}

	//====================================================================================== 

	Result Object::RecordPropertyOverrides( Object* source )
	{
		return Result::INCOMPLETE;
	}

	//====================================================================================== 

	Result Object::ClearAllPropertyOverrides( )
	{
		// Probably have some default behavior here...
		return Result::INCOMPLETE;
	}

	//====================================================================================== 
} 














