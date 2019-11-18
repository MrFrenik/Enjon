#include "Asset/UIStyleConfigAssetLoader.h"
#include "Asset/AssetManager.h"
#include "Graphics/Font.h"
#include "Serialize/ObjectArchiver.h"
#include "Engine.h"
#include "Subsystem.h"
#include "SubsystemCatalog.h"

namespace Enjon
{
	//=======================================================================

	void UIStyleConfiguration::ExplicitConstructor( )
	{
		mFont = EngineSubsystem( AssetManager )->GetDefaultAsset< UIFont >( );
	}

	//=======================================================================
	
	String UIStyleConfigAssetLoader::GetAssetFileExtension() const
	{
		return ".euicfg";
	}

	//=======================================================================
	
	void UIStyleConfigAssetLoader::RegisterDefaultAsset()
	{ 
		// Create new graph and compile
		UIStyleConfig* styleConfig = new UIStyleConfig( );
		styleConfig->mName = "DefaultUIStyleConfig";

		// Set default asset
		mDefaultAsset = styleConfig; 
	}

	//=======================================================================

	Asset* UIStyleConfigAssetLoader::LoadResourceFromFile( const String& filePath )
	{
		// Just return the default material asset
		if ( !mDefaultAsset )
		{
			RegisterDefaultAsset( );
		}

		return mDefaultAsset;
	}

	//======================================================================= 

	String UIStyleSheetAssetLoader::GetAssetFileExtension() const
	{
		return ".ess";
	}

	//======================================================================= 
			
	void UIStyleSheetAssetLoader::RegisterDefaultAsset()
	{
		// Create new graph and compile
		UIStyleSheet* styleSheet = new UIStyleSheet( );
		styleSheet->mName = "DefaultStyleSheet";

		// Set default asset
		mDefaultAsset = styleSheet; 
	}

	//======================================================================= 

	Asset* UIStyleSheetAssetLoader::LoadResourceFromFile( const String& filePath )
	{
		// Just return the default material asset
		if ( !mDefaultAsset )
		{
			RegisterDefaultAsset( );
		}

		return mDefaultAsset; 
	}

	//======================================================================= 

	Result UIStyleSheet::SerializeData( ByteBuffer* buffer ) const
	{ 
		// Serialize out the style rules
		buffer->Write< u32 >( mUIStyleRules.size( ) );
		for ( auto& rg : mUIStyleRules )
		{
			// Name of style rule group
			buffer->Write< String >( rg.first );

			// Number of states in style rule group
			buffer->Write< u32 >( rg.second.size( ) );

			// For each rule in group
			for ( auto& r : rg.second )
			{
				// Write out state this rule is associated with
				buffer->Write< u32 >( r.first );
				// Write out the rule itself
				ObjectArchiver::Serialize( &r.second, buffer ); 
			}
		}

		return Result::SUCCESS;
	}

	//======================================================================= 

	Result UIStyleSheet::DeserializeData( ByteBuffer* buffer )
	{ 
		// Serialize out the style rules
		u32 count = buffer->Read< u32 >();
		for ( u32 i = 0; i < count; ++i )
		{
			// Name of style rule group
			String name = buffer->Read< String >(); 

			// Number of states in style rule group
			u32 numStates = buffer->Read< u32 >();

			for ( u32 s = 0; s < numStates; ++s )
			{
				UIStyleState state = ( UIStyleState )buffer->Read< u32 >( );
				UIStyleRule rule;
				ObjectArchiver::Deserialize( buffer, &rule );

				// Add rule after deserializing
				AddStyleRule( name, rule, state );
			} 
		}

		return Result::SUCCESS;
	}

	//======================================================================= 

#define UI_STYLE_RULE_WRITE_DATA( type, data, buffer )\
	buffer->Write< type >( *( type *)data );

