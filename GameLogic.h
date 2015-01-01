
int Directions[8] = { -8, -7, 1, 9, 8, 7, -1, -9 };
enum Direction
{
	Up,
	UpRight,
	Right,
	DownRight,
	Down,
	DownLeft,
	Left,
	UpLeft
};

static Direction GetDirection(int nDirection)
{
	Direction dir = Up;
	if (nDirection == -8)
		dir = Up;
	else if (nDirection == -7)
		dir = UpRight;
	else if (nDirection == +1)
		dir = Right;
	else if (nDirection == 9)
		dir = DownRight;
	else if (nDirection == 8)
		dir = Down;
	else if (nDirection == 7)
		dir = DownLeft;
	else if (nDirection == -1)
		dir = Left;
	else if (nDirection == -9)
		dir = UpLeft;

	return dir;
}

static bool BorderCheck(int nMove, int nDirection)
{
	Direction Dir = GetDirection(nDirection);

	if (nMove < 8 && (Dir == UpLeft || Dir == Up || Dir == UpRight))
		return false;
	if ((nMove % 8 == 7) && (Dir == UpRight || Dir == Right || Dir == DownRight))
		return false;
	if (nMove > 56 && (Dir == DownLeft || Dir == Down || Dir == DownRight))
		return false;
	if (nMove % 8 == 0 && (Dir == UpLeft || Dir == Left || Dir == DownLeft))
		return false;

	return true;
}

static bool CheckDirection(char* pBoard, int nMove, int nDirection, bool bIsBlack, bool bDoFlip = false)
{
	int nPlayer = (bIsBlack ? 1 : 2);
	int nEnemy = (bIsBlack ? 2 : 1);

	if (nMove < 0 || nMove >= 64)
		return false;
	if (pBoard[nMove] == 0)
		return false;
	if (pBoard[nMove] == nPlayer)
		return true;

	Direction Dir = GetDirection(nDirection);

	if (nMove < 8 && (Dir == UpLeft || Dir == Up || Dir == UpRight))
		return false;
	if ((nMove % 8 == 7) && (Dir == UpRight || Dir == Right || Dir == DownRight))
		return false;
	if (nMove > 56 && (Dir == DownLeft || Dir == Down || Dir == DownRight))
		return false;
	if (nMove % 8 == 0 && (Dir == UpLeft || Dir == Left || Dir == DownLeft))
		return false;


	if (CheckDirection(pBoard, nMove + nDirection, nDirection, bIsBlack, bDoFlip))
	{
		if (bDoFlip)
			pBoard[nMove] = nPlayer;

		return true;
	}

	return false;
}

static bool CheckDirection(const char* pBoard, int nMove, int nDirection, bool bIsBlack)
{
	int nPlayer = (bIsBlack ? 1 : 2);
	int nEnemy = (bIsBlack ? 2 : 1);

	if (nMove < 0 || nMove >= 64)
		return false;
	if (pBoard[nMove] == 0)
		return false;
	if (pBoard[nMove] == nPlayer)
		return true;

	Direction Dir = GetDirection(nDirection);

	if (nMove < 8 && (Dir == UpLeft || Dir == Up || Dir == UpRight))
		return false;
	if ((nMove % 8 == 7) && (Dir == UpRight || Dir == Right || Dir == DownRight))
		return false;
	if (nMove > 56 && (Dir == DownLeft || Dir == Down || Dir == DownRight))
		return false;
	if (nMove % 8 == 0 && (Dir == UpLeft || Dir == Left || Dir == DownLeft))
		return false;


	if (CheckDirection(pBoard, nMove + nDirection, nDirection, bIsBlack))
		return true;

	return false;
}

static bool IsValidMove(const char* pBoard, int nMove, bool bIsBlack)
{
	if (pBoard[nMove] > 0)
		return false;

	int nPlayer = (bIsBlack ? 1 : 2);
	int nEnemy = (bIsBlack ? 2 : 1);
	int nSize = 64;


	if (nMove < 0 || nMove >= nSize)
		return false;
	if (pBoard[nMove] != 0)
		return false;

	for (int i = 0; i < 8; ++i)
	{
		if (nMove % 8 == 0 && i >= 5)
			continue;
		if (nMove % 8 == 7 && i >= 1 && i <= 3)
			continue;

		int nCheck = nMove + Directions[i];
		if (nCheck >= 0 && nCheck < nSize)
		{
			if (pBoard[nCheck] == nEnemy && CheckDirection(pBoard, nCheck, Directions[i], bIsBlack))
				return true;
		}
	}

	return false;
}

static bool HasAnyValidMoves(char* pBoard, bool bIsBlack)
{
	for (int i = 0; i < 64; ++i)
	{
		if (IsValidMove(pBoard, i, bIsBlack))
			return true;
	}
	return false;
}

static void OnlyPlaceBrick(char* pBoard, int nMove, bool bIsBlack)
{
	if (nMove >= 0)
	{
		int Directions[8] = { -8, -8 + 1, 1, +8 + 1, +8, +8 - 1, -1, -8 - 1 };
		for (int i = 0; i < 8; ++i)
		{
			if (BorderCheck(nMove, Directions[i]))
				CheckDirection(pBoard, nMove + Directions[i], Directions[i], bIsBlack, true);
		}
		pBoard[nMove] = (bIsBlack ? 1 : 2);
	}
}