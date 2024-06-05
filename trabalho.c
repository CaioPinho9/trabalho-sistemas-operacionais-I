#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

typedef struct
{
	int X;
	int Y;
	int direction;
} BOMB_ARGS;

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

void *bomb_thread(void *args)
{
	BOMB_ARGS *bomb_args = (BOMB_ARGS *)args;
	int x = bomb_args->X;
	int y = bomb_args->Y;
	int direction = bomb_args->direction;
	free(bomb_args);

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
		break;
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
			break;
		}
		if (is_out(x, y))
			break;

		gotoxy(x, y);
		printf("o\n");
		sleep(1);
		gotoxy(x, y);
		printf(" \n");
	}
	return NULL;
}

void startCannon(int x, int y, int cannon)
{
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
	pthread_t threads[3];
	for (int rocket = 0; rocket < 3; rocket++)
	{
		sleep(1);
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

		BOMB_ARGS *args = malloc(sizeof(BOMB_ARGS));
		args->X = x + 5;
		args->Y = y - 1;
		args->direction = direction;
		pthread_create(&threads[rocket], NULL, bomb_thread, (void *)args);
	}

	for (int rocket = 0; rocket < 3; rocket++)
	{
		pthread_join(threads[rocket], NULL);
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

	return 0;
}
