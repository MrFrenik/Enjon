// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: EntityArchiver.cpp

#include "Serialize/EntityArchiver.h"
#include "SubsystemCatalog.h"

#include "Graphics/GraphicsSubsystem.h"
#include "Entity/Components/GraphicsComponent.h"
#include "Engine.h"
#include "SubsystemCatalog.h"

namespace Enjon
{
	//=========================================================================================

	Result EntityArchiver::Serialize( const EntityHandle& entity )
	{
		// Make call to static function and return result
		return Serialize( entity, &mBuffer );
	} 

	//==========================================================================

	Result EntityArchiver::Serialize( const EntityHandle& entity, ByteBuffer* buffer )
	{ 
		if ( !buffer )
		{
			return Result::FAILURE;
		}

		// Write that entity was null for deserializing later on
		if ( !entity.Get( ) )
		{
			buffer->Write< u32 >( 0 );
			return Result::FAILURE;
		}

		// Write that entity was alive and serialized
		buffer->Write< u32 >( 1 );

		//==========================================================================
		// Local Transform
		//========================================================================== 

		Transform local = entity.Get( )->GetLocalTransform( );

		// Write out position
		buffer->Write< f32 >( local.Position.x );
		buffer->Write< f32 >( local.Position.y );
		buffer->Write< f32 >( local.Position.z );

		// Write out rotation
		buffer->Write< f32 >( local.Rotation.x );
		buffer->Write< f32 >( local.Rotation.y );
		buffer->Write< f32 >( local.Rotation.z );
		buffer->Write< f32 >( local.Rotation.w );

		// Write out scale
		buffer->Write< f32 >( local.Scale.x );
		buffer->Write< f32 >( local.Scale.y );
		buffer->Write< f32 >( local.Scale.z );

		//==========================================================================
		// Components
		//========================================================================== 

		const Vector< Component* >& comps = entity.Get( )->GetComponents( );

		// Write out number of comps
		buffer->Write< u32 >( ( u32 )comps.size( ) );

		// Write out component data
		for ( auto& c : comps )
		{
			// Get component's meta class
			const MetaClass* compCls = c->Class( );

			// Write out component class
			buffer->Write< String >( compCls->GetName( ) );

			// Serialize component data
			Result res = c->SerializeData( buffer );
			if ( res == Result::INCOMPLETE )
			{
				SerializeObjectDataDefault( c, compCls, buffer );
			}
		}

		//================================================================================
		// Entity Children
		//================================================================================

		const Vector< EntityHandle >& children = entity.Get( )->GetChildren( );

		// Write out number of children 
		buffer->Write< u32 >( ( u32 )children.size( ) );

		// Serialize all children into buffer
		for ( auto& c : children )
		{
			Serialize( c, buffer );
		}

		return Result::SUCCESS;
	}

	//=========================================================================================

	Result EntityArchiver::Deserialize( const String& filePath, Vector< EntityHandle >& out )
	{
		return Result::FAILURE;
	}

	//=========================================================================================

	EntityHandle EntityArchiver::Deserialize( const String& filePath )
	{
		Reset( );

		// Fill buffer to read
		mBuffer.ReadFromFile( filePath );

		// If valid, read from buffer and fill out entity handle
		if ( mBuffer.GetStatus( ) == BufferStatus::ReadyToRead )
		{
			return Deserialize( &mBuffer );
		}

		// Return invalid entity handle
		return EntityHandle( );
	}

	EntityHandle EntityArchiver::Deserialize( ByteBuffer* buffer )
	{
		// Read status of entity from buffer
		u32 entityStatus = buffer->Read< u32 >( );

		// If entity wasn't alive, then it wasn't serialized
		if ( !entityStatus )
		{
			// Return empty entity handle
			return EntityHandle();
		}

		// Get entity manager from engine
		EntityManager* entities = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< EntityManager >( )->ConstCast< EntityManager >( );

		// Handle to fill out
		EntityHandle handle = entities->Allocate( );

		//==========================================================================
		// Local Transform
		//========================================================================== 

		Transform local;

		// Read in position
		local.Position.x = buffer->Read< f32 >( );
		local.Position.y = buffer->Read< f32 >( );
		local.Position.z = buffer->Read< f32 >( ); 

		// Read in rotation
		local.Rotation.x = buffer->Read< f32 >( );
		local.Rotation.y = buffer->Read< f32 >( );
		local.Rotation.z = buffer->Read< f32 >( );
		local.Rotation.w = buffer->Read< f32 >( );

		// Read in scale
		local.Scale.x = buffer->Read< f32 >( );
		local.Scale.y = buffer->Read< f32 >( );
		local.Scale.z = buffer->Read< f32 >( );

		 //Set the transform of the entity
		handle.Get( )->SetLocalTransform( local );

		//=================================================================
		// Components
		//=================================================================

		u32 numComps = buffer->Read< u32 >( );

		for ( u32 i = 0; i < numComps; ++i )
		{
			// Get component's meta class
			const MetaClass* cmpCls = Object::GetClass( buffer->Read< String >( ) );

			if ( cmpCls )
			{
				// Attach new component to entity using MetaClass
				Component* cmp = handle.Get( )->AddComponent( cmpCls );
				if ( cmp )
				{
					Result res = cmp->DeserializeData( buffer );
					if ( res == Result::INCOMPLETE )
					{
						res = DeserializeObjectDataDefault( cmp, cmpCls, buffer );
					}
				}
			}
		}

		//=================================================================
		// Entity Children
		//=================================================================

		u32 numChildren = buffer->Read< u32 >();

		// Deserialize all children and add to handle
		for ( u32 i = 0; i < numChildren; ++i )
		{
			EntityHandle child = Deserialize( buffer );
			if ( child.Get() )
			{
				// Grab local transform of child that was just deserialized
				Transform localTrans = child.Get( )->GetLocalTransform( );

				// After adding child, local transform will be incorrect
				handle.Get( )->AddChild( child );

				// Restore local transform
				child.Get( )->SetLocalTransform( localTrans );
			}
		}

		return handle.Get( );
	} 

	//========================================================================================= 
}













