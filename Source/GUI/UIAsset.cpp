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
		// Just trying some ways to apply styles here ( want a unified way of doing this )
		//YGNodeStyleSetWidthAuto( mYogaNode );
		//YGNodeStyleSetFlexGrow( mYogaNode, 1 ); 
		SetInlineStyles( mInlineStyles );

		Vec2 pos = GetCalculatedLayoutPosition();
		Vec2 sz = GetCalculatedLayoutSize(); 
		ImGui::SetCursorScreenPos( ImVec2( pos.x, pos.y ) );
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
		SetInlineStyles( mInlineStyles );
		//ImVec2 cp = ImGui::GetCursorPos( );
		//ImGui::SetCursorPos( ImVec2( cp.x + mPosition.x, cp.y + mPosition.y ) ); 
		//YGNodeStyleSetWidthAuto( mYogaNode );
		//YGNodeStyleSetFlexGrow( mYogaNode, 1 ); 
		Vec2 pos = GetCalculatedLayoutPosition();
		Vec2 sz = GetCalculatedLayoutSize(); 
		ImGui::SetCursorScreenPos( ImVec2( pos.x, pos.y ) );
		ImGui::PushID( (usize )(intptr_t )this );
		if ( ImGui::Button( mID.c_str(), ImVec2( sz.x, sz.y ) ) )
		{
			mOnClick( this );
		}
		ImGui::PopID(); 
	}

	//=================================================================================

	void UIElementImage::OnUI()
	{
		SetInlineStyles( mInlineStyles );
		//ImVec2 cp = ImGui::GetCursorPos( );
		//ImGui::SetCursorPos( ImVec2( cp.x + mPosition.x, cp.y + mPosition.y ) );
		//YGNodeStyleSetWidthAuto( mYogaNode );
		//YGNodeStyleSetFlexGrow( mYogaNode, 1 ); 
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
		SetInlineStyles( mInlineStyles );

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
		mYogaNode = YGNodeNew();
	}

	//=================================================================================

	void UIElement::ExplicitDestructor()
	{ 
	}

	//================================================================================= 

	void UIElement::SetInlineStyles( const UIStyleSettings& styles )
	{
		mInlineStyles = styles;

		// Need to update sll yoga styles based on these
		YGNodeStyleSetFlexGrow( mYogaNode, mInlineStyles.mFlexGrow );
		YGNodeStyleSetFlexShrink( mYogaNode, mInlineStyles.mFlexShrink );
		YGNodeStyleSetFlexDirection( mYogaNode, (YGFlexDirection)mInlineStyles.mFlexDirection ); 

		YGNodeStyleSetJustifyContent( mYogaNode, (YGJustify)mInlineStyles.mJustification );
		YGNodeStyleSetAlignContent( mYogaNode, (YGAlign)mInlineStyles.mAlignContent );
		YGNodeStyleSetAlignItems( mYogaNode, (YGAlign)mInlineStyles.mAlignItems );
		YGNodeStyleSetAlignSelf( mYogaNode, (YGAlign)mInlineStyles.mAlignSelf ); 

		if ( mInlineStyles.mFlexGrow || mInlineStyles.mFlexShrink ) {
			YGNodeStyleSetHeightAuto( mYogaNode );
		}
	}

	//=================================================================================

	void UIElement::SetSize( const Vec2& size )
	{
		if ( mInlineStyles.mFlexGrow || mInlineStyles.mFlexShrink ) {
			mSize = size;
			YGNodeStyleSetWidth( mYogaNode, mSize.x );
			YGNodeStyleSetHeight( mYogaNode, mSize.y ); 
		}
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

		// Construct yoga node
		mYogaNode = YGNodeNew();

		// Set up random style stuff
		//YGNodeStyleSetFlexDirection( mYogaNode, YGFlexDirectionColumn );
		//YGNodeStyleSetPadding( mYogaNode, YGEdgeAll, 0 );
		//YGNodeStyleSetMargin( mYogaNode, YGEdgeAll, 0 );

		YGNodeStyleSetWidth( mYogaNode, mSize.x );
		YGNodeStyleSetHeight( mYogaNode, mSize.y );

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

			// Set up child's yoga parent 
			YGNodeInsertChild( mYogaNode, child->mYogaNode, i );
		} 

		if ( !childCount )
		{
			//YGNodeStyleSetWidthAuto( mYogaNode );
			//YGNodeStyleSetFlexGrow( mYogaNode, 1 ); 
		}

		// Force inline styles to be set
		SetInlineStyles( mInlineStyles ); 

		return Result::SUCCESS;
	}

	//=================================================================================

	UIElement* UIElement::AddChild( UIElement* element )
	{
		mChildren.push_back( element );

		// Push back child into yoga node tree
		YGNodeInsertChild( mYogaNode, element->mYogaNode, YGNodeGetChildCount( mYogaNode ) );

		return element;
	}

	//=================================================================================
	
	UIElement* UIElement::RemoveChild( UIElement* element )
	{
		// Swap and pop? Not safe to do this operation during a GUI call
		std::remove( mChildren.begin(), mChildren.end(), element );

		// Not sure how to remove this out of the tree just yet...

		return element;
	}

	//=================================================================================

	Vec2 UIElement::GetCalculatedLayoutPosition()
	{
		f32 l  = YGNodeLayoutGetLeft( mYogaNode );
		f32 t  = YGNodeLayoutGetTop( mYogaNode ); 
		return Vec2( l, t );
	}

	//=================================================================================

	Vec2 UIElement::GetCalculatedLayoutSize()
	{
		f32 w  = YGNodeLayoutGetWidth( mYogaNode );
		f32 h  = YGNodeLayoutGetHeight( mYogaNode ); 
		return Vec2( w, h );
	}

	//=================================================================================

	Vec4 UIElement::GetCalculatedLayoutRect()
	{ 
		f32 l  = YGNodeLayoutGetLeft( mYogaNode );
		f32 t  = YGNodeLayoutGetTop( mYogaNode );
		f32 r  = YGNodeLayoutGetLeft( mYogaNode ) + YGNodeLayoutGetWidth( mYogaNode );
		f32 b  = YGNodeLayoutGetTop( mYogaNode ) + YGNodeLayoutGetHeight( mYogaNode ); 
		return Vec4( l, t, r, b );
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
		// Just set direction for now of canvas ( need to serialize out styles eventually, but just want something up and running for now )
		YGNodeCalculateLayout( mRoot.mYogaNode, width, height, YGDirectionLTR );
	}

	//=================================================================================
}















