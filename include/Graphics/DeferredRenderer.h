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
	class Mesh;

	struct ToneMapSettings
	{
		ToneMapSettings(float exposure, float gamma, float bloomscalar, float threshold, float saturation)
			: mExposure(exposure), mGamma(gamma), mBloomScalar(bloomscalar), mThreshold(threshold), mSaturation(saturation)
		{}

		float mExposure;
		float mGamma;
		float mBloomScalar;
		float mThreshold;
		float mSaturation;
	};

	struct FXAASettings
	{
		FXAASettings(float span, float mul, float min)
			: mSpanMax(span), mReduceMin(min), mReduceMul(mul), mEnabled(1)
		{}

		float mSpanMax;
		float mReduceMin;
		float mReduceMul;
		u32 mEnabled;
	};

	struct BloomSettings
	{
		BloomSettings(EM::Vec3& blurWeights, EM::Vec3& blurIterations, EM::Vec3& blurRadius)
			: mWeights(blurWeights), mIterations(blurIterations), mRadius(blurRadius)
		{}

		EM::Vec3 mWeights;
		EM::Vec3 mIterations;
		EM::Vec3 mRadius;

		double mSmallGaussianCurve[16];
		double mMediumGaussianCurve[16];
		double mLargeGaussianCurve[16];
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
			void CalculateBlurWeights();
			void RegisterCVars();
			void GBufferPass();
			void LightingPass();
			void LuminancePass();
			void BloomPass();
			void FXAAPass(EG::RenderTarget* inputTarget);
			void CompositePass(EG::RenderTarget* inputTarget);
			void GuiPass();
			void ImGuiStyles();
			void ImGuiStyles2();

			void ShowGraphicsWindow(bool* p_open);
			void ShowGameViewport(bool* open);

			// Frame buffers
			EG::GBuffer* mGbuffer 					= nullptr;
			EG::RenderTarget* mDebugTarget 			= nullptr;
			EG::RenderTarget* mSmallBlurHorizontal  = nullptr;
			EG::RenderTarget* mSmallBlurVertical 	= nullptr;
			EG::RenderTarget* mMediumBlurHorizontal = nullptr;
			EG::RenderTarget* mMediumBlurVertical 	= nullptr;
			EG::RenderTarget* mLargeBlurHorizontal 	= nullptr;
			EG::RenderTarget* mLargeBlurVertical 	= nullptr;
			EG::RenderTarget* mCompositeTarget 		= nullptr;
			EG::RenderTarget* mLightingBuffer		= nullptr;
			EG::RenderTarget* mLuminanceTarget		= nullptr;
			EG::RenderTarget* mFXAATarget			= nullptr;
			EG::RenderTarget* mShadowDepth			= nullptr;
			EG::RenderTarget* mFinalTarget			= nullptr;

			GLuint mCurrentRenderTexture;

			// Full screen quad
			EG::FullScreenQuad* mFullScreenQuad 	= nullptr;

			// Graphics scene
			EG::Scene 				mScene;
			EG::Window 				mWindow;
			EG::Camera 				mSceneCamera;   // Probably part of scene instead
			EG::Camera 				mShadowCamera;	// Probably part of light or scene?

			EG::SpriteBatch* 		mBatch 			= nullptr;

			// float mExposure;
			// float mGamma;
			// float mBloomScalar;
			// float mThreshold;
			// float mSaturation;

			// Post processing settings
			FXAASettings mFXAASettings = FXAASettings(8.0f, 1.0f/8.0f, 1.0f/128.0f);
			ToneMapSettings mToneMapSettings = ToneMapSettings(0.53f, 1.55f, 0.61f, 7.74f, 1.7f);
			BloomSettings mBloomSettings = BloomSettings(EM::Vec3(0.402f, 0.383f, 0.387f), EM::Vec3(9, 7, 12), EM::Vec3(0.004f, 0.008f, 0.019f));

			bool mShowGame = true;
			bool mShowGraphicsOptionsWindow = true;
			bool mShowStyles = true;

			f32 mBGColor[4] = {0.8f, 0.8f, 0.8f, 1.0f};
	};
}}


#endif