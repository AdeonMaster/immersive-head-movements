#include "sqfunc.h"

#include <sqapi.h>

extern "C" SQRESULT SQRAT_API sqmodule_load(HSQUIRRELVM vm, HSQAPI api)
{
	SqModule::Initialize(vm, api);

	Sqrat::RootTable rootable(vm);

	/*
	* @name setLookAtTarget
	* @param (number) x
	* @param (number) y
	*/
	rootable.SquirrelFunc("setLookAtTarget", sq_setLookAtTarget, 3, ".nn");

	/*
	* @name setLookAtTargetPos
	* @param (number) x
	* @param (number) y
	* @param (number) z
	*/
	rootable.SquirrelFunc("setLookAtTargetPos", sq_setLookAtTarget, 4, ".nnn");

	/*
	* @name lookAtTarget
	*/
	rootable.SquirrelFunc("lookAtTarget", sq_lookAtTarget);

	/*
	* @name stopLookAtTarget
	*/
	rootable.SquirrelFunc("stopLookAtTarget", sq_stopLookAtTarget);

	/*
	* @name toggleAI_LookingCam
	* @param (boolean) toggle
	*/
	rootable.SquirrelFunc("toggleAI_LookingCam", sq_toggleAI_LookingCam, 2, ".b");

	SqModule::Print("(Immersive Head Movements): Module has been initialized...");

	return SQ_OK;
}
