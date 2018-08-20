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

		return Result::SUCCESS;
	}

	//=======================================================================================

	Archetype::Archetype( const Archetype& other )
	{
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
 
		//EntityHandle handle = EngineSubsystem( EntityManager )->Allocate( em->GetArchetypeWorld( ) );
		//handle.Get( )->SetName( "Root" );

		//// Stupid bullshit so it won't be destroyed when the window is closed...
		//handle.Get( )->mIsArchetypeRoot = true;

		//mRoot = handle.Get( ); 
	}

	//=======================================================================================

	Result Archetype::SerializeData( ByteBuffer* buffer ) const 
	{
		// Just copy all entity data into the buffer
		if ( !mRoot )
		{
			EntityHandle handle = EngineSubsystem( EntityManager )->Allocate( );
			handle.Get( )->SetName( "Root" );
			EntityArchiver::Serialize( handle, buffer );
			handle.Get( )->Destroy( );
		}
		else
		{
			// TODO(John): GET RID OF ALL OF THIS STUPID SHIT
			// Save all the stupid shit and stuff...
			for ( auto& c : mRoot->GetChildren( ) )
			{
				c.Get( )->mIsArchetypeRoot = true;
			} 

			EntityArchiver::Serialize( mRoot, buffer ); 
		}

		return Result::SUCCESS;
	}

	//=======================================================================================

	Result Archetype::DeserializeData( ByteBuffer* buffer )
	{
		// If already a root exists, then destroy it
		if ( mRoot )
		{
			mRoot->mIsArchetypeRoot = false;
			mRoot->Destroy( );
		}

		// Deserialize into root
		mRoot = EntityArchiver::Deserialize( buffer, EngineSubsystem( EntityManager )->GetArchetypeWorld( ) ).Get( );
		mRoot->mIsArchetypeRoot = true;

		// THIS IS A FUCKING PROBLEM AND HACKY BULLSHIT. FUCKING FIX IT.
		for ( auto& c : mRoot->GetChildren( ) )
		{
			c.Get( )->mIsArchetypeRoot = true;
		}
 
		return Result::SUCCESS; 
	}

	//=======================================================================================

	EntityHandle Archetype::GetRootEntity( )
	{
		if ( !mRoot )
		{
			// Grab entity manager
			EntityManager* em = EngineSubsystem( EntityManager );

			// Construct root if not available
			mRoot = em->Allocate( em->GetArchetypeWorld( ) ).Get( );
			mRoot->mIsArchetypeRoot = true;
			mRoot->SetName( "Root" );
		}

		return mRoot;
	}

	//=======================================================================================

	EntityHandle Archetype::CopyRootEntity( Transform transform, World* world )
	{
		EntityHandle rootCopy = EngineSubsystem( EntityManager )->CopyEntity( mRoot, world );

		// Reset all the important things
		Entity* copy = rootCopy.Get( );
		copy->mInstancedEntities = mRoot->mInstancedEntities;
		copy->mUUID = mRoot->mUUID;
		copy->SetLocalTransform( transform );

		return rootCopy;
	}

	//=======================================================================================

	EntityHandle Archetype::ConstructFromEntity( const EntityHandle& entity )
	{
		// All I need to do here is shallow copy all of the other entity's fields into this root entity's
		if ( mRoot )
		{ 
			EntityManager* em = EngineSubsystem( EntityManager );
			EntityHandle newRoot = em->CopyEntity( entity, entity.Get( )->GetWorld( )->ConstCast< World >( ) );
			Entity* nr = newRoot.Get( );
			nr->mInstancedEntities = mRoot->mInstancedEntities;
			nr->mUUID = mRoot->mUUID; 
			nr->mArchetype = mRoot->mArchetype;

			// Destroy previous entity
			mRoot->mIsArchetypeRoot = false;
			mRoot->Destroy( );

			// Reset to new entity
			mRoot = nr;
			mRoot->mIsArchetypeRoot = true;
			for ( auto& c : mRoot->GetChildren( ) )
			{
				c.Get( )->mIsArchetypeRoot = true;
			}
		}

		return mRoot;
	} 

	//=======================================================================================

	void Archetype::RecursivelySetArchetype( const EntityHandle& handle )
	{
		Entity* ent = handle.Get( );
		ent->SetArchetype( this );
		for ( auto& c : ent->GetChildren( ) )
		{
			RecursivelySetArchetype( c );
		}
	}

	//=======================================================================================

	EntityHandle Archetype::Instantiate( const Transform& transform, World* world ) 
	{ 
		// If world isn't given, then get default world from engine
		if ( !world )
		{
			world = Engine::GetInstance( )->GetWorld( );
		} 

		// Instance this entity
		EntityHandle instanced = EngineSubsystem( EntityManager )->InstanceEntity( mRoot, world );
 
		// Set transform for entity
		instanced.Get( )->SetLocalTransform( transform ); 

		// Recursively set archetype of entity and children to this
		RecursivelySetArchetype( instanced );

		// Return newly constructed entity
		return instanced;
	}

	//=======================================================================================
}


