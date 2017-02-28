/* ===========================================================
* @Author: 	       		John Jackson
* @Title:	       		defines.h 
* @Date:   	       		2015-11-08 11:31:34
* @Last Modified time: 	2016-23-05 16:51:00
=============================================================*/

#ifndef ENJON_DEFINES_H
#define ENJON_DEFINES_H

#define INTROSPECTION(...)
#define ENJON_STRUCT(...)
#define ENJON_CLASS(...)
#define ENJON_FUNCTION(...)
#define ENJON_PROP(...)

#define EG 		Enjon::Graphics
#define EM 		Enjon::Math
#define EI 		Enjon::Input
#define ER 		Enjon::Random
#define EU 		Enjon::Utils
#define EGUI 	Enjon::GUI
#define EP		Enjon::Physics
#define EA		Enjon::Animation
#define EN     	Enjon::Internals
#define ECSS	ECS::Systems
#define EScript	Enjon::Scripting

namespace Enjon
{
	enum class Result
	{
		SUCCESS,
		FAILURE,
		UNKNOWN_ERROR
	};
}

#endif
