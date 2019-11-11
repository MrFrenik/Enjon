#include "GUI/UIAsset.h"
#include "Serialize/ObjectArchiver.h"
#include "ImGui/imgui.h"
#include "Engine.h"
#include "Subsystem.h"
#include "SubsystemCatalog.h"
#include "ImGui/ImGuiManager.h"
#include "Engine.h"
#include "Subsystem.h"
#include "SubsystemCatalog.h"
#include "Asset/AssetManager.h"
#include "Serialize/AssetArchiver.h"
#include "Serialize/BaseTypeSerializeMethods.h"

namespace Enjon
{ 
	//=================================================================================

	Result UI::SerializeData( ByteBuffer* buffer ) const
	{ 
		// Write out root canvas to buffer
		ObjectArchiver::Serialize( &mRoot, buffer ); 
		return Result::INCOMPLETE;
	}

	//=================================================================================
		
	Result UI::DeserializeData( ByteBuffer* buffer )
	{ 
		// Read in canvas from buffer
		ObjectArchiver::Deserialize( buffer, &mRoot );
		return Result::INCOMPLETE;
	} 

	//=================================================================================
	
	void UI::OnUI() const
	{
		// Load ui style, if possible
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		igm->LoadStyle( mStyleConfig );
		const_cast< UI* >( this )->mRoot.OnUI();
	}

	//=================================================================================

	UIElement* UI::FindElement( const char* label ) const
	{
		if ( strcmp( label, mRoot.mID.c_str() )  == 0 )
		{
			return (UIElement*)&mRoot;
		}

		// Not recursive for now...
		for ( auto& c : mRoot.mChildren )
		{
			if ( strcmp( c->mID.c_str(), label ) == 0 )
			{
				return c;
			}
		}

		return nullptr;
	}

	//=================================================================================


	void UIElementText::OnUI()
	{
		Vec2 pos = GetCalculatedLayoutPosition( );
		Vec2 sz = GetCalculatedLayoutSize( );
		ImVec2 textSz = ImGui::CalcTextSize( mText.c_str( ) );

		ImVec2 cp;

		// Calculate position based on alignment of text within box
		switch ( mTextJustification )
		{
			case UIElementJustification::JustifyCenter:		cp.x = pos.x + ( sz.x - textSz.x ) / 2.f;	break; 
			case UIElementJustification::JustifyFlexStart:	cp.x = pos.x;								break; 
			case UIElementJustification::JustifyFlexEnd:	cp.x = pos.x + ( sz.x - textSz.x );			break;
		}

		switch ( mTextAlignment )
		{ 
			case UIElementAlignment::AlignCenter:		cp.y = pos.y + ( sz.y - textSz.y ) / 2.f;		break; 
			case UIElementAlignment::AlignFlexStart:	cp.y = pos.y;									break;
			case UIElementAlignment::AlignFlexEnd:		cp.y = pos.y + ( sz.y - textSz.y );				break;
		}

		ImGui::SetCursorScreenPos( cp );
		ImGui::PushID( (usize)(intptr_t)this );
		{
			mOnSetText( this );
		}
		ImGui::Text( "%s", mText.c_str() );
		ImGui::PopID();
	}
	
	//=================================================================================

	void UIElementButton::OnUI()
	{ 
		Vec2 pos = GetCalculatedLayoutPosition();
		Vec2 sz = GetCalculatedLayoutSize(); 
		ImGui::SetCursorScreenPos( ImVec2( pos.x, pos.y ) ); 
		ImGui::PushID( (usize )(intptr_t )this );
		if ( ImGui::Button( mText.c_str(), ImVec2( sz.x, sz.y ) ) )
		{
			mOnClick( this );
		}
		ImGui::PopID(); 
	}

	//=================================================================================

	void UIElementImage::OnUI()
	{
		Vec2 pos = GetCalculatedLayoutPosition();
		Vec2 sz = GetCalculatedLayoutSize(); 
		ImGui::SetCursorScreenPos( ImVec2( pos.x, pos.y ) );
		ImGui::PushID( ( usize )( intptr_t )this );
		ImTextureID img = mImage ? ( ImTextureID )Int2VoidP( mImage->GetTextureId( ) ) : ( ImTextureID )Int2VoidP( EngineSubsystem( AssetManager )->GetDefaultAsset< Texture >( )->GetTextureId( ) );
		ImGui::Image( img, ImVec2( sz.x, sz.y ), ImVec2( mUV0.x, mUV0.y ), ImVec2( mUV1.x, mUV1.y ) );
		ImGui::PopID();
	}

