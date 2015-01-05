#include <iostream>
#include "PlayerDLL.h"
#include "GameLogic.h"
#include <stdlib.h>
#include <time.h>
#include <crtdbg.h>

#define DLL_EXPORT

int nDoInit = 1;
int nActiveBoard = 0;
const int BOARDSIZE = 64;
const int NUM_BOARDS = 10000000;
char* pBoards;

static const int nMaxPossibleMoves = 30;
int nActivePossibleMoves = 0;
char* pPossibleMoves;

const int SEARCH_DEPTH = 10;
const int INF = 10000;

static int PositionStrategy[64] = {
	10000, -8, 8, 6, 6, 8, -8, 10000,
	-8, -50, -4, -3, -3, -4, -50, -8,
	8, -4, 7, 4, 4, 7, -4, 8,
	6, -3, 4, 0, 0, 4, -3, 6,
	6, -3, 4, 0, 0, 4, -3, 6,
	8, -4, 7, 4, 4, 7, -4, 8,
	-8, -50, -4, -3, -3, -4, -50, -8,
	10000, -8, 8, 6, 6, 8, -8, 10000
};

static int ClockWise[64] = {
	56, 48, 40, 32, 24, 16, 8, 0,
	57, 49, 41, 33, 25, 17, 9, 1,
	58, 50, 42, 34, 26, 18, 10, 2,
	59, 51, 43, 35, 27, 19, 11, 3,
	60, 52, 44, 36, 28, 20, 12, 4,
	61, 53, 45, 37, 29, 21, 13, 5,
	62, 54, 46, 38, 30, 22, 14, 6,
	63, 55, 47, 39, 31, 23, 15, 7
};

static int Corner[64] = {
	0, 0, 0, 0, 7, 7, 7, 7,
	0, 0, 0, 0, 7, 7, 7, 7,
	0, 0, 0, 0, 7, 7, 7, 7,
	0, 0, 0, 0, 7, 7, 7, 7,
	56, 56, 56, 56, 63, 63, 63, 63,
	56, 56, 56, 56, 63, 63, 63, 63,
	56, 56, 56, 56, 63, 63, 63, 63,
	56, 56, 56, 56, 63, 63, 63, 63
};

int GetPositionScore(const char* pBoard, int nMove, bool bIsBlack)
{
	int nPlayer = bIsBlack ? 1 : 2;

	int nScore = PositionStrategy[nMove];
	if (nScore == -8 || nScore == -50)
	{
		if (pBoard[Corner[nMove]] == nPlayer)
		{
			nScore = 100;
		}
	}

	return nScore;
}

int GetScore(const char* pBoard, bool bIsBlack)
{
	int nScore = 0;

	for (int i = 0; i < 64; ++i)
	{
		if (IsValidMove(pBoard, i, bIsBlack))
		{
			nScore += PositionStrategy[i];
		}
	}

	return nScore;
}

const int DirectionsToFloodFill[64] = {
	007, 031, 031, 031, 031, 031, 031, 28,
	199, 255, 255, 255, 255, 255, 255, 124,
	199, 255, 255, 255, 255, 255, 255, 124,
	199, 255, 255, 248, 227, 255, 255, 124,
	199, 255, 255, 062, 143, 255, 255, 124,
	199, 255, 255, 255, 255, 255, 255, 124,
	199, 255, 255, 255, 255, 255, 255, 124,
	193, 241, 241, 241, 241, 241, 241, 112
};

const enum BitDirection{
	BitRight = 1,
	BitRightDown = 2,
	BitDown = 4,
	BitLeftDown = 8,
	BitLeft = 16,
	BitLeftUp = 32,
	BitUp = 64,
	BitRightUp = 128
};

int GetMoveDeltaFromDirection(int eDir)
{
	switch (eDir)
	{
	case BitRight:
		return 1;
	case BitRightDown:
		return 9;
	case BitDown:
		return 8;
	case BitLeftDown:
		return 7;
	case BitLeft:
		return -1;
	case BitLeftUp:
		return -9;
	case BitUp:
		return -8;
	case BitRightUp:
		return -7;
	default:
		return 0;
	}
}

