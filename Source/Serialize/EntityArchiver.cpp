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
		//==========================================================================
		// Local Transform
		//========================================================================== 

		Transform local = entity.Get( )->GetLocalTransform( );

		// Write out position
		mBuffer.Write< f32 >( local.Position.x );
		mBuffer.Write< f32 >( local.Position.y );
		mBuffer.Write< f32 >( local.Position.z ); 

		// Write out rotation
		mBuffer.Write< f32 >( local.Rotation.x );
		mBuffer.Write< f32 >( local.Rotation.y );
		mBuffer.Write< f32 >( local.Rotation.z );
		mBuffer.Write< f32 >( local.Rotation.w );

		// Write out scale
		mBuffer.Write< f32 >( local.Scale.x );
		mBuffer.Write< f32 >( local.Scale.y );
		mBuffer.Write< f32 >( local.Scale.z );

		//==========================================================================
		// Components
		//========================================================================== 

		const Vector< Component* >& comps = entity.Get( )->GetComponents( );

		// Write out number of comps
		mBuffer.Write< u32 >( ( u32 )comps.size( ) );

		// Write out component data
		for ( auto& c : comps )
		{
			// Get component's meta class
			const MetaClass* compCls = c->Class( );

			// Write out component class
			mBuffer.Write< String >( compCls->GetName( ) );

			// Serialize component data
			Result res = c->SerializeData( &mBuffer );
			if ( res == Result::INCOMPLETE )
			{
				SerializeObjectDataDefault( c, compCls );
			}
		}

		//================================================================================
		// Entity Children
		//================================================================================

		const Vector< EntityHandle >& children = entity.Get( )->GetChildren( );

		// Write out number of children 
		mBuffer.Write< u32 >( ( u32 )children.size( ) );

		// Serialize all children into buffer
		for ( auto& c : children )
		{
			Serialize( c );
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
				Component* cmp = handle.Get( )->Attach( cmpCls );
				if ( cmp )
				{
					Result res = cmp->DeserializeData( buffer );
					if ( res == Result::INCOMPLETE )
					{
						res = DeserializeObjectDataDefault( cmp, cmpCls );
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
