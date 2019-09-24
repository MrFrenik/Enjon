// File: Entity.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Entity/EntityManager.h"
#include "ImGui/ImGuiManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"
#include "Serialize/ObjectArchiver.h"
#include "Utils/FileUtils.h"
#include "Base/World.h"

namespace Enjon 
{
	//=====================================================================

	Result Entity::OnEditorUI( )
	{
		// Grab imgui manager
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
 
		Vec2 padding( 20.0f, 7.0f );
		f32 height = ImGui::GetWindowSize( ).y - ImGui::GetCursorPosY( ) - padding.y;
		// Display all components and transform information
		ImGui::ListBoxHeader( "##CompLists", ImVec2(ImGui::GetWindowSize().x - 20.0f, height ) );
		{ 
			// Name / Tag information
			if ( ImGui::CollapsingHeader( "Label" ) )
			{
				ImGui::PushFont( igm->GetFont( "WeblySleek_14" ) );
				igm->DebugDumpProperty( this, Class( )->GetPropertyByName( ENJON_TO_STRING( mName ) ) ); 
				ImGui::PopFont( ); 
			}

			// Transform information
			if ( ImGui::CollapsingHeader( "Transform" ) )
			{
				Transform current = mLocalTransform;
				ImGui::PushFont( igm->GetFont( "WeblySleek_14" ) );
				igm->DebugDumpProperty( this, Class( )->GetPropertyByName( ENJON_TO_STRING( mLocalTransform ) ) ); 
				ImGui::PopFont( );
			} 

			for ( auto& c : GetComponents( ) )
			{
				if ( ImGui::CollapsingHeader( c->Class( )->GetName( ).c_str( ) ) )
				{
					ImGui::PushFont( igm->GetFont( "WeblySleek_14" ) );
					igm->InspectObject( c ); 
					ImGui::PopFont( );

					if ( ImGui::Button( Utils::format( "Remove##%zu", (u32)(usize)c ).c_str() ) )
					{
						RemoveComponent( c->Class( ) );
					}
				}

				c->OnViewportDebugDraw(); 
			} 

			/*
			if ( mArchetype )
			{
				ImGui::Text( Utils::format( "Archetype: {}", mArchetype->GetName( ) ).c_str( ) );
			}
			else
			{
				ImGui::Text( "Archetype: Default" ); 
			}

			if ( mPrototypeEntity )
			{
				ImGui::Text( Utils::format( "Prototype Entity: {}", mPrototypeEntity.Get()->GetUUID().ToString() ).c_str( ) );
				ImGui::Text( Utils::format( "Prototype Entity: {}", mPrototypeEntity.Get( )->GetID( ) ).c_str( ) );
			}
			else
			{
				ImGui::Text( "Prototype Entity: null" ); 
			}

			ImGui::Text( Utils::format( "ID: {}", GetID( ) ).c_str( ) );
			ImGui::Text( Utils::format( "UUID: {}", GetUUID().ToString() ).c_str( ) );

			ImGui::Text( Utils::format( "Instanced Ent:" ).c_str( ) );
			for ( auto& i : GetInstancedEntities() )
			{
				ImGui::Text( Utils::format( "\tID: {}", i.Get()->GetID( ) ).c_str( ) );
				ImGui::Text( Utils::format( "\tUUID: {}", i.Get()->GetUUID().ToString() ).c_str( ) );

			}
			if ( mParent )
			{ 
				ImGui::Text( Utils::format( "Parent ID: {}", mParent.Get()->GetID( ) ).c_str( ) );
				ImGui::Text( Utils::format( "Parent UUID: {}", mParent.Get( )->GetUUID( ).ToString( ) ).c_str( ) );
			}
			*/
			
		} 
		ImGui::ListBoxFooter( ); 

		return Result::SUCCESS; 
	} 

	//=====================================================================
	
	u32 Entity::GetID() const
	{ 
		return mID; 
	}

	//===================================================================== 

	String Entity::GetName( ) const
	{
		return mName;
	}

	//===================================================================== 

	void Entity::SetName( const String& name )
	{
		mName = name;
	}

	//===================================================================== 

	const World* Entity::GetWorld( ) const
	{
		return mWorld;
	}

	//=====================================================================