void GetPossibleMovesFloodfill(int nNode, const char* pBoard, char* pFloodFill, char* PossibleMoves, int& nCount, bool bIsBlack)
{
	if (pFloodFill[nNode] == 3)
		return;
	if (pFloodFill[nNode] == 0)
	{
		pFloodFill[nNode] = 3;
		if (IsValidMove(pBoard, nNode, bIsBlack))
		{
			if(nCount < nMaxPossibleMoves - 1)
				PossibleMoves[nCount++] = nNode;
			else
			{
				int nTmp;
				nTmp = 1;
				_ASSERT(nCount < nMaxPossibleMoves - 1);
			}
		}
		return;
	}

	int eDir = 1;
	while (eDir != 256)
	{
		pFloodFill[nNode] = 3;
		if (eDir && DirectionsToFloodFill[nNode])
			GetPossibleMovesFloodfill(nNode + GetMoveDeltaFromDirection(eDir), pBoard, pFloodFill, PossibleMoves, nCount, bIsBlack);
		eDir *= 2;
	}
}

static const char Circle01[12] =
{ 18, 19, 20, 21, 26, 29, 34, 37, 42, 43, 44, 45 };

static const char Circle02[20] =
{ 9, 10, 11, 12, 13, 14, 17, 22, 25, 30, 33, 38, 41, 46, 49, 50, 51, 52, 53, 54 };

static const char Circle03[28] =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 15, 16, 23, 24, 31, 32, 39, 40, 47, 48, 55, 56, 57, 58, 59, 60, 61, 62, 63 };

inline void GetPossibleMoves(const char* pBoard, bool bIsBlack, char* PossibleMoves, int& nCount)
{
	nCount = 0;

	// ONE CIRCLE AT A TIME
	/*int i = 0;
	char nMove = 0;
	bool bNextCircle = false;
	for (i = 0; i < 12; ++i)
	{
		nMove = Circle01[i];
		if (pBoard[nMove])
			bNextCircle = true;
		else if (IsValidMove(pBoard, nMove, bIsBlack))
		{
			PossibleMoves[nCount++] = nMove;
		}
	}
	if (bNextCircle)
	{
		bNextCircle = false;
		for (i = 0; i < 20; ++i)
		{
			nMove = Circle02[i];
			if (pBoard[nMove])
				bNextCircle = true;
			else if (IsValidMove(pBoard, nMove, bIsBlack))
			{
				PossibleMoves[nCount++] = nMove;
			}
		}
	}
	if (bNextCircle)
	{
		bNextCircle = false;
		for (i = 0; i < 28; ++i)
		{
			nMove = Circle03[i];
			if (pBoard[nMove])
				bNextCircle = true;
			else if (IsValidMove(pBoard, nMove, bIsBlack))
			{
				PossibleMoves[nCount++] = nMove;
			}
		}
	}*/

	// FLOOD FILL
	/*char* pFloodFill = &pBoards[BOARDSIZE * nActiveBoard++];
	for (int i = 0; i < 64; ++i)
		pFloodFill[i] = pBoard[i];
	GetPossibleMovesFloodfill(28, pBoard, &pFloodFill[0], PossibleMoves, nCount, bIsBlack);*/

	// CHECK ALL
	for (int i = 0; i < 64; ++i)
	{
		if (IsValidMove(pBoard, i, bIsBlack))
		{
			//_ASSERT(nCount < nMaxPossibleMoves - 1);
			PossibleMoves[nCount++] = i;
		}
	}
}

