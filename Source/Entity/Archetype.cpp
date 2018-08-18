// File: Archetype.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Entity/Archetype.h"
#include "Entity/EntityManager.h"
#include "Base/World.h"
#include "Math/Transform.h"
#include "Asset/ArchetypeAssetLoader.h"
#include "SubsystemCatalog.h"
#include "Serialize/EntityArchiver.h"
#include "Serialize/ObjectArchiver.h"
#include "Serialize/BaseTypeSerializeMethods.h"
#include "Serialize/EntityArchiver.h"
#include "Engine.h" 

namespace Enjon
{ 
	//=======================================================================================

	Result Archetype::CopyFromOther( const Asset* other )
	{
		// Make sure this object is of class archetype
		MetaClass::AssertIsType< Archetype >( other );

		// Cast to archetype
		const Archetype* otherArch = other->Cast< Archetype >( );

		// Copy byte buffer over
		mEntityData.CopyFromOther( otherArch->mEntityData ); 

		return Result::SUCCESS;
	}

	//=======================================================================================

	Archetype::Archetype( const Archetype& other )
	{
		mEntityData.CopyFromOther( other.mEntityData );
	}

	//=======================================================================================

	void Archetype::ExplicitConstructor( )
	{
		// Need to construct empty entity, fill out buffer data, then destroy entity
		EntityManager* em = EngineSubsystem( EntityManager );

		// Cannot continue with construction
		if ( !em )
		{
			return;
		}

		EntityHandle handle = EngineSubsystem( EntityManager )->Allocate( em->GetArchetypeWorld( ) );
		handle.Get( )->SetName( "Root" );

		// Serialize into data buffer
		EntityArchiver::Serialize( handle, &mEntityData );

		// Destroy empty entity
		handle.Get( )->Destroy( );
	}

	//=======================================================================================

	Result Archetype::SerializeData( ByteBuffer* buffer ) const 
	{
		// Just copy all entity data into the buffer
		buffer->AppendBuffer( mEntityData );

		return Result::SUCCESS;
	}

	//=======================================================================================

	Result Archetype::DeserializeData( ByteBuffer* buffer )
	{
		// Reset data
		mEntityData.Reset( );

		// Deserialize all remaining data from buffer into the entity data buffer from its current read position
		mEntityData.AppendBufferFromReadPosition( buffer );

		return Result::SUCCESS; 
	}

	//=======================================================================================

	void Archetype::ConstructFromEntity( const EntityHandle& entity )
	{
		// Just clear the previous buffer data and then deserialize into it? 
		mEntityData.Reset( );

		AssetHandle< Archetype > archType = entity.Get( )->GetArchetype( );

		entity.Get( )->SetArchetype( nullptr );

		// Serialiize entity data using new entity
		EntityArchiver::Serialize( entity, &mEntityData );

		entity.Get( )->SetArchetype( archType );
	} 

	//=======================================================================================

	EntityHandle Archetype::Instantiate( const Transform& transform, World* world ) 
	{ 
		// If world isn't given, then get default world from engine
		if ( !world )
		{
			world = Engine::GetInstance( )->GetWorld( );
		} 

		// Deserialize data, construct new entity, and place into given world
		// NOTE(John): Will fail if entity data is corrupted or not ready to read
		// Another small change
		// One more test for branch stuff
		EntityHandle entity = EntityArchiver::Deserialize( &mEntityData, world );
 
		// Generate new uuid for entity
		entity.Get( )->SetUUID( UUID::GenerateUUID( ) );

		// Set transform for entity
		entity.Get( )->SetLocalTransform( transform ); 

		// Set archetype of entity to this
		entity.Get( )->SetArchetype( this ); 

		// Reset read position of buffer
		mEntityData.SetReadPosition( 0 );

		// Return newly constructed entity
		return entity;
	}

	//=======================================================================================
}


