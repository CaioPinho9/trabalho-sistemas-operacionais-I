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
	int destroyed;
} ROCKET_ARGS;

typedef struct
{
	int X;
	int Y;
} COORD;

int ship_speed;
int rocket_capacity;
int recharge_cooldown = 0;
int shoot_cooldown = 0;

int current_rocket = 0;

const int cannon_x = 53;
const int cannon_y = 21;

int rocket_index = 0;

ROCKET_ARGS *rockets[10];
pthread_t rocket_threads[10];
pthread_t input_thread;
pthread_t cannon_thread;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int direction;

void mvprint(int x, int y, char *string)
{
	pthread_mutex_lock(&mutex);
	COORD coord;
	coord.X = x;
	coord.Y = y;
	printf("\033[%d;%dH", coord.Y, coord.X);
	printf("%s", string);
	pthread_mutex_unlock(&mutex);
}

void detonate_rocket(int x, int y)
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
	if (x < 23 || x > 109 || y < 0 || y > 25)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void *rocket_thread_function(void *args)
{
	ROCKET_ARGS *rocket_args = (ROCKET_ARGS *)args;

	switch (rocket_args->direction)
	{
	case -2:
		rocket_args->X -= 2;
		break;
	case -1:
		rocket_args->X--;
		break;
	case 0:
		rocket_args->Y--;
		break;
	case 1:
		rocket_args->X++;
		break;
	case 2:
		rocket_args->X += 3;
		break;
	}
	for (; rocket_args->Y > 0; rocket_args->Y--)
	{
		if (rocket_args->destroyed)
			break;

		switch (rocket_args->direction)
		{
		case -2:
			rocket_args->X -= 2;
			rocket_args->Y++;
			break;
		case -1:
			rocket_args->X--;
			break;
		case 1:
			rocket_args->X++;
			break;
		case 2:
			rocket_args->X += 2;
			rocket_args->Y++;
			break;
		}
		if (is_out_range(rocket_args->X, rocket_args->Y))
			break;

		mvprint(rocket_args->X, rocket_args->Y, "o\n");
		usleep(100000);
		mvprint(rocket_args->X, rocket_args->Y, " \n");
	}
	free(rocket_args);
	return NULL;
}

void shoot_rocket(int x, int y, int direction)
{
	ROCKET_ARGS *args = (ROCKET_ARGS *)malloc(sizeof(ROCKET_ARGS));
	args->X = x + 5;
	args->Y = y - 1;
	args->direction = direction;
	rockets[rocket_index % (rocket_capacity * 2)] = args;
	pthread_create(&rocket_threads[rocket_index % (rocket_capacity * 2)], NULL, rocket_thread_function, (void *)args);
	rocket_index++;
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

			char rocket_str[3];
			sprintf(rocket_str, "%02d", current_rocket);
			mvprint(4, 26, rocket_str);

			if (ch == ' ' && current_rocket > 0 && !recharge_cooldown && !shoot_cooldown)
			{
				shoot_rocket(cannon_x, cannon_y, direction);
				current_rocket--;
				shoot_cooldown = 25;
			}
			if (ch == 'e' && current_rocket < rocket_capacity && !recharge_cooldown)
			{
				current_rocket++;
				recharge_cooldown = 50;
			}

			if (ch == 'q')
				exit(0);
		}
		usleep(10000);
		if (recharge_cooldown > 0)
			recharge_cooldown--;
		if (shoot_cooldown > 0)
			shoot_cooldown--;
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

		for (size_t i = 0; i < 10; i++)
		{
			if (rockets[i] != NULL)
			{
				char rocket_str[50];
				sprintf(rocket_str, "Rocket %d: X=%d Y=%d\n", i, rockets[i]->X, rockets[i]->Y);
				mvprint(0, i + 1, rocket_str);
			}
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

void configure_difficulty(int difficulty)
{
	switch (difficulty)
	{
	case 1:
		ship_speed = 1;
		rocket_capacity = 10;
		break;
	case 2:

		ship_speed = 2;
		rocket_capacity = 8;
		break;
	case 3:
		ship_speed = 3;
		rocket_capacity = 6;
		break;
	}
	current_rocket = rocket_capacity;
}

int main()
{
	system("clear");
	printf("Welcome to the game!\n");
	printf("Please select the difficulty level (1-3): ");
	int difficulty = 1;
	scanf("%d", &difficulty);
	printf("You have selected difficulty level %d\n", difficulty);

	configure_difficulty(difficulty);

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
