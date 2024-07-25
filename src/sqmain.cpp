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

	return SQ_OK;
}
