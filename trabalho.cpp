#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>

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
const int cannonX = 53;
const int cannonY = 22;

int current_bomb = 0;
pthread_t bomb_threads[5];
pthread_t cannon_thread;
pthread_t input_thread;

void goToXY(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	printf("\033[%d;%dH", coord.Y, coord.X);
}

void detonateBomb(int x, int y)
{
	goToXY(x, y);
	printf("*");
	sleep(1);
	printf(" ");
	goToXY(x, y - 1);
	printf("O");
	goToXY(x - 1, y);
	printf("O O");
	goToXY(x, y + 1);
	printf("O");
	sleep(1);
	goToXY(x, y - 1);
	printf(" ");
	goToXY(x - 1, y);
	printf("   ");
	goToXY(x, y + 1);
	printf(" ");

	goToXY(x, y - 2);
	printf("o");
	goToXY(x - 2, y);
	printf("o   o");
	goToXY(x, y + 2);
	printf("o");
	sleep(1);
	goToXY(x, y - 2);
	printf(" ");
	goToXY(x - 2, y);
	printf("     ");
	goToXY(x, y + 2);
	printf(" ");
}

int isOutRange(int x, int y)
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

void *bombThreadFunction(void *args)
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
		if (isOutRange(x, y))
			break;

		goToXY(x, y);
		printf("o\n");
		usleep(100000);
		goToXY(x, y);
		printf(" \n");
	}
	return NULL;
}

void startCannon(int x, int y, int cannon)
{
	goToXY(x, y + 1);
	printf(" ___+--+___");
	goToXY(x, y + 2);
	printf("/          \\");
	goToXY(x, y);
	printf("     ||");
}

void shootBomb(int x, int y, int direction)
{
	BOMB_ARGS *args = (BOMB_ARGS *)malloc(sizeof(BOMB_ARGS));
	args->X = x + 5;
	args->Y = y - 1;
	args->direction = direction;
	pthread_create(&bomb_threads[current_bomb % (sizeof(bomb_threads) / sizeof(pthread_t))], NULL, bombThreadFunction, (void *)args);
	current_bomb++;
}

void blockTerminalInput()
{
	struct termios new_termios;
	tcgetattr(0, &new_termios);
	new_termios.c_lflag &= ~ICANON;
	new_termios.c_lflag &= ~ECHO;
	tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}

void getCannonDirection(int *direction)
{
	int ch;
	while (kbhit())
	{
		ch = getchar();
		if (ch == 'w')
			*direction = 0;
		if (ch == 'a')
			*direction--;
		if (ch == 'd')
			*direction++;
		if (*direction > 2)
			*direction = 2;
		if (*direction < -2)
			*direction = -2;

		goToXY(0, 26);
		printf("%d", *direction);

		if (ch == ' ')
			shootBomb(cannonX, cannonY, *direction);
		if (ch == 'q')
			exit(0);
		usleep(100000);
	}
}

void *cannonThreadFunction(void *arg)
{
	int x = cannonX;
	int y = cannonY;
	int direction = 0;
	time_t t;
	char ch;
	goToXY(x, y + 1);
	printf(" ___+--+___");
	goToXY(x, y + 2);
	printf("/          \\");
	goToXY(x, y);
	srand((unsigned)time(&t));
	while (1)
	{
		getCannonDirection(&direction);

		goToXY(x, y);
		if (direction == -2)
		{
			printf("    =/\\ \n");
		}
		else if (direction == -1)
		{
			printf("     \\\\ \n");
		}
		else if (direction == 0)
		{
			printf("     || ");
		}
		else if (direction == 1)
		{
			printf("     // ");
		}
		else if (direction == 2)
		{
			printf("     /\\= ");
		}

		usleep(100000); // Small delay to reduce CPU usage
	}
	return NULL;
}

void plataformLeft()
{
	int line;
	goToXY(0, 7);
	printf("----------+\n");
	goToXY(0, 8);
	printf("|         /\n");
	for (line = 9; line < 25; line++)
	{
		goToXY(0, line);
		printf("|        |\n");
	}
}

void plataformRight()
{
	int line;
	goToXY(108, 7);
	printf("+-----------\n");
	goToXY(109, 8);
	printf("\\         \n");
	for (line = 9; line < 25; line++)
	{
		goToXY(109, line);
		printf(" |        | \n");
	}
}

void bridge()
{
	goToXY(30, 25);
	printf("+-----------------------+\n");
	goToXY(30, 26);
	printf("|         ---           |\n");
	goToXY(30, 27);
	printf("|        /   \\          |\n");
	goToXY(30, 28);
	printf("|       /     \\         |\n");
	goToXY(30, 29);
	printf("|      /       \\        |\n");
}

void deposit()
{
	goToXY(0, 20);
	printf("|\\|\\|\\|\\|\\|\\|\\|\\|\\|\\|\\");
	goToXY(0, 21);
	printf("|                     |");
	goToXY(0, 22);
	printf("|       +----+        |");
	goToXY(0, 23);
	printf("|       |    |        |");
	goToXY(0, 24);
	printf("|       |    |        |");
}

int main()
{
	int column = 5;
	int line = 3;
	int k = 0;
	system("clear");
	blockTerminalInput();
	goToXY(0, 25);
	for (column = 0; column < 120; column++)
	{
		printf("^");
	}
	startCannon(cannonX, cannonY, 0);

	bridge();
	deposit();
	plataformLeft();
	plataformRight();

	pthread_create(&input_thread, NULL, cannonThreadFunction, NULL);

	pthread_join(input_thread, NULL);

	return 0;
}
