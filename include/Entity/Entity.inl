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
ComponentHandle< T >& Entity::GetComponent()
{ 
	return EngineSubsystem( EntityManager )->GetComponent< T >( this );
}

//------------------------------------------------------------------------------
template <typename T>
ComponentHandle< T >& Entity::AddComponent()
{ 
	// If component exists, return it
	if ( HasComponent< T >( ) )
	{
		return GetComponent< T >( );
	} 

	return EngineSubsystem( EntityManager )->AddComponent< T >( this );
}

//------------------------------------------------------------------------------
template <typename T>
void Entity::RemoveComponent()
{ 
	EngineSubsystem( EntityManager )->RemoveComponent< T >( this );
}
