#ifndef ENJON_UI_STYLE_CONFIG_ASSET_LOADER_H
#define ENJON_UI_STYLE_CONFIG_ASSET_LOADER_H

#include "Asset/Asset.h" 
#include "Asset/AssetLoader.h"
#include "Math/Vec2.h"
#include "Graphics/Color.h"

namespace Enjon
{
	class UIStyleConfigAssetLoader;
	class UIStyleSheetAssetLoader;
	class UIFont;

	ENJON_ENUM()
	enum class UIStylePropertyType
	{
		BackgroundColor,
		TextColor,
		Font,
		FontSize,
		PaddingLeft,
		PaddingTop,
		PaddingRight,
		PaddingBottom,
		MarginLeft,
		MarginTop,
		MarginRight,
		MarginBottom,
		BorderLeft,
		BorderTop,
		BorderRight,
		BorderBottom,
		BorderColor,
		BorderRadiusTL,
		BorderRadiusTR,
		BorderRadiusBR,
		BorderRadiusBL,
		TextJustification,
		TextAlignment,
		JustificationSelf,
		AlignmentSelf,
		JustificationContent,
		AlignmentContent,
		FlexGrow,
		FlexShrink,
		FlexDirection,
		PositionType,
		AnchorLeft,
		AnchorTop,
		AnchorRight,
		AnchorBottom,
		Width,
		Height
	};

	ENJON_ENUM()
	enum class UIStyleState
	{
		Default,
		Hovered,
		Active,
		Focused
	};

	class UIStylePropertyDataVariant
	{
		public:

			UIStylePropertyDataVariant( ) = default;
			UIStylePropertyDataVariant( const UIStylePropertyType& type )
				: mType( type )
			{ 
			}

			UIStylePropertyType mType;
			u8 mData[ 8 ];
	};

	ENJON_CLASS( Construct )
	class UIStyleRule : public Object
	{
		ENJON_CLASS_BODY( UIStyleRule )

		public: 

			virtual Result SerializeData( ByteBuffer* buffer ) const override;
			virtual Result DeserializeData( ByteBuffer* buffer ) override;

			s32 StyleExists( const UIStylePropertyType& type )
			{
				s32 i = 0;
				for ( auto& s : mStylePropertyDataSet )
				{
					if ( s.mType == type )
					{
						return i;
					}
					++i;
				}

				return -1;
			}

			template < typename T >
			void AddStyle( const UIStylePropertyType& type, const T& val )
			{
				// Search styles, verify does not already exist
				// I don't want this. Just want to update if existing.
				s32 idx = StyleExists( type );
				if ( idx != -1 ) {
					UIStylePropertyDataVariant& var = mStylePropertyDataSet.at( idx );
					var.mType = type;
					*( T* )( &var.mData ) = val;
				}
				else {
					// Add to style list if does not exist
					UIStylePropertyDataVariant var;
					var.mType = type;
					*( T* )( &var.mData ) = val;
					mStylePropertyDataSet.push_back( var ); 
				} 
			}

			Vector< UIStylePropertyDataVariant > mStylePropertyDataSet;
	};

	// Mapping of UIStyleState -> UIStyleRule
	/*
		For instance: 
			.button {
				background_color: ( 255, 255, 255, 255 )
			}
			.button:active {
				background_color: ( 255, 0, 0, 255 )
			}
			.button:hovered {
				...
			}
			.button:focused { 
			} 
	*/
	using UIStyleRuleGroup = HashMap< u32, UIStyleRule >;

	ENJON_CLASS( Construct )
	class UIStyleConfig : public Asset
	{ 
		friend UIStyleConfigAssetLoader;

		ENJON_CLASS_BODY( UIStyleConfig )

		public: 
			ENJON_PROPERTY()
			Vec2 WindowTitleAlign = Vec2(0.5f, 0.41f);

			ENJON_PROPERTY()
			Vec2 ButtonTextAlign = Vec2(0.5f, 0.5f); 

			ENJON_PROPERTY()
			Vec2 WindowPadding = Vec2(10, 8);

			ENJON_PROPERTY()
			f32 WindowRounding = 0.0f;

			ENJON_PROPERTY()
			Vec2 FramePadding = Vec2(6, 4);

			ENJON_PROPERTY()
			f32 FrameRounding = 2.0f;

			ENJON_PROPERTY()
			Vec2 ItemSpacing = Vec2(8, 3);

			ENJON_PROPERTY()
			Vec2 ItemInnerSpacing = Vec2(2, 3);

			ENJON_PROPERTY()
			f32 IndentSpacing = 20.0f;

