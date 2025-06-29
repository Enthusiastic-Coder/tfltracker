#pragma once

class TurnDirection
{
public:
	enum class Dir 
	{ 
		Free, 
		Left, 
		Right 
	};

    static float GetTurnDiff(float fHdg, float fBrg);
	static Dir GetTurnDir(float fHdg, float fBrg);
};
