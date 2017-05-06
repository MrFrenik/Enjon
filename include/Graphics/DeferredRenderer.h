#ifndef ENJON_DEFERRED_RENDERER_H
#define ENJON_DEFERRED_RENDERER_H

#include "Defines.h"
#include "System/Types.h"
#include "Graphics/Window.h"
#include "Graphics/Scene.h"
#include "Graphics/Camera.h" 
#include "Subsystem.h"

namespace Enjon 
{ 
	class RenderTarget;
	class Mesh; 
	class GBuffer;
	class FullScreenQuad; 
	class SpriteBatch;
	class iVec2;

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
		BloomSettings(Vec3& blurWeights, Vec3& blurIterations, Vec3& blurRadius)
			: mWeights(blurWeights), mIterations(blurIterations), mRadius(blurRadius)
		{}

		Vec3 mWeights;
		Vec3 mIterations;
		Vec3 mRadius;

		double mSmallGaussianCurve[16];
		double mMediumGaussianCurve[16];
		double mLargeGaussianCurve[16];
	};


	class DeferredRenderer : public Subsystem
	{
		public:

			/**
			*@brief Constructor
			*/
			DeferredRenderer();
			
			/**
			*@brief Destructor
			*/
			~DeferredRenderer();

			/**
			*@brief
			*/
			virtual Enjon::Result Initialize() override;

			/**
			*@brief
			*/
			virtual void Update(const f32 dT) override;
			
			/**
			*@brief
			*/
			virtual Enjon::Result Shutdown() override;

			/**
			*@brief
			*/
			void SetViewport(iVec2& dimensions);
			
			/**
			*@brief
			*/
			iVec2 GetViewport();

			/**
			*@brief
			*/
			Scene* GetScene() { return &mScene; }

			/**
			*@brief
			*/
			Camera* GetSceneCamera() { return &mSceneCamera; }
			
			/**
			*@brief
			*/
			Window* GetWindow() { return &mWindow; }

		private:

			/**
			*@brief
			*/
			void InitializeFrameBuffers();
			
			/**
			*@brief
			*/
			void CalculateBlurWeights();
			
			/**
			*@brief
			*/
			void RegisterCVars();

			/**
			*@brief
			*/
			void GBufferPass();
			
			/**
			*@brief
			*/
			void LightingPass();
			
			/**
			*@brief
			*/
			void LuminancePass();
			
			/**
			*@brief
			*/
			void BloomPass();

			/**
			*@brief
			*/
			void BloomPass2( );
			
			/**
			*@brief
			*/
			void FXAAPass(RenderTarget* inputTarget);
			
			/**
			*@brief
			*/
			void CompositePass(RenderTarget* inputTarget);
			
			/**
			*@brief
			*/
			void GuiPass();
			
			/**
			*@brief
			*/
			void ImGuiStyles();
			
			/**
			*@brief
			*/
			void ImGuiStyles2();

			/**
			*@brief
			*/
			void ShowGraphicsWindow(bool* p_open);
			
			/**
			*@brief
			*/
			void ShowGameViewport(bool* open);

		private:
			// Frame buffers
			GBuffer* mGbuffer 										= nullptr;
			RenderTarget* mDebugTarget 								= nullptr;
			RenderTarget* mSmallBlurHorizontal						= nullptr;
			RenderTarget* mSmallBlurVertical 						= nullptr;
			RenderTarget* mMediumBlurHorizontal						= nullptr;
			RenderTarget* mMediumBlurVertical 						= nullptr;
			RenderTarget* mLargeBlurHorizontal 						= nullptr;
			RenderTarget* mLargeBlurVertical 						= nullptr;
			RenderTarget* mCompositeTarget 							= nullptr;
			RenderTarget* mLightingBuffer							= nullptr;
			RenderTarget* mLuminanceTarget							= nullptr;
			RenderTarget* mFXAATarget								= nullptr;
			RenderTarget* mShadowDepth								= nullptr;
			RenderTarget* mFinalTarget								= nullptr;
			RenderTarget* mBloomDownSampleHalfVertical				= nullptr;
			RenderTarget* mBloomDownSampleHalfHorizontal			= nullptr;
			RenderTarget* mBloomDownSampleQuarterVertical			= nullptr;
			RenderTarget* mBloomDownSampleQuarterHorizontal			= nullptr;
			RenderTarget* mBloomDownSampleEighthVertical			= nullptr;
			RenderTarget* mBloomDownSampleEighthHorizontal			= nullptr;
			RenderTarget* mBloomDownSampleSixteenthVertical			= nullptr;
			RenderTarget* mBloomDownSampleSixteenthHorizontal		= nullptr;
			RenderTarget* mBloomDownSampleThirtySecondVertical		= nullptr;
			RenderTarget* mBloomDownSampleThirtySecondHorizontal	= nullptr;
			RenderTarget* mBloomDownSampleSixtyFourthVertical		= nullptr;
			RenderTarget* mBloomDownSampleSixtyFourthHorizontal		= nullptr;
			RenderTarget* mBloomUpSampleHalfVertical				= nullptr;
			RenderTarget* mBloomUpSampleHalfHorizontal				= nullptr;
			RenderTarget* mBloomUpSampleQuarterVertical				= nullptr;
			RenderTarget* mBloomUpSampleQuarterHorizontal			= nullptr;
			RenderTarget* mBloomUpSampleEighthVertical				= nullptr;
			RenderTarget* mBloomUpSampleEighthHorizontal			= nullptr;
			RenderTarget* mBloomUpSampleSixteenthVertical			= nullptr;
			RenderTarget* mBloomUpSampleSixteenthHorizontal			= nullptr;
			RenderTarget* mBloomUpSampleThirtySecondVertical		= nullptr;
			RenderTarget* mBloomUpSampleThirtySecondHorizontal		= nullptr;
			RenderTarget* mBloomUpSampleSixtyFourthVertical			= nullptr;
			RenderTarget* mBloomUpSampleSixtyFourthHorizontal		= nullptr;
		

			GLuint mCurrentRenderTexture;

			// Full screen quad
			FullScreenQuad* mFullScreenQuad 	= nullptr;

			// Graphics scene
			Scene 				mScene;
			Window 				mWindow;
			Camera 				mSceneCamera;   // Probably part of scene instead
			Camera 				mShadowCamera;	// Probably part of light or scene?

			SpriteBatch* 	mBatch 			= nullptr; 

			// Post processing settings
			FXAASettings mFXAASettings = FXAASettings(8.0f, 1.0f/8.0f, 1.0f/128.0f);
			ToneMapSettings mToneMapSettings = ToneMapSettings(0.53f, 1.55f, 0.61f, 7.74f, 1.7f);
			BloomSettings mBloomSettings = BloomSettings(Vec3(0.402f, 0.383f, 0.387f), Vec3(9, 7, 12), Vec3(0.004f, 0.008f, 0.019f));

			bool mShowGame = true;
			bool mShowGraphicsOptionsWindow = true;
			bool mShowStyles = true;

			f32 mBGColor[4] = {0.8f, 0.8f, 0.8f, 1.0f};
	};
}


#endif