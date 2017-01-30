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

	struct ToneMapSettings
	{
		ToneMapSettings(float exposure, float gamma, float bloomscalar, float scale, float threshold, float saturation)
			: mExposure(exposure), mGamma(gamma), mBloomScalar(bloomscalar), mScale(scale), mThreshold(threshold), mSaturation(saturation)
		{}

		float mExposure;
		float mGamma;
		float mBloomScalar;
		float mScale;
		float mThreshold;
		float mSaturation;
	};

	struct FXAASettings
	{
		FXAASettings(float span, float mul, float min)
			: mSpanMax(span), mReduceMin(min), mReduceMul(mul)
		{}

		float mSpanMax;
		float mReduceMin;
		float mReduceMul;
	};

	struct BloomSettings
	{
		BloomSettings(EM::Vec3& blurWeights, EM::Vec3& blurIterations, EM::Vec3& blurRadius)
			: mWeights(blurWeights), mIterations(blurIterations), mRadius(blurRadius)
		{}

		EM::Vec3 mWeights;
		EM::Vec3 mIterations;
		EM::Vec3 mRadius;
	};


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
			void FXAAPass(EG::RenderTarget* input);

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

			// Post processing settings
			FXAASettings mFXAASettings = FXAASettings(8.0f, 1.0f/8.0f, 1.0f/128.0f);
			ToneMapSettings mToneMapSettings = ToneMapSettings(0.5f, 1.5f, 1.0f, 2.0f, 2.0f, 1.0f);
			BloomSettings mBloomSettings = BloomSettings(EM::Vec3(0.4f, 0.35f, 0.0f), EM::Vec3(5, 4, 1), EM::Vec3(0.001f, 0.009f, 0.009f));

	};

}}


#endif