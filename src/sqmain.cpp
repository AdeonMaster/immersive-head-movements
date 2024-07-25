#include "sqfunc.h"

#include <sqapi.h>
#include <Union/Hook.h>
#include <ZenGin/zGothicAPI.h>

using namespace Gothic_II_Addon;

void __fastcall zCAICamera__AI_LookingCam(zCAICamera* self, void* vtable);
auto Hook_zCAICamera_AI_LookingCam = Union::CreateHook((void*)0x004A3690, zCAICamera__AI_LookingCam, Union::HookType::Hook_Detours);
void __fastcall zCAICamera__AI_LookingCam(zCAICamera* self, void* vtable)
{
	Hook_zCAICamera_AI_LookingCam.Disable();

	self->AI_LookingCam();

	Hook_zCAICamera_AI_LookingCam.Enable();
}

extern "C" SQRESULT SQRAT_API sqmodule_load(HSQUIRRELVM vm, HSQAPI api)
{
	SqModule::Initialize(vm, api);

	Sqrat::RootTable rootable(vm);

	/*
	* @name setLookAtTarget
	* @param (number) x
	* @param (number) y
	* @param (number) z
	*/
	rootable.SquirrelFunc("setLookAtTarget", sq_setLookAtTarget, 4, ".nnn");

	/*
	* @name lookAtTarget
	*/
	rootable.SquirrelFunc("lookAtTarget", sq_lookAtTarget);

	SqModule::Print("(Immersive-Head-Movements): Module has been initialized...");

	return SQ_OK;
}
