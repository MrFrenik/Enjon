
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

	const UIStyleConfiguration& UIElement::GetCurrentStyleConfiguration( ) const
	{
		switch ( mState )
		{
			default:
			case UIStyleState::Default: 
			{ 
				return mStyleConfiguration;
			} break;
			case UIStyleState::Hovered: 
			{ 
				return mHoverStyleConfiguration;
			} break;
			case UIStyleState::Active: 
			{ 
				return mActiveStyleConfiguration;
			} break;
		}

	} 

	#define ColorRGBA8ToImVec4( color )\
		ImVec4( (f32)color.r / 255.f, (f32)color.g / 255.f, (f32)color.b / 255.f, (f32)color.a / 255.f )

	//=================================================================================
	void UIElementText::OnUI()
	{ 
		// Push font at particular size for text
		ImGuiManager* igm = EngineSubsystem( ImGuiManager ); 

		Vec2 pos = GetCalculatedLayoutPosition( );
		Vec2 sz = GetCalculatedLayoutSize( );
		ImVec2 textSz = ImGui::CalcTextSize( mText.c_str( ) );
		ImVec2 cp; 
		const UIStyleConfiguration& config = GetCurrentStyleConfiguration( );

		ImDrawList* dl = ImGui::GetWindowDrawList( );

		// Draw color rect behind text
		dl->AddRectFilled( ImVec2( pos.x, pos.y ), ImVec2( pos.x + sz.x, pos.y + sz.y ), ImColor( ColorRGBA8ToImVec4( config.mBackgroundColor ) ) );

		// Push font
		igm->PushFont( config.mFont, config.mFontSize );

		// Calculate position based on alignment of text within box
		switch ( config.mTextJustification )
		{
			case UIElementJustification::JustifyCenter:		cp.x = pos.x + ( sz.x - textSz.x ) / 2.f;	break; 
			case UIElementJustification::JustifyFlexStart:	cp.x = pos.x;								break; 
			case UIElementJustification::JustifyFlexEnd:	cp.x = pos.x + ( sz.x - textSz.x );			break;
		}

		switch ( config.mTextAlignment )
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
		ImGui::TextColored( ColorRGBA8ToImVec4( config.mTextColor ), "%s", mText.c_str() );
		ImGui::PopID();

		igm->PopFont();
	} 

	//=================================================================================

