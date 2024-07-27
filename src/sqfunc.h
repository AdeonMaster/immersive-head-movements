#pragma once

#include <sqapi.h>

SQInteger sq_setLookAtTarget(HSQUIRRELVM vm);
SQInteger sq_setLookAtTargetPos(HSQUIRRELVM vm);
SQInteger sq_lookAtTarget(HSQUIRRELVM vm);
SQInteger sq_stopLookAtTarget(HSQUIRRELVM vm);

SQInteger sq_toggleAI_LookingCam(HSQUIRRELVM vm);