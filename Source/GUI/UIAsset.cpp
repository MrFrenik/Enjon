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
		if ( strcmp( label, mRoot.mLabel.c_str() )  == 0 )
		{
			return (UIElement*)&mRoot;
		}

		// Not recursive for now...
		for ( auto& c : mRoot.mChildren )
		{
			if ( strcmp( c->mLabel.c_str(), label ) == 0 )
			{
				return c;
			}
		}

		return nullptr;
	}

	//=================================================================================

	void UIElementText::OnUI()
	{
		//ImGui::SetCursorScreenPos( ImVec2( mPosition.x, mPosition.y ) );
		ImVec2 cp = ImGui::GetCursorPos( );
		ImGui::SetCursorPos( ImVec2( cp.x + mPosition.x, cp.y + mPosition.y ) );
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
		ImVec2 cp = ImGui::GetCursorPos( );
		ImGui::SetCursorPos( ImVec2( cp.x + mPosition.x, cp.y + mPosition.y ) );
		ImGui::PushID( (usize )(intptr_t )this );
		if ( ImGui::Button( mLabel.c_str(), ImVec2( mSize.x, mSize.y ) ) )
		{
			mOnClick( this );
		}
		ImGui::PopID(); 
	}

	//=================================================================================

	void UIElementImage::OnUI()
	{
		ImVec2 cp = ImGui::GetCursorPos( );
		ImGui::SetCursorPos( ImVec2( cp.x + mPosition.x, cp.y + mPosition.y ) );
		ImGui::PushID( ( usize )( intptr_t )this );
		ImTextureID img = mImage ? ( ImTextureID )Int2VoidP( mImage->GetTextureId( ) ) : ( ImTextureID )Int2VoidP( EngineSubsystem( AssetManager )->GetDefaultAsset< Texture >( )->GetTextureId( ) );
		ImGui::Image( img, ImVec2( mSize.x, mSize.y ), ImVec2( mUV0.x, mUV0.y ), ImVec2( mUV1.x, mUV1.y ) );
		ImGui::PopID();
	}

	//=================================================================================

	void UIElementCanvas::OnUI()
	{
		ImGui::SetNextWindowPos( ImVec2( mPosition.x, mPosition.y ) );
		ImGui::SetNextWindowSize( ImVec2( mSize.x, mSize.y ) );
		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.f, 0.f, 0.f, 0.f ) );
		ImGui::Begin( mLabel.c_str(), nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoBringToFrontOnFocus
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

	UIElement* UIElement::AddChild( UIElement* element )
	{
		mChildren.push_back( element );
		return element;
	}

	//=================================================================================
	
	UIElement* UIElement::RemoveChild( UIElement* element )
	{
		// Swap and pop? Not safe to do this operation during a GUI call
		std::remove( mChildren.begin(), mChildren.end(), element );
		return element;
	}

	//=================================================================================

	Result UI::OnEditorUI()
	{
		// Show the asset handle property
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		const MetaProperty* prop = Class()->GetPropertyByName( ENJON_TO_STRING( mStyleConfig ) );
		if ( prop )
		{
			igm->DebugDumpProperty( this, prop );
		}

		return Result::SUCCESS;
	}

	//=================================================================================
}















