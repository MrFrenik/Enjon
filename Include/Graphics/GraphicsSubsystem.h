#ifndef ENJON_GRAPHICS_SUBSYSTEM_H
#define ENJON_GRAPHICS_SUBSYSTEM_H

#define MAX_DEBUG_LINES 100000

#include "Defines.h"
#include "System/Types.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsScene.h"
#include "Graphics/Camera.h" 
#include "Graphics/SpriteBatch.h"
#include "Graphics/Color.h"
#include "Entity/EntityManager.h"
#include "Base/SubsystemContext.h"
#include "Graphics/GBuffer.h"
#include "Subsystem.h" 

namespace Enjon 
{ 
	class FrameBuffer;
	class Mesh; 
	class FullScreenQuad; 
	class SpriteBatch;
	class iVec2;
	class Shader;
	class ShaderUniform;
	class World;

	ENJON_CLASS( )
	class GraphicsSubsystemContext : public SubsystemContext
	{
		ENJON_CLASS_BODY( GraphicsSubsystemContext )

		public:

			/**
			* @brief
			*/
			GraphicsSubsystemContext( World* world );

			/**
			* @brief
			*/
			virtual void ExplicitDestructor( ) override;

			/**
			* @brief
			*/
			GraphicsScene* GetGraphicsScene( );

			// TODO(): HACK! DESTROY!
			void SetScene( GraphicsScene* scene )
			{
				mScene = *scene;
			}

			/**
			* @brief
			*/
			FrameBuffer* GetFrameBuffer( ) const; 

			/**
			* @brief
			*/
			GBuffer* GetGBuffer( ) const;

			/**
			* @brief
			*/
			FrameBuffer* GetObjectIDBuffer( ) const;

			/**
			* @brief
			*/
			void ReinitializeFrameBuffers( const iVec2& viewport );

		protected: 

			GraphicsScene mScene;
			FrameBuffer* mBackBuffer = nullptr;	// Eventually will need to just have a rendertarget that can be used with a "global" framebuffer
			FrameBuffer* mObjectIDBuffer = nullptr;
			GBuffer* mGBuffer = nullptr;
	};

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

	struct DebugLine
	{
		Vec3 mStart;
		Vec3 mStartColor;
		Vec3 mEnd;
		Vec3 mEndColor;
	}; 

	ENJON_CLASS( )
	class GraphicsSubsystem : public Subsystem
	{
		ENJON_CLASS_BODY( GraphicsSubsystem )

		public: 

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
			void AddContext( GraphicsSubsystemContext* context );

			/**
			*@brief
			*/ 
			void RemoveContext( GraphicsSubsystemContext* context );

			/**
			*@brief
			*/
			GraphicsScene* GetGraphicsScene( );

			/**
			*@brief
			*/
			const Camera* GetGraphicsSceneCamera( );
			
			/**
			*@brief
			*/
			const Window* GetWindow() const { return mCurrentWindow; }

			/*
			* @brief
			*/
			void BindShader( const Enjon::Shader* shader );

			/*
			* @brief
			*/
			u32 GetCurrentRenderTextureId( ) const;

			/*
			* @brief
			*/
			u32 GetGBufferTexture( GBufferTextureType type );

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
			PickResult GetPickedObjectResult( const iVec2& screenPosition, GraphicsSubsystemContext* ctx );

			/**
			*@brief
			*/
			PickResult GetPickedObjectResult( const Vec2& screenPosition, GraphicsSubsystemContext* ctx );

			/**
			*@brief
			*/
			iVec2 GetImGuiViewport( ) const;

			/**
			*@brief
			*/
			Vector< Window* > GetWindows( ) const
			{
				return mWindows;
			}

			// NOTE(): HACK HACK HACK! Just for testing
			Window* GetMainWindow( )
			{
				if ( mWindows.size( ) )
				{
					return mWindows.at( 0 );
				}

				return nullptr;
			}

			void AddWindow( Window* window )
			{
				mWindows.push_back( window );
			}

			/** 
			* @brief
			*/
			void RemoveWindow( Window* window );

			/**
			*@brief
			*/
			void ReinitializeFrameBuffers( );

			void DrawDebugLine( const Vec3& start, const Vec3& end, const Vec3& color = Vec3( 1.0f ) );

			void DrawDebugCircle( const Vec3& center, const f32& radius, const Vec3& normal = Vec3::ZAxis(), const u32& numSegments = 8, const Vec3& color = Vec3( 1.0f ) );

