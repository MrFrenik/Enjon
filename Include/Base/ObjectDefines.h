#pragma once
#ifndef ENJON_OBJECT_DEFINES_H
#define ENJON_OBJECT_DEFINES_H

#include "System/Config.h"

enum MetaClassEnumDefines
{
	Abstract
};

enum MetaClassComponentDefines
{
	Requires,
	ConstructInstanceData
};

// Would like for these to actually be user-defined attributes, similar to what users can define in C#
enum MetaPropertyEnumDefines
{
	UIMin,
	UIMax,
	Delegates,
	Accessor,
	Mutator,
	Editable,
	HideInEditor,
	NonSerializeable,
	ReadOnly
};

#define ENJON_TO_STRING( Name ) #Name

/*
	Used as boilerplate for all classes participating in object/reflection model.
*/
#define ENJON_CLASS_BODY_INTERNAL( ClassName, ... )\
	friend Enjon::Object;\
	public:\
		ClassName()\
		{\
			ExplicitConstructor( );\
		}\
		~ClassName()\
		{\
			ExplicitDestructor( );\
		}\
		virtual const Enjon::MetaClass* Class( ) const override\
		{\
			return GetClassInternal();\
		}\
	protected:\
		virtual const Enjon::MetaClass* GetClassInternal() const override; 

#define ENJON_CLASS_BODY( ClassName, ... )\
	ENJON_CLASS_BODY_INTERNAL( ClassName, ... )

#define ENJON_COMPONENT_INTERNAL( ComponentName, ... )\
	friend Enjon::Object;\
	friend IComponentInstanceData;\
	public:\
		ComponentName()\
		{\
			ExplicitConstructor( );\
		}\
		~ComponentName()\
		{\
			ExplicitDestructor( );\
		}\
		virtual const Enjon::MetaClass* Class( ) const override\
		{\
			return GetClassInternal();\
		}\
	protected:\
		virtual const Enjon::MetaClass* GetClassInternal() const override;\
	public:\
		virtual void Destroy() override\
		{\
			DestroyBase<ComponentName>();\
		} 

#define ENJON_COMPONENT( ComponentName, ... )\
	ENJON_COMPONENT_INTERNAL( ComponentName, ... )

#define ENJON_MODULE_BODY_INTERNAL( ModuleName )\
	friend Enjon::Object;\
	public:\
		ModuleName() = default;\
		~ModuleName() = default;\
		virtual const Enjon::MetaClass* Class( ) const override\
		{\
			return GetClassInternal();\
		}\
	protected:\
		virtual const Enjon::MetaClass* GetClassInternal() const override;\
	public:\
		virtual Enjon::Result ModuleName::BindApplicationMetaClasses() override;\
		virtual Enjon::Result ModuleName::UnbindApplicationMetaClasses() override;\

#define ENJON_MODULE_BODY( ModuleName )\
	ENJON_MODULE_BODY_INTERNAL( ModuleName )


#define ENJON_ENUM( ... )
#define ENJON_PROPERTY( ... )
#define ENJON_FUNCTION( ... )
#define ENJON_CLASS( ... )
#define ENJON_STRUCT( ... )

#ifdef ENJON_SYSTEM_WINDOWS
#define ENJON_EXPORT __declspec(dllexport) 
#endif

#define ENJON_MODULE_DECLARE_INTERNAL( ModuleName )\
	class ModuleName;\
	extern "C"\
	{\
		ENJON_EXPORT void SetEngineInstance( Enjon::Engine* engine );\
		ENJON_EXPORT Enjon::Application* CreateApplication( Enjon::Engine* engine );\
		ENJON_EXPORT void DeleteApplication( Enjon::Application* app );\
	}

#define ENJON_MODULE_DECLARE( ModuleName )\
	ENJON_MODULE_DECLARE_INTERNAL( ModuleName )

#define ENJON_MODULE_DEFINE_INTERNAL( ModuleName )\
	extern "C"\
	{\
		ENJON_EXPORT Enjon::Application* CreateApplication( Enjon::Engine* engine )\
		{\
			ModuleName* app = new ModuleName();\
			if ( app )\
			{\
				Enjon::Engine::SetInstance( engine );\
				Enjon::Engine::BindImGuiContext();\
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

#define ENJON_MODULE_DEFINE( ModuleName )\
	ENJON_MODULE_DEFINE_INTERNAL( ModuleName ) 


#endif