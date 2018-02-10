#include "../stdafx.h"
#include "MathUtil.hpp"
#include "../ttapi.h"

#ifdef _EDITOR
#include "SkeletonX.h"
#include "SkeletonCustom.h"
#else
#include "../../xr_3da/xrRender/SkeletonXVertRender.h"
#define RENDER 1
#include "../../xr_3da/Render.h"
#include "../../xr_3da/xrRender/light.h"
#endif

#include "SkinXW_SSE.hpp"
#include "Skin4W_MT.hpp"
#include "PLC_SSE.hpp"
#include "../_math.h"

namespace XRay
{
namespace Math
{
Skin1WFunc Skin1W;
Skin2WFunc Skin2W;
Skin3WFunc Skin3W;
Skin4WFunc Skin4W;
PLCCalcFunc PLCCalc;

void Initialize()
{
    static bool initialized = false;
    if (initialized)
        return;
    Skin1W = Skin1W_SSE;
    Skin2W = Skin2W_SSE;
    Skin3W = Skin3W_SSE;
    Skin4W = Skin4W_SSE;
    Skin4W_MTs = Skin4W_SSE;
    PLCCalc = PLCCalc_SSE;
    if (ttapi_GetWorkerCount() > 1)
        Skin4W = Skin4W_MT;
    initialized = true;
}

} // namespace Math
} // namespace XRay
