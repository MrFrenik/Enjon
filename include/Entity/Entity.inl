//------------------------------------------------------------------------------
template <typename T>
bool Entity::HasComponent()
{
	return ((mComponentMask & Enjon::GetComponentBitMask<T>()) != 0);
}

//------------------------------------------------------------------------------
template <typename T>
T* Entity::GetComponent()
{
	// Assert that it has component
	//assert(HasComponent<T>());

	// Assert entity manager exists
	assert(mManager != nullptr);

	return mManager->GetComponent<T>(this);
}

//------------------------------------------------------------------------------
template <typename T>
T* Entity::Attach()
{
	// Check to make sure isn't already attached to this entity
	assert(!HasComponent<T>());

	// Make sure that entity manager isn't null
	assert(mManager != nullptr);

	return mManager->Attach<T>(this);
}

//------------------------------------------------------------------------------
template <typename T>
void Entity::Detach()
{
	// Check to make sure isn't already attached to this entity
	assert(HasComponent<T>());

	// Make sure entity manager isn't null
	assert(mManager != nullptr);

	// Detach component
	mManager->Detach<T>(this);
}
