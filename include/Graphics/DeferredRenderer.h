#ifndef ENJON_DEFERRED_RENDERER_H
#define ENJON_DEFERRED_RENDERER_H

#include "Defines.h"
#include "System/Types.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/GBuffer.h"
#include "Graphics/Window.h"
#include "Graphics/Scene.h"
#include "Graphics/Camera.h"
#include "Graphics/SpriteBatch.h"

namespace Enjon { namespace Math { 
	class iVec2;
}}

namespace Enjon { namespace Graphics {

	class DeferredRenderer
	{
		public:
			DeferredRenderer();
			~DeferredRenderer();

			void Init();
			void Update(float dt);

			void SetViewport(EM::iVec2& dimensions);
			EM::iVec2 GetViewport();

		private:

			void InitializeFrameBuffers();
			void GBufferPass();

			void SubmitRenderable(EG::Renderable* renderable);

			// Frame buffers
			EG::GBuffer 			mGbuffer;
			EG::RenderTarget 		mDebugTarget;
			EG::RenderTarget 		mSmallBlurHorizontal;
			EG::RenderTarget 		mSmallBlurVertical;
			EG::RenderTarget 		mMediumBlurHorizontal;
			EG::RenderTarget 		mMediumBlurVertical;
			EG::RenderTarget 		mLargeBlurHorizontal;
			EG::RenderTarget 		mLargeBlurVertical;
			EG::RenderTarget 		mComposite;
			EG::RenderTarget 		mLightingBuffer;
			EG::RenderTarget 		mLuminanceBuffer;
			EG::RenderTarget		mFXAATarget;
			EG::RenderTarget		mShadowDepth;

			// Graphics scene
			EG::Scene 				mScene;
			EG::Window 				mWindow;
			EG::Camera 				mSceneCamera;   // Probably part of scene instead

			EG::SpriteBatch 		mBatch;
	};

}}


#endif