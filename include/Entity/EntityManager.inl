
//--------------------------------------------------------------------------
template <typename T>
void EntityManager::RegisterComponent()
{
	static_assert(std::is_base_of<Component, T>::value, "EntityManager::RegisterComponent:: T must inherit from Component.");
	u32 index = static_cast<u32>(Component::GetComponentType<T>());
	mComponents[ index ] = new ComponentWrapper<T>;
}

//--------------------------------------------------------------------------
template <typename T>
std::vector<T>* EntityManager::GetComponentList()
{
	u32 index = Component::GetComponentType<T>();
	assert(Components.at(index) != nullptr);
	return &(static_cast<ComponentWrapper<T>*>(Components.at(index))->mComponentPtrs);	
}

//--------------------------------------------------------------------------

template <typename T>
T* EntityManager::AddComponent(const Enjon::EntityHandle& handle)
{
	Enjon::Entity* entity = handle.Get( );

	// Assert entity is valid
	assert(entity != nullptr);
	// Check to make sure isn't already attached to this entity

	// If component exists, return it
	if ( entity->HasComponent< T >( ) )
	{
		return entity->GetComponent< T >( );
	}

	// Entity id
	u32 eid = entity->GetID();

	// Get index into vector and assert that entity manager has this component
	u32 compIdx = Component::GetComponentType<T>();

	// If the component doens't exist, need to register it
	if ( !ComponentBaseExists< T >( ) )
	{
		RegisterComponent< T >( );
	}

	// Make sure that component isn't still null
	assert(mComponents.at(compIdx) != nullptr); 

	ComponentWrapperBase* base = mComponents[ compIdx ];

	// If component already exists
	if ( base->HasEntity( eid ) )
	{
		return (T*)base->GetComponent( eid );
	}

	// Otherwise new component and place into map
	T* component = (T*)base->AddComponent( eid );
	component->SetEntity(entity);
	component->SetID(compIdx);
	component->SetBase( base );
	component->mEntityID = entity->mID; 

	// Get component ptr and push back into entity components
	entity->mComponents.push_back( compIdx ); 

	return component;
 
	// Shouldn't get here
	return nullptr;
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
	entity->mComponentMask ^= Enjon::GetComponentBitMask<T>();
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








