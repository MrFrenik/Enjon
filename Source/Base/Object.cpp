
#include "Base/Object.h"
#include "Serialize/ByteBuffer.h"
#include "Serialize/ObjectArchiver.h"
#include "Serialize/AssetArchiver.h"
#include "Entity/EntityManager.h"
#include "Engine.h"

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
} 














