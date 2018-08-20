// File: Entity.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Entity/EntityManager.h"
#include "ImGui/ImGuiManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"
#include "Serialize/ObjectArchiver.h"
#include "Base/World.h"

#include <fmt/format.h>

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

					if ( ImGui::Button( fmt::format( "Remove##{}", (u32)c ).c_str() ) )
					{
						RemoveComponent( c->Class( ) );
					}
				}
			} 

			if ( mArchetype )
			{
				ImGui::Text( fmt::format( "Archetype: {}", mArchetype->GetName( ) ).c_str( ) );
			}
			else
			{
				ImGui::Text( "Archetype: Default" ); 
			}

			ImGui::Text( fmt::format( "ID: {}", GetID( ) ).c_str( ) );
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
		if ( !ObjectArchiver::HasPropertyOverrides( &mLocalTransform ) )
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

				// Add to children
				AddChild( instanced );

				// Guess I need to merge objects?
				ObjectArchiver::MergeObjects( sourceEnt, instanced.Get( ), mergeType );

				// Set archetype of instanced entity to this archetype
				instanced.Get( )->SetArchetype( mArchetype );
			} 
		}

		// Return incomplete result so we can continue to merge other default properties
		return Result::INCOMPLETE; 
	} 

}