//------------------------------------------------------------------------------
template <typename T>
bool Entity::HasComponent()
{
	u32 compId = Component::GetComponentType< T >( );
	auto query = std::find( mComponents.begin( ), mComponents.end( ), compId );
	return ( query != mComponents.end( ) );
}

//------------------------------------------------------------------------------
template <typename T>
T* Entity::GetComponent()
{ 
	// Assert entity manager exists
	assert(mManager != nullptr);

	return mManager->GetComponent<T>(this);
}

//------------------------------------------------------------------------------
template <typename T>
T* Entity::AddComponent()
{
	// Make sure that entity manager isn't null
	assert(mManager != nullptr);

	// If component exists, return it
	if ( HasComponent< T >( ) )
	{
		return GetComponent< T >( );
	} 

	return mManager->AddComponent<T>(this);
}

//------------------------------------------------------------------------------
template <typename T>
void Entity::RemoveComponent()
{ 
	// Make sure entity manager isn't null
	assert(mManager != nullptr); 

	// Detach component
	mManager->RemoveComponent<T>(this);
}