			ENJON_PROPERTY()
			f32 ScrollbarSize = 14.0f;

			ENJON_PROPERTY()
			f32 ScrollbarRounding = 0.0f;

			ENJON_PROPERTY()
			f32 GrabMinSize	= 5.0f;

			ENJON_PROPERTY()
			f32 GrabRounding = 2.0f;

			ENJON_PROPERTY()
			f32 Alpha = 1.0f;

			ENJON_PROPERTY()
			f32 FrameBorderSize	= 0.0f;

			ENJON_PROPERTY()
			f32 WindowBorderSize = 1.0f; 

			ENJON_PROPERTY()
			ColorRGBA32 Col_Text = ColorRGBA32( 1.00f, 1.00f, 1.00f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_TextDisabled = ColorRGBA32( 0.50f, 0.50f, 0.50f, 0.57f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_WindowBg = ColorRGBA32( 0.13f, 0.13f, 0.13f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ChildBg = ColorRGBA32( 0.00f, 0.00f, 0.00f, 0.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_PopupBg = ColorRGBA32( 0.12f, 0.12f, 0.12f, 1.00f );
			ENJON_PROPERTY()

			ColorRGBA32 Col_Border = ColorRGBA32( 0.12f, 0.12f, 0.12f, 0.45f );
			ENJON_PROPERTY()

			ENJON_PROPERTY()
			ColorRGBA32 Col_BorderShadow = ColorRGBA32( 0.00f, 0.00f, 0.00f, 1.00f );

			ENJON_PROPERTY() 
			ColorRGBA32 Col_FrameBg = ColorRGBA32( 0.03f, 0.03f, 0.03f, 0.39f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_FrameBgHovered = ColorRGBA32( 0.09f, 0.09f, 0.09f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_FrameBgActive = ColorRGBA32( 0.16f, 0.16f, 0.16f, 1.00f );

			ENJON_PROPERTY() 
			ColorRGBA32 Col_TitleBg = ColorRGBA32( 0.04f, 0.04f, 0.04f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_TitleBgActive = ColorRGBA32( 0.15f, 0.53f, 0.61f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_TitleBgCollapsed = ColorRGBA32( 0.00f, 0.00f, 0.00f, 0.51f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_MenuBarBg = ColorRGBA32( 0.17f, 0.17f, 0.17f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ScrollbarBg = ColorRGBA32( 0.00f, 0.00f, 0.00f, 0.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ScrollbarGrab = ColorRGBA32( 0.31f, 0.31f, 0.31f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ScrollbarGrabHovered = ColorRGBA32( 0.41f, 0.41f, 0.41f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ScrollbarGrabActive = ColorRGBA32( 0.51f, 0.51f, 0.51f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_CheckMark = ColorRGBA32( 1.00f, 1.00f, 1.00f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_SliderGrab = ColorRGBA32( 0.06f, 0.48f, 0.89f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_SliderGrabActive = ColorRGBA32( 0.07f, 0.39f, 0.71f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_Button = ColorRGBA32( 0.24f, 0.24f, 0.24f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ButtonHovered = ColorRGBA32( 0.27f, 0.27f, 0.27f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ButtonActive = ColorRGBA32( 0.17f, 0.17f, 0.17f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_Header = ColorRGBA32( 0.20f, 0.20f, 0.20f, 0.31f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_HeaderHovered = ColorRGBA32( 0.06f, 0.48f, 0.89f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_HeaderActive = ColorRGBA32( 0.06f, 0.48f, 0.89f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_Separator = ColorRGBA32( 0.29f, 0.29f, 0.29f, 0.50f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_SeparatorHovered = ColorRGBA32( 0.13f, 0.77f, 1.00f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_SeparatorActive = ColorRGBA32( 0.10f, 0.40f, 0.75f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ResizeGrip = ColorRGBA32( 0.06f, 0.48f, 0.89f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ResizeGripHovered = ColorRGBA32( 0.06f, 0.48f, 0.89f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ResizeGripActive = ColorRGBA32( 0.08f, 0.41f, 0.74f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_CloseButton = ColorRGBA32( 0.41f, 0.41f, 0.41f, 0.50f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_CloseButtonHovered = ColorRGBA32( 0.98f, 0.39f, 0.36f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_CloseButtonActive = ColorRGBA32( 0.98f, 0.39f, 0.36f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_PlotLines = ColorRGBA32( 0.61f, 0.61f, 0.61f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_PlotLinesHovered = ColorRGBA32( 1.00f, 0.43f, 0.35f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_PlotHistogram = ColorRGBA32( 0.90f, 0.70f, 0.00f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_PlotHistogramHovered = ColorRGBA32( 1.00f, 0.60f, 0.00f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_TextSelectedBg = ColorRGBA32( 0.29f, 0.43f, 0.58f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ModalWindowDarkening = ColorRGBA32( 0.00f, 0.00f, 0.00f, 0.80f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_DragDropTarget = ColorRGBA32( 1.00f, 1.00f, 0.00f, 0.90f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_NavHighlight = ColorRGBA32( 0.26f, 0.59f, 0.98f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_NavWindowingHighlight = ColorRGBA32( 1.00f, 1.00f, 1.00f, 0.70f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_SelectableHovered = ColorRGBA32( 0.14f, 0.23f, 0.32f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_SelectableActive = ColorRGBA32( 0.06f, 0.48f, 0.89f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_Selectable = ColorRGBA32( 0.06f, 0.48f, 0.89f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ComboBox = ColorRGBA32( 0.14f, 0.14f, 0.14f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ComboBoxHovered = ColorRGBA32( 0.09f, 0.09f, 0.09f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ListSelection = ColorRGBA32( 0.06f, 0.48f, 0.89f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ListSelectionHovered = ColorRGBA32( 0.88f, 0.40f, 0.18f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ListSelectionActive = ColorRGBA32( 0.88f, 0.40f, 0.18f, 1.00f );

			ENJON_PROPERTY()
			ColorRGBA32 Col_ListSelectionRenamed = ColorRGBA32( 0.88f, 0.40f, 0.18f, 1.00f ); 


	}; 

	ENJON_CLASS( )
	class UIStyleConfigAssetLoader : public AssetLoader
	{ 
		ENJON_CLASS_BODY( UIStyleConfigAssetLoader )

		public: 

			/**
			* @brief
			*/
			virtual String GetAssetFileExtension( ) const override;
			
		protected:
			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( ) override; 

		private: 
			/**
			* @brief
			*/
			virtual Asset* LoadResourceFromFile( const String& filePath ) override; 
	}; 

	ENJON_ENUM()
	enum class UIElementEdge
	{
		EdgeLeft,
		EdgeTop,
		EdgeRight,
		EdgeBottom,
		EdgeStart,
		EdgeEnd,
		EdgeHorizontal,
		EdgeVertical,
		EdgeAll 
	};

	ENJON_ENUM()
	enum class UIElementDirection
	{
		DirectionInherit,
		DirectionLTR,
		DirectionRTL 
	};

	ENJON_ENUM()
	enum class UIElementFlexDirection
	{
		FlexDirectionColumn,
		//FlexDirectionColumnReverse,
		FlexDirectionRow,
		//FlexDirectionRowReverse
	};

	ENJON_ENUM()
	enum class UIElementJustification
	{ 
		JustifyFlexStart,
		JustifyCenter,
		JustifyFlexEnd,
		//JustifySpaceBetween,
		//JustifySpaceAround,
		//JustifySpaceEvenly
	};

	ENJON_ENUM()
	enum class UIElementAlignment
	{ 
		//AlignAuto,
		AlignFlexStart,
		AlignCenter,
		AlignFlexEnd,
		//AlignStretch,
		//AlignBaseline,
		//AlignSpaceBetween,
		//AlignSpaceAround
	}; 

	ENJON_ENUM()
	enum class UIElementFlexWrap
	{
		None,
		Wrap
	};

	ENJON_ENUM()
	enum class UIElementPositionType
	{
		Relative,
		Absolute
	};

	ENJON_CLASS( Construct )
	class UIStyleSettings : public Object
	{ 
		ENJON_CLASS_BODY( UIStyleSettings )

		public: 

			ENJON_PROPERTY()
			UIElementFlexDirection mFlexDirection = UIElementFlexDirection::FlexDirectionColumn;
			
			ENJON_PROPERTY()
			UIElementJustification mJustification = UIElementJustification::JustifyFlexStart;

			ENJON_PROPERTY()
			UIElementAlignment mAlignContent = UIElementAlignment::AlignCenter;

			ENJON_PROPERTY()
			UIElementAlignment mAlignSelf = UIElementAlignment::AlignCenter;

			//ENJON_PROPERTY()
			//UIElementAlignment mAlignItems = UIElementAlignment::AlignAuto;

			ENJON_PROPERTY()
			UIElementDirection mDirection = UIElementDirection::DirectionInherit;

			ENJON_PROPERTY()
			UIElementPositionType mPositionType = UIElementPositionType::Relative;

			ENJON_PROPERTY( UIMin = 0.f, UIMax = 1.f )
			f32 mFlexGrow = 0.f;

			ENJON_PROPERTY( UIMin = 0.f, UIMax = 1.f )
			f32 mFlexShrink = 0.f;

			ENJON_PROPERTY( UIMin = 0.f )
			Vec4 mMargin = Vec4( 0.f );

			ENJON_PROPERTY( UIMin = 0.f )
			Vec4 mPadding = Vec4( 0.f );

			ENJON_PROPERTY()
			Vec4 mPosition = Vec4( 0.f );

			ENJON_PROPERTY()
			Vec2 mSize = Vec2( 0.f );
	};

	ENJON_CLASS( Construct )
	class UIStyleConfiguration : public Object
	{ 
		ENJON_CLASS_BODY( UIStyleConfiguration )

		public:

			void ExplicitConstructor( ) override;

			static UIStyleConfiguration GetDefaultStyleConfiguration( );

			void MergeRuleIntoStyle( const UIStyleRule* rule );
	
		public: 

			ENJON_PROPERTY( )
			UIElementFlexDirection mFlexDirection = UIElementFlexDirection::FlexDirectionColumn;

			ENJON_PROPERTY( )
			UIElementJustification mJustification = UIElementJustification::JustifyFlexStart;

			ENJON_PROPERTY( )
			UIElementAlignment mAlignContent = UIElementAlignment::AlignCenter;

			ENJON_PROPERTY( )
			UIElementAlignment mAlignSelf = UIElementAlignment::AlignCenter;

			//ENJON_PROPERTY()
			//UIElementAlignment mAlignItems = UIElementAlignment::AlignAuto;

			ENJON_PROPERTY( )
			UIElementDirection mDirection = UIElementDirection::DirectionInherit;

			ENJON_PROPERTY( )
			UIElementPositionType mPositionType = UIElementPositionType::Relative;

			ENJON_PROPERTY( UIMin = 0.f, UIMax = 1.f )
			f32 mFlexGrow = 0.f;

			ENJON_PROPERTY( UIMin = 0.f, UIMax = 1.f )
			f32 mFlexShrink = 0.f;

			ENJON_PROPERTY( UIMin = 0.f )
			Vec4 mMargin = Vec4( 0.f );

			ENJON_PROPERTY( UIMin = 0.f )
			Vec4 mPadding = Vec4( 0.f );

			ENJON_PROPERTY( UIMin = 0.f )
			Vec4 mBorder = Vec4( 0.f ); 

			ENJON_PROPERTY( UIMin = 0.f )
			Vec4 mBorderRadius = Vec4( 0.f ); 

			ENJON_PROPERTY( )
			Vec4 mPosition = Vec4( 0.f );

			ENJON_PROPERTY( )
			Vec2 mSize = Vec2( 30.f );

			ENJON_PROPERTY( )
			ColorRGBA8 mBackgroundColor = RGBA8_Black();

			ENJON_PROPERTY( )
			ColorRGBA8 mBorderColor = RGBA8_Black();

			ENJON_PROPERTY( )
			ColorRGBA8 mTextColor = RGBA8_White();

			ENJON_PROPERTY( )
			UIElementAlignment mTextAlignment = UIElementAlignment::AlignCenter;

			ENJON_PROPERTY( )
			UIElementJustification mTextJustification = UIElementJustification::JustifyCenter;

			ENJON_PROPERTY( )
			AssetHandle< UIFont > mFont;

			ENJON_PROPERTY( )
			f32 mFontSize = 16.f; 
	};

	ENJON_CLASS( Construct )
	class UIStyleSheet : public Asset
	{ 
		ENJON_CLASS_BODY( UIStyleSheet )
 
		friend UIStyleSheetAssetLoader;
 
		public: 

			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			virtual Result DeserializeData( ByteBuffer* buffer ) override;

			bool StyleRuleExists( const String& name );

			void AddStyleRule( const String& name, const UIStyleRule& rule, const UIStyleState& state );

			const UIStyleRuleGroup* GetStyleGroup( const String& selector ) const;

			const UIStyleRule* GetStyleRuleFromSelector( const String& selector, const UIStyleState& state ) const;

		public: 
			HashMap< String, UIStyleRuleGroup > mUIStyleRules; 
	}; 

	ENJON_CLASS( Construct )
	class UIStyleSheetAssetLoader : public AssetLoader
	{
		ENJON_CLASS_BODY( UIStyleSheetAssetLoader );

		public: 

			/**
			* @brief
			*/
			virtual String GetAssetFileExtension( ) const override;
			
		protected:
			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( ) override; 

		private: 
			/**
			* @brief
			*/
			virtual Asset* LoadResourceFromFile( const String& filePath ) override; 
	};

}

#endif