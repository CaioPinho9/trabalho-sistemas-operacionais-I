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
const int cannon_x = 53;
const int cannon_y = 21;

int current_bomb = 0;
pthread_t bomb_threads[5];
pthread_t input_thread;
pthread_t cannon_thread;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int direction;

void mvprint(int x, int y, char *string)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	pthread_mutex_lock(&mutex);
	printf("\033[%d;%dH", coord.Y, coord.X);
	printf("%s", string);
	pthread_mutex_unlock(&mutex);
}

void detonate_bomb(int x, int y)
{
	mvprint(x, y, "*");
	sleep(1);
	mvprint(x, y, " ");
	mvprint(x, y - 1, "O");
	mvprint(x - 1, y, "O O");
	mvprint(x, y + 1, "O");
	sleep(1);
	mvprint(x, y - 1, " ");
	mvprint(x - 1, y, "   ");
	mvprint(x, y + 1, " ");

	mvprint(x, y - 2, "o");
	mvprint(x - 2, y, "o   o");
	mvprint(x, y + 2, "o");
	sleep(1);
	mvprint(x, y - 2, " ");
	mvprint(x - 2, y, "     ");
	mvprint(x, y + 2, " ");
}

int is_out_range(int x, int y)
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

void *bomb_thread_function(void *args)
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
		if (is_out_range(x, y))
			break;

		mvprint(x, y, "o\n");
		usleep(100000);
		mvprint(x, y, " \n");
	}
	return NULL;
}

void shoot_bomb(int x, int y, int direction)
{
	BOMB_ARGS *args = (BOMB_ARGS *)malloc(sizeof(BOMB_ARGS));
	args->X = x + 5;
	args->Y = y - 1;
	args->direction = direction;
	pthread_create(&bomb_threads[current_bomb % (sizeof(bomb_threads) / sizeof(pthread_t))], NULL, bomb_thread_function, (void *)args);
	current_bomb++;
}

void block_terminal_input()
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

void *input_thread_function(void *args)
{
	int ch;
	while (1)
	{
		while (kbhit())
		{
			ch = getchar();
			if (ch == 'w')
				direction = 0;
			if (ch == 'a')
				direction--;
			if (ch == 'd')
				direction++;
			if (direction > 2)
				direction = 2;
			if (direction < -2)
				direction = -2;

			char direction_str[3];
			sprintf(direction_str, "%02d", direction);
			mvprint(1, 26, direction_str);

			if (ch == ' ')
				shoot_bomb(cannon_x, cannon_y, direction);
			if (ch == 'q')
				exit(0);
		}
		usleep(10000);
	}
	return NULL;
}

void *cannon_thread_function(void *args)
{
	int x = cannon_x;
	int y = cannon_y;
	mvprint(x, y + 1, " ___+--+___");
	mvprint(x, y + 2, "/          \\");
	while (1)
	{
		if (direction == -2)
		{
			mvprint(x, y, "    =/\\ \n");
		}
		else if (direction == -1)
		{
			mvprint(x, y, "     \\\\ \n");
		}
		else if (direction == 0)
		{
			mvprint(x, y, "     || ");
		}
		else if (direction == 1)
		{
			mvprint(x, y, "     // ");
		}
		else if (direction == 2)
		{
			mvprint(x, y, "     /\\= ");
		}

		usleep(10000); // Small delay to reduce CPU usage
	}
	return NULL;
}

void plataform_left()
{
	int line;
	mvprint(0, 7, "----------+\n");
	mvprint(0, 8, "|         /\n");
	for (line = 9; line < 25; line++)
	{
		mvprint(0, line, "|        |\n");
	}
}

void plataform_right()
{
	int line;
	mvprint(108, 7, "+-----------\n");
	mvprint(109, 8, "\\         \n");
	for (line = 9; line < 25; line++)
	{
		mvprint(109, line, " |        | \n");
	}
}

void bridge()
{
	mvprint(30, 25, "+-----------------------+\n");
	mvprint(30, 26, "|         ---           |\n");
	mvprint(30, 27, "|        /   \\          |\n");
	mvprint(30, 28, "|       /     \\         |\n");
	mvprint(30, 29, "|      /       \\        |\n");
}

void deposit()
{
	mvprint(0, 20, "|\\|\\|\\|\\|\\|\\|\\|\\|\\|\\|\\");
	mvprint(0, 21, "|                     |");
	mvprint(0, 22, "|       +----+        |");
	mvprint(0, 23, "|       |    |        |");
	mvprint(0, 24, "|       |    |        |");
}

int main()
{
	int column = 5;
	system("clear");
	block_terminal_input();
	for (column = 0; column < 120; column++)
	{
		mvprint(0 + column, 25, "^");
	}
	bridge();
	deposit();
	plataform_left();
	plataform_right();

	pthread_create(&input_thread, NULL, input_thread_function, NULL);
	pthread_create(&cannon_thread, NULL, cannon_thread_function, NULL);

	pthread_join(input_thread, NULL);
	pthread_join(cannon_thread, NULL);

	return 0;
}
