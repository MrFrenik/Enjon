#pragma once
#ifndef ENJON_OBJECT_DEFINES_H
#define ENJON_OBJECT_DEFINES_H

#include "System/Config.h"

/*
	Used as boilerplate for all classes participating in object/reflection model.
*/
#define ENJON_CLASS_BODY( ... )																	\
	friend Enjon::Object;																			\
	public:																							\
		virtual const Enjon::MetaClass* Class( ) const override\
		{\
			return GetClassInternal();\
		}\
	private:\
		const Enjon::MetaClass* GetClassInternal() const; 

#define ENJON_COMPONENT( type )\
public:\
	virtual void Destroy() override\
	{\
		DestroyBase<type>();\
	} 

#define ENJON_ENUM( ... )
#define ENJON_PROPERTY( ... )
#define ENJON_FUNCTION( ... )
#define ENJON_CLASS( ... )
#define ENJON_STRUCT( ... )

#ifdef ENJON_SYSTEM_WINDOWS
#define ENJON_EXPORT __declspec(dllexport) 
#endif

#define ENJON_MODULE_BODY( ModuleName )\
	public:\
		virtual Enjon::Result ModuleName::BindApplicationMetaClasses() override;\
		virtual Enjon::Result ModuleName::UnbindApplicationMetaClasses() override;\


#define ENJON_MODULE_DECLARE( ModuleName )\
	class ModuleName;\
	extern "C"\
	{\
		ENJON_EXPORT void SetEngineInstance( Enjon::Engine* engine );\
		ENJON_EXPORT Enjon::Application* CreateApplication( Enjon::Engine* engine );\
		ENJON_EXPORT void DeleteApplication( Enjon::Application* app );\
	}

#define ENJON_MODULE_DEFINE( ModuleName )\
	extern "C"\
	{\
		ENJON_EXPORT Enjon::Application* CreateApplication( Enjon::Engine* engine )\
		{\
			ModuleName* app = new ModuleName();\
			if ( app )\
			{\
				Enjon::Engine::SetInstance( engine );\
				app->BindApplicationMetaClasses();\
				return app;\
			}\
			return nullptr;\
		}\
		\
		ENJON_EXPORT void DeleteApplication( Enjon::Application* app )\
		{\
			if ( app )\
			{\
				app->UnbindApplicationMetaClasses();\
				delete app;\
				app = nullptr;\
			}\
		}\
	} 

#endif