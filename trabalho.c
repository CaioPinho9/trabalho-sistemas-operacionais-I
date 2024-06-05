#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

// my adaptation of the COORD type that came from windows.h, aparently
typedef struct
{
	int X;
	int Y;
} COORD;

int x, missao = 0;
int originCannon = 53;

void gotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	printf("\033[%d;%dH", coord.Y, coord.X);
}

void detonate_bomb(int x, int y)
{
	gotoxy(x, y);
	printf("*");
	sleep(1);
	printf(" ");
	gotoxy(x, y - 1);
	printf("O");
	gotoxy(x - 1, y);
	printf("O O");
	gotoxy(x, y + 1);
	printf("O");
	sleep(1);
	gotoxy(x, y - 1);
	printf(" ");
	gotoxy(x - 1, y);
	printf("   ");
	gotoxy(x, y + 1);
	printf(" ");

	gotoxy(x, y - 2);
	printf("o");
	gotoxy(x - 2, y);
	printf("o   o");
	gotoxy(x, y + 2);
	printf("o");
	sleep(1);
	gotoxy(x, y - 2);
	printf(" ");
	gotoxy(x - 2, y);
	printf("     ");
	gotoxy(x, y + 2);
	printf(" ");
}

int is_out(int x, int y)
{
	if (x < 12 || x > 109 || y < 0 || y > 25)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void bomb(int x, int y, int direction)
{
	switch (direction)
	{
	case -2:
		x -= 2;
		break;
	case -1:
		x--;
		break;
	case 0:
		y--;
		break;
	case 1:
		x++;
		break;
	case 2:
		x += 3;
		}
	for (; y > 0; y--)
	{
		switch (direction)
		{
		case -2:
			x -= 2;
			y++;
			break;
		case -1:
			x--;
			break;
		case 1:
			x++;
			break;
		case 2:
			x += 2;
			y++;
		}
		if (is_out(x, y))
			break;

		gotoxy(x, y);
		printf("o\n");
		sleep(1);
		gotoxy(x, y);
		printf(" \n");
	}
};

void startCannon(int x, int y, int cannon)
{
	time_t t;
	gotoxy(x, y + 1);
	printf(" ___+--+___");
	gotoxy(x, y + 2);
	printf("/          \\");
	gotoxy(x, y);
	printf("     ||");
}

void shotCannon(int x, int y)
{
	int direction;
	time_t t;
	gotoxy(x, y + 1);
	printf(" ___+--+___");
	gotoxy(x, y + 2);
	printf("/          \\");
	gotoxy(x, y);
	srand((unsigned)time(&t));
	for (int rocket = 3; rocket > 0; rocket--)
	{
		direction = (rand() % 5) - 2;
		gotoxy(0, 28);
		printf("%6d  %6d\n", rocket, direction);
		gotoxy(x, y);
		switch (direction)
		{
		case -2:
			printf("    -|| \n");
			break;
		case -1:
			printf("     \\\\ \n");
			break;
		case 0:
			printf("     || ");
			break;
		case 1:
			printf("     // ");
			break;
		case 2:
			printf("     ||- ");
		}
		bomb(x + 5, y - 1, direction);
	}
}

void plataformLeft()
{
	int line;
	gotoxy(0, 7);
	printf("----------+\n");
	gotoxy(0, 8);
	printf("|         /\n");
	for (line = 9; line < 25; line++)
	{
		gotoxy(0, line);
		printf("|        |\n");
	}
}

void plataformRight()
{
	int line;
	gotoxy(108, 7);
	printf("+-----------\n");
	gotoxy(109, 8);
	printf("\\         \n");
	for (line = 9; line < 25; line++)
	{
		gotoxy(109, line);
		printf(" |        | \n");
	}
}

void bridge()
{
	gotoxy(30, 25);
	printf("+-----------------------+\n");
	gotoxy(30, 26);
	printf("|         ---           |\n");
	gotoxy(30, 27);
	printf("|        /   \\          |\n");
	gotoxy(30, 28);
	printf("|       /     \\         |\n");
	gotoxy(30, 29);
	printf("|      /       \\        |\n");
}
void deposit()
{
	gotoxy(0, 20);
	printf("|\\|\\|\\|\\|\\|\\|\\|\\|\\|\\|\\");
	gotoxy(0, 21);
	printf("|                     |");
	gotoxy(0, 22);
	printf("|       +----+        |");
	gotoxy(0, 23);
	printf("|       |    |        |");
	gotoxy(0, 24);
	printf("|       |    |        |");
}

int main()
{
	int column = 5;
	int line = 3;
	int k = 0;
	system("clear");
	gotoxy(0, 25);
	for (column = 0; column < 120; column++)
	{
		printf("^");
	}
	startCannon(originCannon, 22, 0);
	bridge();
	deposit();
	plataformLeft();
	plataformRight();

	shotCannon(originCannon, 22);

	for (int i = 0; i < 7; i++)
	{
		detonate_bomb((rand() % 70) + 10, (rand() % 10) + 4);
		sleep(1);
	}
}