			void DrawDebugSphere( const Vec3& center, const f32& radius, const u32& numSlices = 5, const u32& numSegments = 8, const Vec3& color = Vec3( 1.0f ) );

			void DrawDebugAABB( const Vec3& min, const Vec3& max, const Vec3& color = Vec3( 1.0f ) );

		private:

			void SubmitSkybox( GraphicsSubsystemContext* ctx );

			void InitDebugDrawing( );

			void DebugDrawSubmit( );

			/**
			*@brief
			*/
			void FreeAllFrameBuffers( );

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
			void GBufferPass( GraphicsSubsystemContext* ctx );
			
			/**
			*@brief
			*/
			void SSAOPass( GraphicsSubsystemContext* ctx );
			
			/**
			*@brief
			*/
			void LightingPass( GraphicsSubsystemContext* ctx );
			
			/**
			*@brief
			*/
			void LuminancePass( GraphicsSubsystemContext* ctx );
			
			/**
			*@brief
			*/
			void BloomPass( GraphicsSubsystemContext* ctx );

			/**
			*@brief
			*/
			void BloomPass2( GraphicsSubsystemContext* ctx );

			/**
			*@brief
			*/
			void MotionBlurPass( FrameBuffer* inputTarget, GraphicsSubsystemContext* ctx );
			
			/**
			*@brief
			*/
			void FXAAPass( FrameBuffer* inputTarget, GraphicsSubsystemContext* ctx );
			
			/**
			*@brief
			*/
			void CompositePass( FrameBuffer* inputTarget, GraphicsSubsystemContext* ctx );

			/**
			*@brief
			*/
			void UIPass( FrameBuffer* inputTarget, GraphicsSubsystemContext* ctx ); 
			
			/**
			*@brief
			*/
			void ImGuiPass( );

			/**
			*@brief
			*/
			void PresentBackBuffer( );
			
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
			FrameBuffer* mDebugTarget 								= nullptr;
			FrameBuffer* mSmallBlurHorizontal						= nullptr;
			FrameBuffer* mSmallBlurVertical 						= nullptr;
			FrameBuffer* mMediumBlurHorizontal						= nullptr;
			FrameBuffer* mMediumBlurVertical 						= nullptr;
			FrameBuffer* mLargeBlurHorizontal 						= nullptr;
			FrameBuffer* mLargeBlurVertical 						= nullptr;
			FrameBuffer* mCompositeTarget 							= nullptr;
			FrameBuffer* mLightingBuffer							= nullptr;
			FrameBuffer* mLuminanceTarget							= nullptr;
			FrameBuffer* mFXAATarget								= nullptr;
			FrameBuffer* mShadowDepth								= nullptr;
			FrameBuffer* mFinalTarget								= nullptr;
			FrameBuffer* mSSAOTarget								= nullptr;
			FrameBuffer* mSSAOBlurTarget							= nullptr;
			FrameBuffer* mMotionBlurTarget							= nullptr;

			GLuint mCurrentRenderTexture; 

			// Full screen quad
			FullScreenQuad* mFullScreenQuad 	= nullptr;

			// Graphics scene
			GraphicsScene 		mGraphicsScene;
			Window 				mWindow;
			Window 				mWindowOther;
			Window*				mCurrentWindow = nullptr;
			Vector<Window*>		mWindows;
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
			ToneMapSettings mToneMapSettings = ToneMapSettings(0.77f, 2.2f, 0.61f, 7.74f, 1.3f);
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

			f32 mSSAORadius = 0.01f;
			f32 mSSAOScale = 5.0f;
			f32 mSSAOBias = 0.2f;
			f32 mSSAOIntensity = 15.0f;
			u32 mSSAOKernelSize = 16;

			Enjon::Mat4x4* mModelMatricies = nullptr;
			Enjon::Renderable* mInstancedRenderable = nullptr;
			u32 mInstancedAmount = 0;
			u32 mInstancedVBO; 

			Enjon::SpriteBatch mUIBatch;

			Mat4x4 mPreviousViewProjectionMatrix = Mat4x4::Identity( );

			f32 mMotionBlurVelocityScale = 2.0f;
			u32 mMotionBlurEnabled = true; 

			Vector< DebugLine > mDebugLines;
			GLuint mDebugLineVAO; 
			GLuint mDebugLineVBO; 

			HashSet< GraphicsSubsystemContext* > mContexts;
	};
}


#endif