#define ColorRGBA8ToImVec4( color )\
	ImVec4( (f32)color.r / 255.f, (f32)color.g / 255.f, (f32)color.b / 255.f, (f32)color.a / 255.f )

	ImVec2 TextAlignJustifyValue( const UIElementJustification& justification, const UIElementAlignment& alignment )
	{ 
		ImVec2 val = ImVec2( 0.f, 0.f );
		switch ( justification )
		{
			case UIElementJustification::JustifyCenter: { 
				val.x = 0.5f;
			} break;
			case UIElementJustification::JustifyFlexStart: { 
				val.x = 0.0f;
			} break;
			case UIElementJustification::JustifyFlexEnd: { 
				val.x = 1.f;
			} break;
		}
		switch ( alignment )
		{
			case UIElementAlignment::AlignCenter: {
				val.y = 0.5f;
			} break;
			case UIElementAlignment::AlignFlexStart: {
				val.y = 0.0f;
			} break;
			case UIElementAlignment::AlignFlexEnd: {
				val.y = 1.0f;
			} break;
		}

		return val;
	}

	void UIElementButton::OnUI()
	{ 
		// Push font at particular size for text
		ImGuiManager* igm = EngineSubsystem( ImGuiManager ); 
		const UIStyleConfiguration& config = GetCurrentStyleConfiguration( );

		igm->PushFont( config.mFont, config.mFontSize );

		// Wait, how would this actually work?... 
		ImGui::PushStyleColor( ImGuiCol_Button, ColorRGBA8ToImVec4( mStyleConfiguration.mBackgroundColor ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ColorRGBA8ToImVec4( mHoverStyleConfiguration.mBackgroundColor ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ColorRGBA8ToImVec4( mActiveStyleConfiguration.mBackgroundColor ) );
		ImGui::PushStyleColor( ImGuiCol_Text, ColorRGBA8ToImVec4( config.mTextColor ) );
		ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 0.f );
		ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 0.f );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.f );
		ImGui::PushStyleVar( ImGuiStyleVar_ButtonTextAlign, TextAlignJustifyValue( config.mTextJustification, config.mTextAlignment ) ); 

		Vec2 pos = GetCalculatedLayoutPosition();
		Vec2 sz = GetCalculatedLayoutSize(); 
		ImGui::SetCursorScreenPos( ImVec2( pos.x, pos.y ) ); 

		ImGui::PushID( (usize )(intptr_t )this );
		if ( ImGui::InvisibleButton( std::to_string( (usize)(intptr_t)this ).c_str(), ImVec2( sz.x, sz.y ) ) )
		{
			mOnClick( this );
		}
		ImDrawList* dl = ImGui::GetWindowDrawList( );
		dl->AddRectFilled( ImVec2( pos.x, pos.y ), ImVec2( pos.x + sz.x, pos.y + sz.y ), ImColor( ColorRGBA8ToImVec4( config.mBackgroundColor ) ), 0.f );

		bool active = ImGui::IsItemActive( );
		bool hovered = ImGui::IsItemHovered( );
		mState = active ? UIStyleState::Active : hovered ? UIStyleState::Hovered : UIStyleState::Default;

		ImGui::PopID(); 

		ImGui::PopStyleColor( 4 );
		ImGui::PopStyleVar( 4 );

		igm->PopFont();
	}

	//=================================================================================

	void UIElementImage::OnUI()
	{
		Vec2 pos = GetCalculatedLayoutPosition();
		Vec2 sz = GetCalculatedLayoutSize(); 
		const UIStyleConfiguration& config = GetCurrentStyleConfiguration( );
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

	//void UIElement::SetInlineStyles( const UIStyleSettings& styles )
	//{
	//	mInlineStyles = styles; 
	//}

	//=================================================================================

	void UIElement::SetSize( const Vec2& size )
	{
		SetStyleWidth( size.x );
		SetStyleHeight( size.y );
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

		//// Force inline styles to be set
		//SetInlineStyles( mInlineStyles ); 

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

	Vec4 UIElement::CalculateChildBounds( const AssetHandle< UI >& ui )
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
					// Calculate style of the given element
					c->CalculateStyle( ui );

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
					c->CalculateStyle( ui );

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

	void UIElement::CalculateLayout( const AssetHandle< UI >& ui )
	{
		if ( mParent == nullptr )
		{
			CalculateStyle( ui );
			mCalculatedLayoutSize = Vec2( GetStyleWidth( ), GetStyleHeight( ) );
		}

		mChildBounds = CalculateChildBounds( ui );

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

					c->CalculateLayout( ui );

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

					c->CalculateLayout( ui );

					// Increment cp in column direction
					cp.y += c->mCalculatedLayoutSize.y + margin.y + margin.w;
				}

			} break;
		}
	}

	//=================================================================================

	void UIElement::CalculateStyle( const AssetHandle<UI>& uiAsset )
	{
		const UI* ui = uiAsset.Get( );
		if ( !ui ) {
			return;
		}

		AssetHandle< UIStyleSheet > styleSheet = ui->GetStyleSheet();
		const UIStyleSheet* ss = styleSheet.Get( );
		if ( !ss ) {
			return;
		}

		// Get tag for this particular element type ( just the meta class name )
		String tag = Class( )->GetName( );

		//switch( mState )
		{
			// Get default style configuration for this element type
			UIStyleConfiguration config = UIStyleConfiguration::GetDefaultStyleConfiguration( ); 

			const UIStyleRule* defaultTagStyleRule = ss->GetStyleRuleFromSelector( tag, UIStyleState::Default );
			if ( defaultTagStyleRule ) {
				config.MergeRuleIntoStyle( defaultTagStyleRule );
				
			}

			//case UIStyleState::Default:
			{
				const UIStyleRule* tagStyleRule = ss->GetStyleRuleFromSelector( tag, UIStyleState::Default );
				if ( tagStyleRule ) {
					config.MergeRuleIntoStyle( tagStyleRule );
				}

				for ( auto& s : mStyleSelectors )
				{
					const UIStyleRule* classStyleRule = ss->GetStyleRuleFromSelector( s, UIStyleState::Default );
					if ( classStyleRule ) {
						config.MergeRuleIntoStyle( classStyleRule );
					}
				}

				// Then Id
				const UIStyleRule* idStyleRule = ss->GetStyleRuleFromSelector( mID, UIStyleState::Default );
				if ( idStyleRule ) {
					config.MergeRuleIntoStyle( idStyleRule );
				}

				// Finally apply any overrides
				config.MergeRuleIntoStyle( &mStyleOverrides );

				// Set style
				mStyleConfiguration = config;
			}

			//} break;

			//case UIStyleState::Hovered:
			{
				// Get default style configuration for this element type
				UIStyleConfiguration hoverConfig = config;

				const UIStyleRule* tagStyleRule = ss->GetStyleRuleFromSelector( tag, UIStyleState::Hovered );
				if ( tagStyleRule ) {
					hoverConfig.MergeRuleIntoStyle( tagStyleRule );
				}

				for ( auto& s : mStyleSelectors )
				{
					const UIStyleRule* classStyleRule = ss->GetStyleRuleFromSelector( s, UIStyleState::Hovered );
					if ( classStyleRule ) {
						hoverConfig.MergeRuleIntoStyle( classStyleRule );
					}
				}

				// Then Id
				const UIStyleRule* idStyleRule = ss->GetStyleRuleFromSelector( mID, UIStyleState::Hovered );
				if ( idStyleRule ) {
					hoverConfig.MergeRuleIntoStyle( idStyleRule );
				}

				// Finally apply any overrides
				hoverConfig.MergeRuleIntoStyle( &mStyleOverrides );

				// Set style
				mHoverStyleConfiguration = hoverConfig;
			}
			//} break;

			//case UIStyleState::Focused:
			//{ 
			//} break;

			//case UIStyleState::Active:
			{
				// Get default style configuration for this element type
				UIStyleConfiguration activeConfig = config; 
				
				const UIStyleRule* tagStyleRule = ss->GetStyleRuleFromSelector( tag, UIStyleState::Active );
				if ( tagStyleRule ) {
					activeConfig.MergeRuleIntoStyle( tagStyleRule );
				}

				for ( auto& s : mStyleSelectors )
				{
					const UIStyleRule* classStyleRule = ss->GetStyleRuleFromSelector( s, UIStyleState::Active );
					if ( classStyleRule ) {
						activeConfig.MergeRuleIntoStyle( classStyleRule );
					}
				}

				// Then Id
				const UIStyleRule* idStyleRule = ss->GetStyleRuleFromSelector( mID, UIStyleState::Active );
				if ( idStyleRule ) {
					activeConfig.MergeRuleIntoStyle( idStyleRule );
				}

				// Finally apply any overrides
				activeConfig.MergeRuleIntoStyle( &mStyleOverrides );

				// Set style
				mActiveStyleConfiguration = activeConfig;
			}
			//} break;
		}
	}

	//================================================================================= 

	bool UIStyleSheet::StyleRuleExists( const String& selector )
	{
		return ( mUIStyleRules.find( selector ) != mUIStyleRules.end( ) );
	}

	//================================================================================= 

	void UIStyleSheet::AddStyleRule( const String& selector, const UIStyleRule& rule, const UIStyleState& state )
	{
		// Add new rule
		if ( !StyleRuleExists( selector ) )
		{
			mUIStyleRules[ selector ] = UIStyleRuleGroup( );
		}

		// Add rule to state
		mUIStyleRules[ selector ][ ( u32 )state ] = rule;
	}

	//================================================================================= 

	const UIStyleRuleGroup* UIStyleSheet::GetStyleGroup( const String& selector ) const
	{
		if ( mUIStyleRules.find( selector ) != mUIStyleRules.end( ) )
		{
			return &mUIStyleRules.at( selector );
		}

		return nullptr;
	}

	//================================================================================= 

	const UIStyleRule* UIStyleSheet::GetStyleRuleFromSelector( const String& selector, const UIStyleState& state ) const
	{ 
		// Get group from selector
		const UIStyleRuleGroup* sg = GetStyleGroup( selector );
		if ( !sg ) {
			return nullptr;
		}

		// Need to determine if style rule exists for given state as well	
		if ( sg->find( ( u32 )state ) != sg->end( ) )
		{
			// Don't know if this is going to just return a pointer to the local iterator instead
			return ( &sg->at( u32( state ) ) );
		}

		return nullptr; 
	}

	//================================================================================= 

	UIStyleConfiguration UIStyleConfiguration::GetDefaultStyleConfiguration( )
	{ 
		return UIStyleConfiguration( );
	}

	//================================================================================= 

