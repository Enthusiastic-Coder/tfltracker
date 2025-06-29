#include "TurnDirection.h"
#include <jibbs/math/MathSupport.h>
#include <cmath>

float TurnDirection::GetTurnDiff(float fHdg, float fBrg)
{
    float fDiff = MathSupport<float>::normAng(fBrg)-MathSupport<float>::normAng(fHdg);

    if (fDiff < -180)
        fDiff += 360;

    if (fDiff > 180)
        fDiff -= 360;

    return fDiff;
}

TurnDirection::Dir TurnDirection::GetTurnDir(float fHdg, float fBrg)
{
    float fDiff = GetTurnDiff(fHdg, fBrg);

    if (std::abs((std::fabs(fDiff) - 180)) < 0.1f)
		return Dir::Free;

    if (fDiff < 0)
        return Dir::Left;
    else
        return Dir::Right;
}
