
//--------------------------------------------------------------------------
template <typename T>
void EntityManager::RegisterComponent()
{
	static_assert(std::is_base_of<Component, T>::value, "EntityManager::RegisterComponent:: T must inherit from Component.");
	u32 index = static_cast<u32>(Enjon::GetComponentType<T>());
	mComponents[ index ] = new ComponentWrapper<T>;
}

//--------------------------------------------------------------------------
template <typename T>
std::vector<T>* EntityManager::GetComponentList()
{
	u32 index = Enjon::GetComponentType<T>();
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
	assert(!entity->HasComponent<T>());

	// Entity id
	u32 eid = entity->GetID();

	// Get index into vector and assert that entity manager has this component
	u32 index = Enjon::GetComponentType<T>();
	assert(mComponents.at(index) != nullptr);

	ComponentWrapper<T>* cWrapper = static_cast<ComponentWrapper<T>*>(mComponents.at(index));
	auto cMap = &(cWrapper->mComponentMap);
	auto componentList = &(cWrapper->mComponentPtrs);

	// Create new component and place into map
	auto query = cMap->find(eid);
	if (query == cMap->end())
	{
		T component;
		component.SetEntity(entity);
		component.SetID(index);
		component.SetBase(cWrapper); 
		component.mEntityID = entity->mID;
		cWrapper->mComponentMap[eid] = component;
		componentList->push_back(&cWrapper->mComponentMap[eid]);

		// Set bitmask field for component
		entity->mComponentMask |= Enjon::GetComponentBitMask<T>();

		// Get component ptr and push back into entity components
		auto compPtr = &cWrapper->mComponentMap[eid];
		entity->mComponents.push_back(compPtr);

		return compPtr;
	}

	// Otherwise the entity already has the component
	assert(false);

	// Return null to remove warnings from compiler
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
	// Assert that it has component
	//assert(entity->HasComponent<T>());

	// Assert entity manager exists
	assert(entity != nullptr);

	u32 eid = entity->GetID();

	// Get component wrapper
	auto cWrapper = static_cast<ComponentWrapper<T>*>(mComponents.at(Enjon::GetComponentType<T>()));

	// Return component
	auto* cMap = &cWrapper->mComponentMap;
	auto query = cMap->find(eid);
	if (query != cMap->end())
	{
		return &query->second;
	}

	// We shoudln't hit this
	//assert(false);

	// Return nullptr to get rid of compiler warnings
	return nullptr;
}

//=======================================================================================

template <typename T>
void EntityManager::DetachComponentFromEntity(Entity* entity)
{
	u32 idx = Enjon::GetComponentType<T>();
	assert(mComponents.at(idx) != nullptr);

	auto cWrapper = static_cast<ComponentWrapper<T>*>(mComponents.at(idx));
	auto cPtrList = &cWrapper->mComponentPtrs;
	auto cMap = &cWrapper->mComponentMap;

	auto compPtr = &cWrapper->mComponentMap[entity->mID];

	// Search through entity component list and remove reference
	auto entComps = &entity->mComponents;
	entComps->erase(std::remove(entComps->begin(), entComps->end(), compPtr), entComps->end());

	// Remove ptr from point list map
	cPtrList->erase(std::remove(cPtrList->begin(), cPtrList->end(), compPtr), cPtrList->end());	

	// Finally remove from map
	cMap->erase(entity->mID);
}

//=======================================================================================

template <typename T>
T* Entity::AddComponent( )
{
	return mManager->AddComponent< T >( GetHandle( ) );
}

//=======================================================================================









