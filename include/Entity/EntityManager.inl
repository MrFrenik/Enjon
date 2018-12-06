
//======================================================================================================

template <typename T>
void EntityManager::RegisterComponent()
{
	static_assert(std::is_base_of<Component, T>::value, "EntityManager::RegisterComponent:: T must inherit from Component.");
	u32 index = static_cast<u32>(Component::GetComponentType<T>());
	mComponents[ index ] = new ComponentArray( );
}

//======================================================================================================

template <typename T>
void EntityManager::RegisterComponentSystem( )
{
	static_assert(std::is_base_of<IComponentSystem, T>::value, "EntityManager::RegisterComponentSystem:: T must inherit from IComponentSystem.");
	u32 index = static_cast< u32 >( Object::GetClass< T >()->GetTypeId() );
	mComponentSystems[ index ] = new T( ); 
}

//======================================================================================================

template < typename T >
ComponentInstanceData< T >* EntityManager::RegisterIComponent( )
{
	static_assert( std::is_base_of< Component, T >::value, "EntityManager::RegisterIComponent:: T must inherit from IComponent." );
	const MetaClass* cls =  Object::GetClass< T >( );
	u32 cId = static_cast< u32 >( cls->GetTypeId( ) );
	if ( mComponentInstanceDataMap.find( cId ) == mComponentInstanceDataMap.end( ) )
	{
		mComponentInstanceDataMap[ cId ] = ( ComponentInstanceData< T >* )( IComponentInstanceData::ConstructComponentInstanceData< T >( ) );
	}
	return ( ComponentInstanceData< T >* )( mComponentInstanceDataMap[ cId ] );
}

//======================================================================================================

template < typename T >
ComponentInstanceData< T >* EntityManager::GetIComponentInstanceData( )
{
	return ( RegisterIComponent< T >( ) );
}

//====================================================================================================== 

template <typename T>
T* EntityManager::AddComponent(const Enjon::EntityHandle& handle)
{
	return ( T* )( AddComponent( Object::GetClass< T >( ), handle ) );
}

//--------------------------------------------------------------------------
template <typename T>
void EntityManager::RemoveComponent(Entity* entity)
{
	// Check to make sure isn't already attached to this entity
	assert(entity->HasComponent<T>());

	// Remove component from entity manager components
	DetachComponentFromEntity<T>(entity);

	// Set bitmask field for component
	//entity->mComponentMask ^= Enjon::GetComponentBitMask<T>();
}

//--------------------------------------------------------------------------
template <typename T>
T* EntityManager::GetComponent(Entity* entity)
{ 
	// Assert entity manager exists
	assert(entity != nullptr);

	u32 eid = entity->GetID();

	// Get component idx
	u32 compIdx = Component::GetComponentType<T>();
	assert(mComponents.at(compIdx) != nullptr); 

	ComponentWrapperBase* base = mComponents.at( compIdx );
	return (T*)base->GetComponent( eid ); 
}

//=======================================================================================

template <typename T>
void EntityManager::DetachComponentFromEntity(Entity* entity)
{
	u32 compIdx = Enjon::GetComponentType<T>();
	assert(mComponents.at(compIdx) != nullptr);

	auto cWrapper = static_cast<ComponentWrapper<T>*>(mComponents.at(compIdx));
	auto cPtrList = &cWrapper->mComponentPtrs;
	auto cMap = &cWrapper->mComponentMap; 
	auto compPtr = &cWrapper->mComponentMap[entity->mID];

	// Search through entity component list and remove reference
	auto entComps = &entity->mComponents;
	entComps->erase(std::remove(entComps->begin(), entComps->end(), compIdx), entComps->end());

	// Remove ptr from point list map
	cPtrList->erase(std::remove(cPtrList->begin(), cPtrList->end(), compPtr), cPtrList->end());	

	// Finally remove from map
	cMap->erase(entity->mID);

	// Delete comp and set to nullptr
	delete compPtr;
	compPtr = nullptr;
}

//=======================================================================================

/**
*@brief
*/
template <typename T >
bool EntityManager::ComponentBaseExists( )
{
	u32 idx = Component::GetComponentType< T >( );
	return ( mComponents.find( idx ) != mComponents.end( ) );
}

//=======================================================================================

template < typename T >
Vector< Component* > EntityManager::GetAllComponentsOfType( )
{
	// Get idx of component
	u32 idx = Component::GetComponentType< T >( );

	// If the component exists, then return its component array
	if ( ComponentBaseExists( idx ) )
	{
		return mComponents[idx]->GetComponents( );
	}
	// Otherwise return empty vector
	else
	{ 
		return Vector<Component*>( );
	}
}

//=======================================================================================








