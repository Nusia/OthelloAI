#ifndef _PLAYER_DLL_H_
#define _PLAYER_DLL_H_
#include <iostream>

//#if defined DLL_EXPORT
#define DECLDIR __declspec(dllexport)
//#else
//#define DECLDIR __declspec(dllimport)
//#endif

extern "C"
{
	DECLDIR int GetMove(const int* pBoard, int nMoveNr, const int* pHistory, bool bIsBlack);
}

#endif // _PLAYER_DLL_H_