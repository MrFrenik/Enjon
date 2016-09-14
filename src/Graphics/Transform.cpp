#include <Graphics/Transform.h>

namespace Enjon { namespace Graphics {


	Transform Transform::operator*(Transform& Parent) const
	{
		Transform WorldSpace;

		WorldSpace.Position 	= Parent.Position + Parent.Orientation * (Parent.Scale * Position); 	
		WorldSpace.Orientation 	= Parent.Orientation * Orientation;
		WorldSpace.Scale 		= Parent.Scale * (Parent.Orientation * Scale);

		return WorldSpace;
	}

	Transform operator/(Transform& World, Transform& Parent)
	{
		Transform Local;

		auto ParentConjugate = Parent.Orientation.Conjugate();

		Local.Position 		= (ParentConjugate * (World.Position - Parent.Position)) / Parent.Scale;
		Local.Orientation 	= ParentConjugate * World.Orientation;
		Local.Scale 		= ParentConjugate * (World.Scale / Parent.Scale);

		return Local;
	}

	Transform Transform::Inverse()
	{
		Transform I;

		return I / *this;
	}

}}