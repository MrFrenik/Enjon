#include "GUI/UIAsset.h"
#include "Serialize/ObjectArchiver.h"
#include "ImGui/imgui.h"

namespace Enjon
{ 
	//=================================================================================

	Result UI::SerializeData( ByteBuffer* buffer ) const
	{ 
		// Write out root canvas to buffer
		return ObjectArchiver::Serialize( &mRoot, buffer ); 
	}

	//=================================================================================
		
	Result UI::DeserializeData( ByteBuffer* buffer )
	{ 
		// Read in canvas from buffer
		return ObjectArchiver::Deserialize( buffer, &mRoot );
	} 

	//=================================================================================
	
	void UI::OnUI() const
	{
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

	void UIElementButton::OnUI()
	{ 
		ImGui::SetCursorPos( ImVec2( mPosition.x, mPosition.y ) );
		ImGui::PushID( (usize )(intptr_t )this );
		if ( ImGui::Button( mLabel.c_str() ) )
		{
			mOnClick( this );
		}
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

	UIElement* UIElementCanvas::AddChild( UIElement* element )
	{
		mChildren.push_back( element );
		return element;
	}

	//=================================================================================
	
	UIElement* UIElementCanvas::RemoveChild( UIElement* element )
	{
		// Swap and pop? Not safe to do this operation during a GUI call
		std::remove( mChildren.begin(), mChildren.end(), element );
		return element;
	}

	//=================================================================================
}