	Result UIStyleRule::SerializeData( ByteBuffer* buffer ) const
	{
		// For each property, write out data
		buffer->Write< u32 >( mStylePropertyDataSet.size( ) );

		for ( auto& p : mStylePropertyDataSet )
		{
			// Write out type of data
			buffer->Write< u32 >( ( u32 )p.mType );

			// Write out data
			switch ( p.mType )
			{ 
				case UIStylePropertyType::AlignmentContent:	
				case UIStylePropertyType::AlignmentSelf:
				case UIStylePropertyType::TextAlignment:
				case UIStylePropertyType::TextJustification:
				{
					UI_STYLE_RULE_WRITE_DATA( u32, p.mData, buffer );
				} break;

				case UIStylePropertyType::FontSize:
				case UIStylePropertyType::PaddingLeft:
				case UIStylePropertyType::PaddingTop:
				case UIStylePropertyType::PaddingRight:
				case UIStylePropertyType::PaddingBottom:
				case UIStylePropertyType::MarginLeft:
				case UIStylePropertyType::MarginTop:
				case UIStylePropertyType::MarginRight:
				case UIStylePropertyType::MarginBottom:
				case UIStylePropertyType::BorderLeft:
				case UIStylePropertyType::BorderTop:
				case UIStylePropertyType::BorderRight:
				case UIStylePropertyType::BorderBottom:
				case UIStylePropertyType::BorderRadiusTL:
				case UIStylePropertyType::BorderRadiusTR:
				case UIStylePropertyType::BorderRadiusBR:
				case UIStylePropertyType::BorderRadiusBL:
				case UIStylePropertyType::JustificationSelf:
				case UIStylePropertyType::FlexGrow:
				case UIStylePropertyType::FlexShrink:
				case UIStylePropertyType::FlexDirection:
				case UIStylePropertyType::PositionType:
				case UIStylePropertyType::AnchorLeft:
				case UIStylePropertyType::AnchorTop:
				case UIStylePropertyType::AnchorRight:
				case UIStylePropertyType::AnchorBottom:
				case UIStylePropertyType::Width:
				case UIStylePropertyType::Height:
				{
					UI_STYLE_RULE_WRITE_DATA( f32, p.mData, buffer );
				} break;

				case UIStylePropertyType::TextColor:
				case UIStylePropertyType::BorderColor:
				case UIStylePropertyType::BackgroundColor:
				{
					ColorRGBA8 col = *( ColorRGBA8* )p.mData;
					buffer->Write< u8 >( col.r );
					buffer->Write< u8 >( col.g );
					buffer->Write< u8 >( col.b );
					buffer->Write< u8 >( col.a );
				} break;

				// This needs to be handled specially
				case UIStylePropertyType::Font:
				{ 
					// Write out uuid of font
					AssetHandle< UIFont > font = ( UIFont* )p.mData;
					buffer->Write< UUID >( font->GetUUID( ) );
				} break;
			}
		}

		return Result::SUCCESS;
	}

	//======================================================================= 

	Result UIStyleRule::DeserializeData( ByteBuffer* buffer )
	{
		// For each property, write out data
		u32 count = buffer->Read< u32 >();

		// Resize to appropriate count
		mStylePropertyDataSet.resize( count );

		for ( u32 i = 0; i < count; ++i )
		{
			UIStylePropertyDataVariant variant;

			// Write out type of data
			variant.mType = ( UIStylePropertyType )buffer->Read< u32 >();

			// Write out data
			switch ( variant.mType )
			{ 
				case UIStylePropertyType::AlignmentContent:	
				case UIStylePropertyType::AlignmentSelf:
				case UIStylePropertyType::TextAlignment:
				case UIStylePropertyType::TextJustification:
				{
					*( u32* )variant.mData = buffer->Read< u32 >( );
				} break;

				case UIStylePropertyType::FontSize:
				case UIStylePropertyType::PaddingLeft:
				case UIStylePropertyType::PaddingTop:
				case UIStylePropertyType::PaddingRight:
				case UIStylePropertyType::PaddingBottom:
				case UIStylePropertyType::MarginLeft:
				case UIStylePropertyType::MarginTop:
				case UIStylePropertyType::MarginRight:
				case UIStylePropertyType::MarginBottom:
				case UIStylePropertyType::BorderLeft:
				case UIStylePropertyType::BorderTop:
				case UIStylePropertyType::BorderRight:
				case UIStylePropertyType::BorderBottom:
				case UIStylePropertyType::BorderRadiusTL:
				case UIStylePropertyType::BorderRadiusTR:
				case UIStylePropertyType::BorderRadiusBR:
				case UIStylePropertyType::BorderRadiusBL:
				case UIStylePropertyType::JustificationSelf:
				case UIStylePropertyType::FlexGrow:
				case UIStylePropertyType::FlexShrink:
				case UIStylePropertyType::FlexDirection:
				case UIStylePropertyType::PositionType:
				case UIStylePropertyType::AnchorLeft:
				case UIStylePropertyType::AnchorTop:
				case UIStylePropertyType::AnchorRight:
				case UIStylePropertyType::AnchorBottom:
				case UIStylePropertyType::Width:
				case UIStylePropertyType::Height:
				{
					*( f32* )variant.mData = buffer->Read< f32 >( );
				} break;

				case UIStylePropertyType::TextColor:
				case UIStylePropertyType::BorderColor:
				case UIStylePropertyType::BackgroundColor:
				{
					ColorRGBA8 col;
					col.r = buffer->Read< u8 >( );
					col.g = buffer->Read< u8 >( );
					col.b = buffer->Read< u8 >( );
					col.a = buffer->Read< u8 >( );
					*( ColorRGBA8* )variant.mData = col;
				} break;

				// This needs to be handled specially
				case UIStylePropertyType::Font:
				{ 
					UUID uuid = buffer->Read< UUID >( );
					*( AssetHandle< UIFont >* )(variant.mData) = EngineSubsystem( AssetManager )->GetAsset< UIFont >( uuid ); 
				} break;
			}

			// Assign variant to set
			mStylePropertyDataSet.at( i ) = variant;
		}

		return Result::SUCCESS;

	}

	//======================================================================= 
}