#define MERGE_RULE_INTERNAL( field, type )\
	do {\
		field = *( type* )( p.mData );\
	} while ( 0 )

	void UIStyleConfiguration::MergeRuleIntoStyle( const UIStyleRule* rule )
	{
		// Iterate through all the rules, then set the style on the configuration
		for ( auto& p : rule->mStylePropertyDataSet )
		{ 
			switch ( p.mType )
			{
				case UIStylePropertyType::AlignmentContent:		MERGE_RULE_INTERNAL( mAlignContent, UIElementAlignment );			break; 
				case UIStylePropertyType::BackgroundColor:		MERGE_RULE_INTERNAL( mBackgroundColor, ColorRGBA8 );				break;
				case UIStylePropertyType::BorderColor:			MERGE_RULE_INTERNAL( mBorderColor, ColorRGBA8 );					break; 
				case UIStylePropertyType::Font:					MERGE_RULE_INTERNAL( mFont, AssetHandle< UIFont > );				break;
				case UIStylePropertyType::FontSize:				MERGE_RULE_INTERNAL( mFontSize, f32 );								break; 
				case UIStylePropertyType::TextColor:			MERGE_RULE_INTERNAL( mTextColor, ColorRGBA8 );						break;
				case UIStylePropertyType::TextAlignment:		MERGE_RULE_INTERNAL( mTextAlignment, UIElementAlignment );			break;
				case UIStylePropertyType::TextJustification:	MERGE_RULE_INTERNAL( mTextJustification, UIElementJustification );	break;
				case UIStylePropertyType::Height:				MERGE_RULE_INTERNAL( mSize.y, f32 );								break;
				case UIStylePropertyType::Width:				MERGE_RULE_INTERNAL( mSize.x, f32 );								break;
				case UIStylePropertyType::JustificationContent: MERGE_RULE_INTERNAL( mJustification, UIElementJustification );		break;
				case UIStylePropertyType::MarginLeft:			MERGE_RULE_INTERNAL( mMargin.x, f32 );								break;
				case UIStylePropertyType::MarginTop:			MERGE_RULE_INTERNAL( mMargin.y, f32 );								break;
				case UIStylePropertyType::MarginRight:			MERGE_RULE_INTERNAL( mMargin.z, f32 );								break;
				case UIStylePropertyType::MarginBottom:			MERGE_RULE_INTERNAL( mMargin.w, f32 );								break;
				case UIStylePropertyType::PaddingLeft:			MERGE_RULE_INTERNAL( mPadding.x, f32 );								break;
				case UIStylePropertyType::PaddingTop:			MERGE_RULE_INTERNAL( mPadding.y, f32 );								break;
				case UIStylePropertyType::PaddingRight:			MERGE_RULE_INTERNAL( mPadding.z, f32 );								break;
				case UIStylePropertyType::PaddingBottom:		MERGE_RULE_INTERNAL( mPadding.w, f32 );								break;
				case UIStylePropertyType::PositionType:			MERGE_RULE_INTERNAL( mPositionType, UIElementPositionType );		break;
				case UIStylePropertyType::FlexGrow:				MERGE_RULE_INTERNAL( mFlexGrow, f32 );								break;
				case UIStylePropertyType::FlexShrink:			MERGE_RULE_INTERNAL( mFlexShrink, f32 );							break;
				case UIStylePropertyType::FlexDirection:		MERGE_RULE_INTERNAL( mFlexDirection, UIElementFlexDirection );		break;
				case UIStylePropertyType::BorderRadiusTL:		MERGE_RULE_INTERNAL( mBorderRadius.x, f32 );						break;
				case UIStylePropertyType::BorderRadiusTR:		MERGE_RULE_INTERNAL( mBorderRadius.y, f32 );						break;
				case UIStylePropertyType::BorderRadiusBR:		MERGE_RULE_INTERNAL( mBorderRadius.z, f32 );						break;
				case UIStylePropertyType::BorderRadiusBL:		MERGE_RULE_INTERNAL( mBorderRadius.w, f32 );						break;
				case UIStylePropertyType::AnchorLeft:			MERGE_RULE_INTERNAL( mPosition.x, f32 );							break;
				case UIStylePropertyType::AnchorTop:			MERGE_RULE_INTERNAL( mPosition.y, f32 );							break;
				case UIStylePropertyType::AnchorRight:			MERGE_RULE_INTERNAL( mPosition.z, f32 );							break;
				case UIStylePropertyType::AnchorBottom:			MERGE_RULE_INTERNAL( mPosition.w, f32 );							break;
			}
		}
	}

	//================================================================================= 

	AssetHandle< UIStyleSheet > UI::GetStyleSheet( ) const
	{
		return mStyleSheet;
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
		// I want to set the override style for this...
		mRoot.SetStyleWidth( width );
		mRoot.SetStyleHeight( height ); 
		mRoot.CalculateLayout( this );
	}

	//=================================================================================

	void UI::ExplicitConstructor( )
	{
		mStyleSheet = EngineSubsystem( AssetManager )->GetDefaultAsset< UIStyleSheet >( );
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
}















