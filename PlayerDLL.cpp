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
const int SEARCH_DEPTH = 3;

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
	//int nRow = nMove / 8;
	//int nCol = nMove % 8;
	//int nSize = nRow * nCol;
	//nSize += ceil(float(nRow) / float(nCol));
	//
	//bool bGood = pBoard[0] == nPlayer;
	//// Check upper left
	//for (int i; i <= nCol && bGood; ++i)
	//{
	//	for (int j; j <= nRow && bGood; ++i)
	//	{
	//		if (pBoard[i + (j * 8)] != nPlayer)
	//			bGood = false;
	//	}
	//}
	//if (bGood)
	//{

	//}

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

void GetPossibleMoves(const char* pBoard, bool bIsBlack, int PossibleMoves[60], int& nCount)
{
	nCount = 0;

	for (int i = 0; i < 64; ++i)
	{
		if (IsValidMove(pBoard, i, bIsBlack))
			PossibleMoves[nCount++] = i;
	}
}

int EvaluateMove(const char* pBoard, int nMove, bool bIsBlack, bool bIsBlackGood, int nDepth)
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

	int PossibleMoves[60];
	int nCount = 0;
	memset(PossibleMoves, -1, sizeof(int) * 60);
	GetPossibleMoves(pBoard, bIsBlack, PossibleMoves, nCount);
	
	if (nDepth == 0)
		return nCount;
	


	_ASSERT(nActiveBoard < NUM_BOARDS);
	char* pTestBoard = &pBoards[BOARDSIZE * nActiveBoard++];
	memcpy(pTestBoard, pBoard, sizeof(char) * 64);
	OnlyPlaceBrick(pTestBoard, nMove, bIsBlack);
	
	nCount = 0;
	memset(PossibleMoves, -1, sizeof(int) * 60);
	GetPossibleMoves(pTestBoard, !bIsBlack, PossibleMoves, nCount);
	
	if (nCount == 0)
		return 0;

	int nBestScore = 0;

	// Opponents turn to place brick
	if (bIsBlack == bIsBlackGood)
	{
		nBestScore = 101;
		for (int i = 0; i < nCount; ++i)
		{
			int nNewScore = EvaluateMove(pTestBoard, PossibleMoves[i], !bIsBlack, bIsBlackGood, nDepth - 1);
			if (nBestScore > nNewScore)
				nBestScore = nNewScore;
		}
	}
	// My turn to place brick
	else
	{
		nBestScore = -101;
		for (int i = 0; i < nCount; ++i)
		{
			int nNewScore = EvaluateMove(pTestBoard, PossibleMoves[i], !bIsBlack, bIsBlackGood, nDepth - 1);
			if (nBestScore < nNewScore)
				nBestScore = nNewScore;
		}
	}

	return nBestScore;
	


	//// Move score.
	//int nScore = PositionStrategy[nMove];
	//if (bIsBlack != bIsBlackGood)
	//	nScore *= -1;
	//if (nScore == 10000 || nScore == -10000)
	//	return nScore * 10;

	//// Place Brick
	//_ASSERT(nActiveBoard < NUM_BOARDS);
	//char* pTestBoard = &pBoards[BOARDSIZE * nActiveBoard++];
	//memcpy(pTestBoard, pBoard, sizeof(char) * 64);
	//OnlyPlaceBrick(pTestBoard, nMove, bIsBlack);

	//// If end of search, return a score;
	//int nBoardScore = GetScore(pBoard, !bIsBlack);
	//if (bIsBlack == bIsBlackGood)
	//	nBoardScore *= -1;
	//nScore += nBoardScore;

	//if (nDepth == 0)
	//	return nScore;

	//
	//// Evaluate one step deeper;
	//int PossibleMoves[60];
	//memset(PossibleMoves, -1, sizeof(int) * 60);
	//int nCount = 0;
	//GetPossibleMoves(pTestBoard, !bIsBlack, PossibleMoves, nCount);

	//int nBestScore = nScore;
	//for (int i = 0; i < nCount; ++i)
	//{
	//	if (PossibleMoves[i] == 0 || PossibleMoves[i] == 7 || PossibleMoves[i] == 56 || PossibleMoves[i] == 63)
	//	{
	//		if (!bIsBlack == bIsBlackGood)
	//			nBestScore = 10000;
	//		else
	//			nBestScore = -10000;
	//	}
	//	else
	//	{
	//		int nTmpScore = nScore + EvaluateMove(pTestBoard, PossibleMoves[i], !bIsBlack, bIsBlackGood, nDepth - 1);
	//		if (nTmpScore > nBestScore)
	//			nBestScore = nTmpScore;
	//	}
	//}

	//return nBestScore;
}

void Setup()
{
	nDoInit = 0;
	pBoards = new char[BOARDSIZE * NUM_BOARDS];
	//for (int i = 0, nSize = BOARDSIZE*NUM_BOARDS; i < nSize; ++i)
		//pBoards[i] = -1;
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

		int PossibleMoves[60];
		memset(PossibleMoves, -1, sizeof(int) * 60);
		int nCount = 0;
		GetPossibleMoves(pCharBoard, bIsBlack, PossibleMoves, nCount);
		
		int nBestMove = -1;
		int nBestScore = -101;

		for (int i = 0; i < nCount; ++i)
		{
			char* pTestBoard = &pBoards[BOARDSIZE * nActiveBoard++];
			memcpy(pTestBoard, pCharBoard, sizeof(char) * 64);
			int nScore = EvaluateMove(pTestBoard, PossibleMoves[i], bIsBlack, bIsBlack, SEARCH_DEPTH);
			if (nBestScore < nScore)
			{
				nBestScore = nScore;
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