	Result Entity::MergeWith( Object* sourceObj, MergeType mergeType )
	{
		const MetaClass* cls = sourceObj->Class();

		// Can't merge with an instance of another object type
		if ( !cls->InstanceOf< Entity >( ) )
		{
			return Result::FAILURE;
		} 

		// Cast to entity
		Entity* source = sourceObj->ConstCast< Entity >( );

		// Check local transform of objects
		bool transformPropChanged = false;
		//if ( !ObjectArchiver::HasPropertyOverrides( &mLocalTransform ) )
		{
			const MetaProperty* scaleProp = mLocalTransform.Class( )->GetPropertyByName( "mScale" );
			if ( !scaleProp->HasOverride( &mLocalTransform ) )
			{
				ObjectArchiver::MergeProperty( &source->mLocalTransform, &mLocalTransform, scaleProp, mergeType );
			}

			const MetaProperty* positionProp = mLocalTransform.Class( )->GetPropertyByName( "mPosition" );
			if ( !positionProp->HasOverride( &mLocalTransform ) )
			{
				ObjectArchiver::MergeProperty( &source->mLocalTransform, &mLocalTransform, positionProp, mergeType );
			}

			const MetaProperty* rotationProp = mLocalTransform.Class( )->GetPropertyByName( "mRotation" );
			if ( !rotationProp->HasOverride( &mLocalTransform ) )
			{
				ObjectArchiver::MergeProperty( &source->mLocalTransform, &mLocalTransform, rotationProp, mergeType );
			}
		} 

		bool componentPropChangeExists = false;
		for ( auto& c : source->GetComponents() )
		{
			// If component doesn't exist, add it
			if ( !HasComponent( c->Class() ) )
			{ 
				// Add component 
				AddComponent( c->Class( ) );
			}

			// Attempt to merge the components 
			ObjectArchiver::MergeObjects( c, GetComponent( c->Class() ), mergeType );	
		}

		// Add override if property changes
		//if ( componentPropChangeExists )
		//{
		//	MetaProperty* prop = const_cast< MetaProperty* >( cls->GetPropertyByName( ENJON_TO_STRING( mComponents ) ) );
		//	prop->AddOverride( this );
		//}

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
					if ( !ObjectArchiver::HasPropertyOverrides( child ) )
					{
						child->Destroy();
					}
				} 
				// Case 2: Proto ent valid ( was found )
				else
				{
					// Merge with proto ent
					ObjectArchiver::MergeObjects( protoEnt.Get(), child, mergeType );
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
		Vector< EntityHandle > children = source->GetChildren();	

		// Attempt to merge new entities into arrays
		for ( auto& e : children )
		{
			Entity* sourceEnt = e.Get();

			// Get id of source entity
			UUID sourceId = sourceEnt->GetUUID(); 

			// Proto entity not found, so create new entity and add it
			if ( !containsProtoEntity( sourceId ) )
			{ 
				// Construct new instanced entity
				EntityHandle instanced = EngineSubsystem( EntityManager )->InstanceEntity( sourceEnt, GetWorld( )->ConstCast< World >( ) );

				// Cache local transform
				Transform localTransform = instanced.Get( )->GetLocalTransform( );

				// Add to children
				AddChild( instanced );

				// Guess I need to merge objects?
				ObjectArchiver::MergeObjects( sourceEnt, instanced.Get( ), mergeType );

				// Set archetype of instanced entity to this archetype
				instanced.Get( )->SetArchetype( mArchetype );

				// Set local transform 
				instanced.Get( )->SetLocalTransform( localTransform );
			} 
		}

		// Return incomplete result so we can continue to merge other default properties
		return Result::INCOMPLETE; 
	} 

	//=====================================================================

#define ENJON_RECORD_OVERRIDE_POD( cls, sourceObj, destObj, prop, podType )\
{\
	podType sourceVal = *cls->GetValueAs< podType >( sourceObj, prop );\
	podType destVal = *cls->GetValueAs< podType >( destObj, prop );\
\
	if ( sourceVal != destVal )\
	{\
		prop->AddOverride( destObj, sourceObj );\
	}\
}

	Result Entity::RecordPropertyOverrides( Object* source )
	{
		// Get source class
		const MetaClass* cls = source->Class( );

		if ( !source->Class( )->InstanceOf( Class() ) )
		{
			// Error, cannot operate on separate types
			return Result::FAILURE;
		}

		for ( usize i = 0; i < cls->GetPropertyCount( ); ++i )
		{
			// Grab property from class
			MetaProperty* prop = const_cast< MetaProperty* >( cls->GetProperty( i ) );

			switch ( prop->GetType( ) )
			{
				case MetaPropertyType::S32: ENJON_RECORD_OVERRIDE_POD( cls, source, this, prop, s32 ); break; 
				case MetaPropertyType::U32: ENJON_RECORD_OVERRIDE_POD( cls, source, this, prop, u32 ); break; 
				case MetaPropertyType::F32: ENJON_RECORD_OVERRIDE_POD( cls, source, this, prop, f32 ); break; 
				case MetaPropertyType::Vec2: ENJON_RECORD_OVERRIDE_POD( cls, source, this, prop, Vec2 ); break; 
				case MetaPropertyType::Vec3: ENJON_RECORD_OVERRIDE_POD( cls, source, this, prop, Vec3 ); break; 
				case MetaPropertyType::Vec4: ENJON_RECORD_OVERRIDE_POD( cls, source, this, prop, Vec4 ); break; 
				case MetaPropertyType::Quat: ENJON_RECORD_OVERRIDE_POD( cls, source, this, prop, Quaternion ); break; 

				case MetaPropertyType::Transform:
				{ 
					Transform* sourceTransform = cls->GetValueAs< Transform >( source, prop )->ConstCast< Transform >( );
					Transform* destTransform = cls->GetValueAs< Transform >( this, prop )->ConstCast < Transform >( );

					// Record property overrides
					ObjectArchiver::RecordAllPropertyOverrides( sourceTransform, destTransform ); 
				} break; 

				default: break;
			}
		} 

		// Record children overrides with their prototype entities
		for ( auto& c : mChildren )
		{
			if ( c.Get( )->HasPrototypeEntity( ) )
			{
				c.Get( )->RecordPropertyOverrides( c.Get( )->GetPrototypeEntity( ).Get( ) );
			}
		}

		// Record overrides for components
		for ( auto& c : GetComponents() )
		{
			// Only record overrides if proto entity has component as well
			Component* sourceComponent = mPrototypeEntity.Get( )->GetComponent( c->Class( ) );
			if ( sourceComponent )
			{
				ObjectArchiver::RecordAllPropertyOverrides( sourceComponent, c ); 
			}
		}

		return Result::SUCCESS;
	} 

	//=====================================================================

	Result Entity::ClearAllPropertyOverrides( )
	{
		// Clear components
		for ( auto& c : GetComponents( ) )
		{
			ObjectArchiver::ClearAllPropertyOverrides( c );
		}

		return Result::INCOMPLETE;
	}

	//=====================================================================
}