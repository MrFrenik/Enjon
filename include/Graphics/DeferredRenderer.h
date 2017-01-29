#ifndef ENJON_DEFERRED_RENDERER_H
#define ENJON_DEFERRED_RENDERER_H

#include "Defines.h"
#include "System/Types.h"
#include "Graphics/Window.h"
#include "Graphics/Scene.h"
#include "Graphics/Camera.h"

namespace Enjon { namespace Math { 
	class iVec2;
}}

namespace Enjon { namespace Graphics {

	class RenderTarget;
	class GBuffer;
	class FullScreenQuad;
	class SpriteBatch;

	class DeferredRenderer
	{
		public:
			DeferredRenderer();
			~DeferredRenderer();

			void Init();
			void Update(float dt);

			void SetViewport(EM::iVec2& dimensions);
			EM::iVec2 GetViewport();

			EG::Scene* GetScene() { return &mScene; }

			EG::Camera* GetSceneCamera() { return &mSceneCamera; }
			EG::Window* GetWindow() { return &mWindow; }

		private:

			void InitializeFrameBuffers();
			void GBufferPass();
			void LightingPass();

			// Frame buffers
			EG::GBuffer* mGbuffer 					= nullptr;
			EG::RenderTarget* mDebugTarget 			= nullptr;
			EG::RenderTarget* mSmallBlurHorizontal  = nullptr;
			EG::RenderTarget* mSmallBlurVertical 	= nullptr;
			EG::RenderTarget* mMediumBlurHorizontal = nullptr;
			EG::RenderTarget* mMediumBlurVertical 	= nullptr;
			EG::RenderTarget* mLargeBlurHorizontal 	= nullptr;
			EG::RenderTarget* mLargeBlurVertical 	= nullptr;
			EG::RenderTarget* mComposite 			= nullptr;
			EG::RenderTarget* mLightingBuffer		= nullptr;
			EG::RenderTarget* mLuminanceBuffer		= nullptr;
			EG::RenderTarget* mFXAATarget			= nullptr;
			EG::RenderTarget* mShadowDepth			= nullptr;

			// Full screen quad
			EG::FullScreenQuad* mFullScreenQuad 	= nullptr;

			// Graphics scene
			EG::Scene 				mScene;
			EG::Window 				mWindow;
			EG::Camera 				mSceneCamera;   // Probably part of scene instead
			EG::Camera 				mShadowCamera;	// Probably part of light or scene?

			EG::SpriteBatch* 		mBatch 			= nullptr;

	};

}}


#endif