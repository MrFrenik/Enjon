#ifndef ENJON_GRAPHICS_SUBSYSTEM_H
#define ENJON_GRAPHICS_SUBSYSTEM_H

#include "Defines.h"
#include "System/Types.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsScene.h"
#include "Graphics/Camera.h" 
#include "Graphics/SpriteBatch.h"
#include "Entity/EntityManager.h"
#include "Subsystem.h" 

namespace Enjon 
{ 
	class RenderTarget;
	class Mesh; 
	class GBuffer;
	class FullScreenQuad; 
	class SpriteBatch;
	class iVec2;
	class Shader;
	class ShaderUniform;

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

	struct PickResult
	{
		EntityHandle mEntity;
		u32 mId;
	};


	ENJON_CLASS( )
	class GraphicsSubsystem : public Subsystem
	{
		ENJON_CLASS_BODY( )

		public:

			/**
			*@brief Constructor
			*/
			GraphicsSubsystem();
			
			/**
			*@brief Destructor
			*/
			~GraphicsSubsystem();

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
			iVec2 GetViewport() const;

			/**
			*@brief
			*/
			GraphicsScene* GetGraphicsScene() { return &mGraphicsScene; }

			/**
			*@brief
			*/
			const Camera* GetGraphicsSceneCamera() const 
			{ 
				return &mGraphicsSceneCamera; 
			}
			
			/**
			*@brief
			*/
			const Window* GetWindow() const { return &mWindow; }

			/*
			* @brief
			*/
			void BindShader( const Enjon::Shader* shader );

			/*
			* @brief
			*/
			u32 GetCurrentRenderTextureId( ) const;

			/**
			*@brief
			*/
			void ShowGraphicsWindow(bool* p_open);

			/**
			*@brief
			*/
			ShaderUniform* NewShaderUniform( const MetaClass* uniformClass ) const;

			/**
			*@brief
			*/
			PickResult GetPickedObjectResult( const iVec2& screenPosition );

			/**
			*@brief
			*/
			PickResult GetPickedObjectResult( const Vec2& screenPosition );

			/**
			*@brief
			*/
			iVec2 GetImGuiViewport( ) const;

			/**
			*@brief
			*/
			void ReinitializeRenderTargets( );

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
			void SSAOPass();
			
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
			void MotionBlurPass(RenderTarget* inputTarget);
			
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
			void UIPass( RenderTarget* inputTarget ); 
			
			/**
			*@brief
			*/
			void ImGuiPass();
			
			/**
			*@brief
			*/
			void ImGuiStyles();

			/**
			*@brief
			*/
			void InitializeNoiseTexture( ); 
			
			/**
			*@brief
			*/
			void ImGuiStyles2(); 
			
			/**
			*@brief
			*/
			void ShowGameViewport(bool* open);

			void STBTest( );

			void InstancingTest( );

			void RenderCube( );


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
			RenderTarget* mSSAOTarget								= nullptr;
			RenderTarget* mSSAOBlurTarget							= nullptr;
			RenderTarget* mMotionBlurTarget							= nullptr;

			GLuint mCurrentRenderTexture; 

			// Full screen quad
			FullScreenQuad* mFullScreenQuad 	= nullptr;

			// Graphics scene
			GraphicsScene 		mGraphicsScene;
			Window 				mWindow;
			Camera 				mGraphicsSceneCamera;   // Probably part of scene instead
			Camera 				mShadowCamera;	// Probably part of light or scene?

			SpriteBatch* 	mBatch 			= nullptr; 

			Enjon::Shader* mActiveShader = nullptr;

			// Post processing settings
			FXAASettings mFXAASettings = FXAASettings(8.0f, 1.0f/8.0f, 1.0f/128.0f);	// Stock settings
			//FXAASettings mFXAASettings = FXAASettings(8.0f, 0.00001f, 0.00001f);

			//float mExposure;
			//float mGamma;
			//float mBloomScalar;
			//float mThreshold;
			//float mSaturation;
			ToneMapSettings mToneMapSettings = ToneMapSettings(0.77f, 1.84f, 0.61f, 7.74f, 1.2f);
			BloomSettings mBloomSettings = BloomSettings(Vec3(0.384f, 0.366f, 0.500f), Vec3(3, 3, 2), Vec3(0.001f, 0.006f, 0.015f)); 

			bool mShowGame = true;
			bool mShowGraphicsOptionsWindow = true;
			bool mShowStyles = true;

			f32 mBGColor[4] = {0.8f, 0.8f, 0.8f, 1.0f};

			Vec4 uvs = Vec4( 0.f, 0.f, 1.f, 1.f ); 
			
			u32 mHDRTextureID = 0;
			u32 mEnvCubemapID = 0; 
			u32 mIrradianceMap = 0;
			u32 mPrefilteredMap = 0;
			u32 mBRDFLUT = 0;
			u32 mCaptureFBO = 0;
			u32 mCaptureRBO = 0; 

			// SSAO
			u32 mSSAOColorBuffer = 0;
			u32 mSSAOColorBufferBlur = 0;
			u32 mSSAONoiseTexture = 0;
			u32 mSSAOFBO = 0;
			u32 mSSAOBlurFBO = 0;
			std::vector< Enjon::Vec3 > mSSAOKernel; 

			Enjon::Mesh* mMesh = nullptr;

			f32 mSSAORadius = 0.142f;
			f32 mSSAOScale = 5.0f;
			f32 mSSAOBias = 0.057f;
			f32 mSSAOIntensity = 15.0f;
			u32 mSSAOKernelSize = 16;

			Enjon::Mat4* mModelMatricies = nullptr;
			Enjon::Renderable* mInstancedRenderable = nullptr;
			u32 mInstancedAmount = 0;
			u32 mInstancedVBO; 

			Enjon::SpriteBatch mUIBatch;

			Mat4 mPreviousViewProjectionMatrix = Mat4::Identity( );

			f32 mMotionBlurVelocityScale = 1.0f;
			u32 mMotionBlurEnabled = true;
	};
}


#endif