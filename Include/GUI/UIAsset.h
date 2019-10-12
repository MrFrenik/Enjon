#pragma once
#ifndef ENJON_UI_ASSET_H
#define ENJON_UI_ASSET_H 

#include "System/Types.h"
#include "Math/Maths.h"
#include "Asset/Asset.h"

namespace Enjon
{ 

#define UIBindFunction( elem_ptr, elem_type, var, func )\
	do {\
		elem_type* elem = ( elem_type* )( elem_ptr );\
		if ( elem ) {\
			elem->var = func;\
		}\
	} while ( 0 )

	ENJON_CLASS( Abstract )
	class UIElement : public Object
	{
		ENJON_CLASS_BODY( UIElement )

		public: 
			/*
			* @brief
			*/
			virtual void OnUI() = 0;

			/*
			* @brief
			*/
			static inline void DefaultFunc( UIElement* elem ) {
				// Do nothing...
			}

			ENJON_PROPERTY()
			String mLabel = "";

			ENJON_PROPERTY()
			Vec2 mPosition = Vec2( 0.f );
	}; 

	// Not sure if I like this here...need a way to be able to view this in the editor and set the function
	ENJON_CLASS( )
	class UICallback : public Object
	{
		ENJON_CLASS_BODY( UICallback )

		public:

			virtual void ExplicitConstructor() override
			{
				mCallback = UIElement::DefaultFunc; 
			}

			void operator()( UIElement* elem )
			{
				mCallback( elem );
			}

			void operator=( const std::function< void( UIElement* ) >& func )
			{
				mCallback = func;
			}

			std::function< void( UIElement* ) > mCallback;
	};

	ENJON_CLASS()
	class UIElementButton : public UIElement
	{
		ENJON_CLASS_BODY( UIElementButton )

		public: 

			/*
			* @brief
			*/
			virtual void OnUI() override;

		public:
			// Can I serialize this somehow? 
			UICallback mOnClick;
	}; 

	ENJON_CLASS( Construct )
	class UIElementCanvas : public UIElement
	{
		ENJON_CLASS_BODY( UIElementCanvas )

		public:

			/*
			* @brief
			*/
			virtual void OnUI() override;

			/*
			* @brief
			*/
			UIElement* AddChild( UIElement* element ); 

			/*
			* @brief
			*/
			UIElement* RemoveChild( UIElement* element );
		
		public:

			ENJON_PROPERTY()
			Vec2 mSize = Vec2( 10.f, 10.f );

			ENJON_PROPERTY()			
			Vector< UIElement* > mChildren;
	}; 

	ENJON_CLASS( )
	class UI : public Asset
	{ 
		ENJON_CLASS_BODY( UI ) 

		public:
			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;
			
			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override; 

			/*
			* @brief
			*/
			void OnUI() const;

			/*
			* @brief
			*/
			UIElement* FindElement( const char* label ) const;

		public: 

			// Will handle all serialization of ui manually
			ENJON_PROPERTY( NonSerializeable )
			UIElementCanvas mRoot; 
	}; 
}

#endif