int EvaluateMove(const char* pBoard, int nMove, bool bIsBlack, bool bIsBlackGood, int nDepth, int nAlpha, int nBeta)
{
	if (bIsBlack == bIsBlackGood)
	{
		if (((nMove == 1 || nMove == 8 || nMove == 9) && pBoard[0] == 0)
			|| ((nMove == 6 || nMove == 14 || nMove == 15) && pBoard[7] == 0)
			|| ((nMove == 48 || nMove == 49 || nMove == 57) && pBoard[56] == 0)
			|| ((nMove == 54 || nMove == 55 || nMove == 62) && pBoard[63] == 0))
			return -100;
	}
	if (bIsBlack == bIsBlackGood)
	{
		if (nMove == 0
			|| nMove == 7
			|| nMove == 56
			|| nMove == 63)
			return 100;
	}

	_ASSERT(nActiveBoard < NUM_BOARDS);
	char* pTestBoard = &pBoards[BOARDSIZE * nActiveBoard++];
	memcpy(pTestBoard, pBoard, sizeof(char) * 64);
	OnlyPlaceBrick(pTestBoard, nMove, bIsBlack);

	char PossibleMoves[nMaxPossibleMoves];
	memset(&PossibleMoves, -1, sizeof(char)*nMaxPossibleMoves);
	int nCount = 0;
	if (nDepth == 0)
	{
		GetPossibleMoves(pTestBoard, bIsBlackGood, &PossibleMoves[0], nCount);
		return nCount;
	}
	else
	{
		GetPossibleMoves(pTestBoard, !bIsBlack, &PossibleMoves[0], nCount);
	}

	if (nCount == 0)
		return 0;


	// Opponents turn to place brick.
	if (bIsBlack == bIsBlackGood)
	{
		for (int i = 0; i < nCount; ++i)
		{
			int nNewScore = EvaluateMove(pTestBoard, PossibleMoves[i], !bIsBlack, bIsBlackGood, nDepth - 1, nAlpha, nBeta);
			if (nNewScore < nBeta)
			{
				nBeta = nNewScore;
				if (nBeta <= nAlpha)
					return nBeta;
			}
		}
		return nBeta;
	}
	// My turn to place brick.
	else
	{
		for (int i = 0; i < nCount; ++i)
		{
			int nNewScore = EvaluateMove(pTestBoard, PossibleMoves[i], !bIsBlack, bIsBlackGood, nDepth - 1, nAlpha, nBeta);
			if (nNewScore > nAlpha)
			{
				nAlpha = nNewScore;
				if (nBeta <= nAlpha)
					return nAlpha;
			}
		}
		return nAlpha;
	}

	return 0;
}

void Setup()
{
	nDoInit = 0;
	pBoards = new char[BOARDSIZE * NUM_BOARDS];
	memset(pBoards, -1, sizeof(char) * BOARDSIZE * NUM_BOARDS);
}

extern "C"
{
	// FEW MOVES
	DECLDIR int GetMove(const int* pBoard, int nMoveNr, const int* pHistory, bool bIsBlack)
	{
		if (nDoInit)
			Setup();

		nActiveBoard = 0;
		char* pCharBoard = &pBoards[BOARDSIZE * nActiveBoard++];
		for (int i = 0; i < 64; ++i)
			pCharBoard[i] = pBoard[i];

		char PossibleMoves[nMaxPossibleMoves];
		memset(&PossibleMoves, -1, sizeof(char)*nMaxPossibleMoves);
		int nCount = 0;
		GetPossibleMoves(pCharBoard, bIsBlack, &PossibleMoves[0], nCount);
		
		int nBestMove = -1;
		int nAlpha = -INF;
		int nBeta = INF;

		for (int i = 0; i < nCount; ++i)
		{
			nActiveBoard = 0;
			char* pTestBoard = &pBoards[BOARDSIZE * nActiveBoard++];
			memcpy(pTestBoard, pCharBoard, sizeof(char) * 64);
			int nScore = EvaluateMove(pTestBoard, PossibleMoves[i], bIsBlack, bIsBlack, SEARCH_DEPTH-1, nAlpha, nBeta);
			if (nScore > nAlpha)
			{
				nAlpha = nScore;
				nBestMove = PossibleMoves[i];
			}
		}

		return nBestMove;
	}

	//// RANDOM
	/*DECLDIR int GetMove(const int* pBoard, int nMoveNr, const int* pHistory, bool bIsBlack)
	{
		int PossibleMoves[60];
		memset(PossibleMoves, -1, 60);
		int nCount = 0;

		for (int i = 0; i < 64; ++i)
		{
			if (IsValidMove(pBoard, i, 8, bIsBlack))
				PossibleMoves[nCount++] = i;
		}

		if (nCount > 0)
		{
			srand(time(GetTickCount()));
			int nRand = rand() % nCount;
			std::cout << "returning " << PossibleMoves[nRand] << std::endl;
			return PossibleMoves[nRand];
		}

		return -1;
	}*/

	// FIRST
	/*DECLDIR int GetMove(const int* pBoard, int nMoveNr, const int* pHistory, bool bIsBlack)
	{
	for (int i = 0; i < 64; ++i)
	{
	if (IsValidMove(pBoard, i, 8, bIsBlack))
	return i;
	}
	return -1;
	}*/
}