/*

	// Serializing / Deserializing entities with archetypes in mind 
 
	EntityHandle EntityArchiver::Deserialize( ByteBuffer* buffer, World* world )
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
		// TODO(): This will fail if entity cannot be allocated! Need to check for that and then exit gracefully.
		EntityHandle handle = entities->Allocate( world );

		// Continue to deserialize data into entity
		Result res = EntityArchiver::DeserializeInternal( handle, buffer, world ); 

		if ( res == Result::FAILURE )
		{ 
			// Error...
			return res;
		}

		// Try and resolve changes with archetype if possible
		Entity* ent = handle.Get();
		if ( ent && ent->HasPrototypeEntity() )
		{
			EntityHandle protoEnt = ent->GetPrototypeEntity();
			
			// Attempt to merge properties ( AcceptMerge )
			// Would like for this to be as generic as possible to be able to extend to any type
			MergeObjects( protoEnt.Get(), ent, MergeType::AcceptMerge );
		} 
	}

	virtual void Entity::MergeWith( Entity* source, MergeType mergeType ) override
	{
		const MetaClass* cls = source->Class();

		// Would like a generic way to be able to detect if a property change does exist eventually, but
		// since the component list is just a list of opaque ids, tough to really do that

		// Specifics have to deal with children and components

		for ( auto& c : source->GetComponents() )
		{
			if ( !HasComponent( c->Class() )
			{
				componentPropChangeExists |= true;
			}
			else
			{
				// Maybe have a way of being able to merge this specifically depending on the component itself? Provide a way to override this? 
				MergeObjects( c, dest->GetComponent< c->Class() >(), mergeType );	
			} 
		}

		if ( componentPropChangeExists )
		{
			MetaProperty* prop = const_cast< MetaProperty* >( cls->GetPropertyByName( ENJON_TO_STRING( mComponents ) ) );
			prop->SetPropertyOverride( dest, true );
		}

		/////////////////////////
		// Children /////////////
		///////////////////////// 

		// How to check if the children are correct...
		for( auto& e : GetChildren() )
		{
			// Need to check and see if this child has a proto entity
			Entity* child = e.Get();
			if ( child->HasPrototypeEntity() )
			{
				EntityHandle protoEnt = child->GetPrototypeEntity();

				// Case 1: Proto ent is invalid ( not found )
				if ( !protoEnt )
				{
					// Delete the entity IFF there are no existing property overrides
					if ( child->HasPropertyOverrides() )
					{
						child->Destroy();
					}
				} 
				// Case 2: Proto ent valid ( was found )
				else
				{
					// Merge with proto ent
					MergeObjects( protoEnt.Get(), child, mergeType );
				}
			}
		}

		auto containsProtoEntity = [&]( const UUID& sourceId )
		{
			// Search for specific sourceId
			for ( auto& e : GetChildren() )
			{
				Entity* child = e.Get();
				if ( child->HasPrototypeEntity() && child->GetPrototypeEntity().Get()->GetUUID() == sourceId )
				{
					return true;
				}
			}

			// Not found in child array
			return false;
		};

		// Cache children vector
		Vector< EntityHandle > children = GetChildren();	

		// Attempt to merge new entities into arrays
		for ( auto& e : children )
		{
			Entity* sourceEnt = e.Get();

			// Get id of source entity
			UUID sourceId = sourceEnt->GetUUID(); 

			// Proto entity not found, so create new entity and add it
			if ( !containsProtoEntity( sourceId )
			{ 
				// Construct new instanced entity
				EntityHandle instanced = EngineSubsystem( EntityManager )->InstanceEntity( sourceEnt );

				// Add to children
				AddChild( instanced );
			} 
		}

		// Generic property merging that I can write later
		for ( usize i = 0; i < cls->GetPropertyCount(); ++i )
		{ 
			// Stuff...
		}
	} 

	virtual Result Object::MergeWith( Object* other, MergeType mergeType )
	{
		return Result::INCOMPLETE;
	} 

	void MergeObjects( Object* source, Object* dest, MergeType mergeType )
	{
		if ( !source->InstanceOf( dest->Class() )
		{
		// Error, can't operate on separate types.
			return;
		}

		const MetaClass* cls = source->Class();

		// Attempt to do customized merging depending on the object itself
		Result mergeResult = dest->MergeWith( source, mergeType );

		// If incomplete operation, then default merging behavior
		if ( mergeResult == Result::INCOMPLETE )
		{
			MergeObjectsDefault( source, dest, mergeType );
		}
	}

	virtual Result Object::HasPropertyOverrides( bool& hasOverrides )
	{
		return Result::INCOMPLETE;
	}

	virtual bool Entity::HasPropertyOverrides() override
	{
		// Perhaps the bool should be passed in instead?
		bool containsOverrides = false;

		for ( auto& c : GetComponents() )
		{
			containsOverrides |= c->HasPropertyOverrides();
		}

		for ( auto& e : GetEntities() )
		{
			containsOverrides |= e.Get()->HasPropertyOverrides();
		}

		return containsOverrides;
	}

	void MergeObjectsDefault( Object* source, Object* dest, MergeType mergeType )
	{
		const MetaClass* cls = source->Class();

		for ( usize i = 0; i < cls->GetPropertyCount(); ++i )
		{
			const MetaProperty* prop = cls->GetProperty( i );

			// Maybe same as "IsSerializeable"? 
			if ( prop->IsMergeable() )
			{
				switch( mergeType )
				{
					case MergeType::AcceptSource:
					{
						MergeProperty( source, dest, prop );
					} break;

					case MergeType::AcceptTarget:
					{
						MergeProperty( dest, source, prop );
					} break;

					case MergeType::AcceptMerge:
					{
						// Only merge iff destination object doesn't have a property override
						if ( !prop->HasOverride( dest ) )
						{
							MergeProperty( source, dest, prop );
						}
					} break;
				}
			}
		}
	} 

	void MergeProperty( Object* source, Object* dest, const MetaProperty* prop )
	{
		// Will merge the source object property into the destination property
		const MetaClass* cls = source->Class();
		
		switch ( prop->GetType() ) 
		{
			case MetaPropertyType::U32:
			{ 
				cls->SetValue( dest, prop, *cls->GetValueAs< u32 >( source, prop ) ); 
			} break;

			case MetaPropertyType::F32:
			{ 
				cls->SetValue( dest, prop, *cls->GetValueAs< f32 >( source, prop ) ); 
			} break;

			// Etc...
		}
	}

*/













