#include "sqfunc.h"

#include <sqapi.h>
#include <Union/Hook.h>
#include <ZenGin/zGothicAPI.h>

using namespace Gothic_II_Addon;

SQInteger sq_setLookAtTarget(HSQUIRRELVM vm)
{
	SQFloat x;
	sq_getfloat(vm, 2, &x);

	SQFloat y;
	sq_getfloat(vm, 3, &y);

	SQFloat z;
	sq_getfloat(vm, 4, &z);
	
	zVEC3 target (x, y, z);
	player->human_ai->SetLookAtTarget(target);

	return SQ_OK;
}

SQInteger sq_lookAtTarget(HSQUIRRELVM vm)
{
	player->human_ai->LookAtTarget();

	return SQ_OK;
}