	//=================================================================================

	void UIElementCanvas::OnUI()
	{
		Vec2 pos = GetCalculatedLayoutPosition();
		Vec2 sz = GetCalculatedLayoutSize();
		ImGui::SetNextWindowPos( ImVec2( pos.x, pos.y ) );
		ImGui::SetNextWindowSize( ImVec2( sz.x, sz.y ) ); 
		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.f, 0.f, 0.f, 0.f ) );
		ImGui::Begin( mID.c_str(), nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoBringToFrontOnFocus | 
			ImGuiWindowFlags_NoScrollbar | 
			ImGuiWindowFlags_NoScrollWithMouse
		);
		{
			// Do children
			for ( auto& c : mChildren )
			{
				c->OnUI();
			}
		}
		ImGui::End();
		ImGui::PopStyleColor();
	}

	//=================================================================================

	void UIElement::ExplicitConstructor()
	{
	}

	//=================================================================================

	void UIElement::ExplicitDestructor()
	{ 
		for ( auto& c : mChildren )
		{
			RemoveChild( c );
		}

		RemoveFromParent( );

		mChildren.clear( );
	}

	//================================================================================= 

	void UIElement::SetInlineStyles( const UIStyleSettings& styles )
	{
		mInlineStyles = styles; 
	}

	//=================================================================================

	void UIElement::SetSize( const Vec2& size )
	{
		mInlineStyles.mSize = size; 
	}

	//=================================================================================

	Result UIElement::SerializeData( ByteBuffer* buffer ) const
	{
		// Don't serialize children by default? Construct new child instead?  
		// Do default serialization of this node, then serialize yoga node, then serialize children data

		// Serialize default object data
		ObjectArchiver::SerializeObjectDataDefault( this, this->Class(), buffer );

		// Serialize specific yoga node data ( for now, nothing, until we get a better handle of how this will go ) 

		// Child count
		buffer->Write< u32 >( mChildren.size() );

		// Serialize children
		for ( auto& c : mChildren )
		{
			ObjectArchiver::Serialize( c, buffer );
		}

		return Result::SUCCESS;
	}

	//=================================================================================

	Result UIElement::DeserializeData( ByteBuffer* buffer )
	{
		// Deserialize all properties first
		ObjectArchiver::DeserializeObjectDataDefault( this, this->Class(), buffer ); 

		// Read in child count
		u32 childCount = buffer->Read< u32 >(); 
		mChildren.resize( childCount );
		for ( u32 i = 0; i < childCount; ++i )
		{
			UIElement* child = (UIElement*)ObjectArchiver::Deserialize( buffer );

			// Set element's parent
			child->mParent = this;

			// Add child to list
			mChildren.at( i ) = child; 
		} 

		// Force inline styles to be set
		SetInlineStyles( mInlineStyles ); 

		return Result::SUCCESS;
	}

	//=================================================================================

	UIElement* UIElement::AddChild( UIElement* element )
	{
		mChildren.push_back( element ); 
		element->mParent = this;

		return element;
	}

	//=================================================================================
	
	UIElement* UIElement::RemoveChild( UIElement* element )
	{ 
		if ( !element || element->mParent != this )
		{
			return element;
		}

		auto it = std::find( mChildren.begin( ), mChildren.end( ), element );
		mChildren.erase( it ); 

		// Set to this element's parent
		element->mParent = mParent; 

		return element;
	}

	//=================================================================================

	void UIElement::RemoveFromParent( )
	{
		if ( mParent )
		{
			mParent->RemoveChild( this );
		} 
	}

	//=================================================================================

	Vec2 UIElement::GetCalculatedLayoutPosition()
	{
		return mCalculatedLayoutPosition;
	}

	//=================================================================================

	Vec2 UIElement::GetCalculatedLayoutSize()
	{
		return mCalculatedLayoutSize;
	}

	//=================================================================================

	Vec4 UIElement::GetCalculatedLayoutRect()
	{ 
		Vec2 pos = GetCalculatedLayoutPosition( );
		Vec2 sz = GetCalculatedLayoutSize( );
		return Vec4( pos, pos + sz );
	}

	//=================================================================================

	Vec4 UIElement::CalculateChildBounds( )
	{
		Vec4 bounds = Vec4( 0.f );

		// This depends on how everything grows... want to know max width and max height
		switch ( GetStyleFlexDirection( ) )
		{
			case UIElementFlexDirection::FlexDirectionRow:
			{
				// Bounds are total width with max height
				f32 maxHeight = 0.f;
				for ( auto& c : mChildren )
				{
					Vec4 padding = c->GetStylePadding( );
					Vec4 margin = c->GetStyleMargin( );
					f32 width = c->GetStyleWidth( );
					f32 height = c->GetStyleHeight( );

					maxHeight = height > maxHeight ? height : maxHeight;

					bounds.z += width + margin.x + margin.z;
				}

				bounds.w = maxHeight;

				// Set bounds position, based on justification and alignment ( set main axis alignment, which in this case is X axis )
				switch ( GetStyleJustification( ) )
				{
					case UIElementJustification::JustifyCenter:
					{
						bounds.x = mCalculatedLayoutPosition.x + ( mCalculatedLayoutSize.x - bounds.z ) / 2.f;
					} break;
					case UIElementJustification::JustifyFlexStart:
					{
						bounds.x = mCalculatedLayoutPosition.x;
					} break;
					case UIElementJustification::JustifyFlexEnd:
					{
						bounds.x = mCalculatedLayoutPosition.x + mCalculatedLayoutSize.x - bounds.z;
					} break;
				}

				switch ( GetStyleAlignContent( ) )
				{
					case UIElementAlignment::AlignCenter:
					{
						bounds.y = mCalculatedLayoutPosition.y + ( mCalculatedLayoutSize.y - bounds.w ) / 2.f;
					} break;
					case UIElementAlignment::AlignFlexStart:
					{
						bounds.y = mCalculatedLayoutPosition.y;
					} break;
					case UIElementAlignment::AlignFlexEnd:
					{
						bounds.y = mCalculatedLayoutPosition.y + ( mCalculatedLayoutSize.y - bounds.w );
					} break;
				} 
			} break;

			case UIElementFlexDirection::FlexDirectionColumn:
			{
				f32 maxWidth = 0.f;
				for ( auto& c : mChildren )
				{
					Vec4 padding = c->GetStylePadding( );
					Vec4 margin = c->GetStyleMargin( );
					f32 width = c->GetStyleWidth( );
					f32 height = c->GetStyleHeight( );

					maxWidth = width > maxWidth ? width : maxWidth;

					bounds.w += height + margin.y + margin.w;
				}

				bounds.z = maxWidth;

				// Set bounds position, based on justification and alignment ( set main axis alignment, which in this case is Y axis )
				switch ( GetStyleJustification( ) )
				{
					case UIElementJustification::JustifyCenter:
					{
						bounds.y = mCalculatedLayoutPosition.y + ( mCalculatedLayoutSize.y - bounds.w ) / 2.f;
					} break;
					case UIElementJustification::JustifyFlexStart:
					{
						bounds.y = mCalculatedLayoutPosition.y;
					} break;
					case UIElementJustification::JustifyFlexEnd:
					{
						bounds.y = mCalculatedLayoutPosition.y + ( mCalculatedLayoutSize.y - bounds.w );
					} break;
				}

				switch ( GetStyleAlignContent( ) )
				{
					case UIElementAlignment::AlignCenter:
					{
						bounds.x = mCalculatedLayoutPosition.x + ( mCalculatedLayoutSize.x - bounds.z ) / 2.f;
					} break;
					case UIElementAlignment::AlignFlexStart:
					{
						bounds.x = mCalculatedLayoutPosition.x;
					} break;
					case UIElementAlignment::AlignFlexEnd:
					{
						bounds.x = mCalculatedLayoutPosition.x + ( mCalculatedLayoutSize.x - bounds.z );
					} break;
				}

			} break;
		}

		return bounds;
	}

	//=================================================================================

	void UIElement::CalculateLayout( )
	{
		if ( mParent == nullptr )
		{
			mCalculatedLayoutSize = Vec2( GetStyleWidth( ), GetStyleHeight( ) );
		}

		mChildBounds = CalculateChildBounds( );

		switch ( GetStyleFlexDirection( ) )
		{
			case UIElementFlexDirection::FlexDirectionRow:
			{
				// Cursor position is the bounds starting position
				Vec2 cp = Vec2( mChildBounds.x, mChildBounds.y );

				// Want to set the calculated layout position and size for each child now
				for ( auto& c : mChildren )
				{
					Vec4 padding = c->GetStylePadding( );
					Vec4 margin = c->GetStyleMargin( );
					f32 width = c->GetStyleWidth( );
					f32 height = c->GetStyleHeight( );

					c->mCalculatedLayoutSize = Vec2( width, height );

					// Position element based on alignment and justification rules of parent element
					c->mCalculatedLayoutPosition.x = cp.x + margin.x;

					switch ( GetStyleAlignContent( ) )
					{
						case UIElementAlignment::AlignCenter:
						{
							c->mCalculatedLayoutPosition.y = cp.y + ( mChildBounds.w - c->mCalculatedLayoutSize.y ) / 2.f;
						} break;
						case UIElementAlignment::AlignFlexStart:
						{
							c->mCalculatedLayoutPosition.y = cp.y;
						} break;
						case UIElementAlignment::AlignFlexEnd:
						{
							c->mCalculatedLayoutPosition.y = cp.y + ( mChildBounds.w - c->mCalculatedLayoutSize.y );
						} break;
					}

					c->CalculateLayout( );

					// Increment cp in row direction
					cp.x += c->mCalculatedLayoutSize.x + margin.x + margin.z;
				}

			} break;

			case UIElementFlexDirection::FlexDirectionColumn:
			{
				Vec2 cp = Vec2( mChildBounds.x, mChildBounds.y );

				for ( auto& c : mChildren )
				{
					Vec4 padding = c->GetStylePadding( );
					Vec4 margin = c->GetStyleMargin( );
					f32 width = c->GetStyleWidth( );
					f32 height = c->GetStyleHeight( );

					c->mCalculatedLayoutSize = Vec2( width, height );

					// Position element based on alignment and justification rules of parent element
					c->mCalculatedLayoutPosition.y = cp.y + margin.y;

					switch ( GetStyleAlignContent( ) )
					{
						case UIElementAlignment::AlignCenter:
						{
							c->mCalculatedLayoutPosition.x = cp.x + ( mChildBounds.z - c->mCalculatedLayoutSize.x ) / 2.f;
						} break;
						case UIElementAlignment::AlignFlexStart:
						{
							c->mCalculatedLayoutPosition.x = cp.x;
						} break;
						case UIElementAlignment::AlignFlexEnd:
						{
							c->mCalculatedLayoutPosition.x = cp.x + ( mChildBounds.z - c->mCalculatedLayoutSize.x );
						} break;
					}

					c->CalculateLayout( );

					// Increment cp in column direction
					cp.y += c->mCalculatedLayoutSize.y + margin.y + margin.w;
				}

			} break;
		}
	}

	//=================================================================================


	Result UI::OnEditorUI()
	{
		// Show the asset handle property
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		// Style config
		{
			const MetaProperty* prop = Class()->GetPropertyByName( ENJON_TO_STRING( mStyleConfig ) );
			if ( prop ) {
				igm->DebugDumpProperty( this, prop );
			} 
		}
		// Style sheet
		{
			const MetaProperty* prop = Class()->GetPropertyByName( ENJON_TO_STRING( mStyleSheet ) );
			if ( prop ) {
				igm->DebugDumpProperty( this, prop );
			} 
		}

		return Result::SUCCESS;
	}

	//=================================================================================

	void UI::CalculateLayout( const u32& width, const u32& height )
	{
		mRoot.SetStyleWidth( width );
		mRoot.SetStyleHeight( height ); 

		mRoot.CalculateLayout( );
	}

	//=================================================================================

	void UI::ExplicitDestructor( )
	{
		// Destroy all children, release children
		u32 c = mRoot.mChildren.size( );
		while ( c )
		{
			delete( mRoot.mChildren.back( ) );
			c = mRoot.mChildren.size( );
		}
	}

	//=================================================================================
}















