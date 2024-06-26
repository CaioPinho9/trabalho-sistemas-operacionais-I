#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#define MAX_ROCKETS 10
#define MAX_SHIPS 10

#define COLISION_UPDATE_INTERVAL 0.05

int game_over = 0;

typedef struct
{
	int X;
	int Y;
	int direction;
	int destroyed;
	int index;
} ROCKET_ARGS;

typedef struct
{
	int X;
	int Y;
} COORD;

typedef struct
{
	int X;
	float Y;
	int destroyed;
	int index;
} SHIP_ARGS;

float ship_speed;
int ship_spawn_time = 5;
int rocket_capacity;
int recharge_cooldown = 0;
int shoot_cooldown = 0;

int current_rocket = 0;

const int floor_level = 26;
const int cannon_x = 53;
const int cannon_y = floor_level - 4;

int rocket_index = 0;
int ship_index = 0;

ROCKET_ARGS *rockets[MAX_ROCKETS];
SHIP_ARGS *ships[MAX_SHIPS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int direction;

int total_ships;
int total_ships_destroyed;
int total_ships_land;
int remaining_ships;

void mvprint(int x, int y, char *string)
{
	if (game_over)
		return;
	pthread_mutex_lock(&mutex);
	COORD coord;
	coord.X = x;
	coord.Y = y;
	printf("\033[%d;%dH", coord.Y, coord.X);
	printf("%s", string);
	pthread_mutex_unlock(&mutex);
}

int is_out_range(int x, int y)
{
	return (x < 23 || x > 109 || y < 0 || y > floor_level);
}
void *rocket_thread_function(void *arg)
{
	ROCKET_ARGS *rocket_args = (ROCKET_ARGS *)arg;

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

	return NULL;
}

void *ship_thread_function(void *arg)
{
	SHIP_ARGS *ship_args;
	if (ships[ship_index] == NULL)
	{
		ship_args = (SHIP_ARGS *)malloc(sizeof(SHIP_ARGS));
		ships[ship_index] = ship_args;
	}
	else
	{
		ship_args = ships[ship_index];
	}

	int pos_x = (rand() % 83) + 25;
	ship_args->X = pos_x;
	ship_args->Y = 0;
	ship_args->destroyed = 0;
	ship_args->index = ship_index;
	ship_index++;
	ship_index %= MAX_SHIPS;

	for (; ship_args->Y < floor_level - 4; ship_args->Y += ship_speed)
	{
		if (ship_args->destroyed)
		{
			total_ships_destroyed++;
			return NULL;
		}
		int y = ceil(ship_args->Y);
		mvprint(ship_args->X, y, ">V<\n");
		mvprint(ship_args->X, y + 1, "<=>\n");
		mvprint(ship_args->X, y + 2, " v \n");
		usleep(100000);
		mvprint(ship_args->X, y, "   \n");
		mvprint(ship_args->X, y + 1, "   \n");
		mvprint(ship_args->X, y + 2, "   \n");
	}
	total_ships_land++;
	return NULL;
}

void shoot_rocket(int x, int y, int direction)
{
	if (rocket_index >= MAX_ROCKETS)
	{
		rocket_index = 0;
	}

	ROCKET_ARGS *rocket;
	if (rockets[rocket_index] == NULL)
	{
		rocket = (ROCKET_ARGS *)malloc(sizeof(ROCKET_ARGS));
	}
	else
	{
		rocket = rockets[rocket_index];
		rocket->destroyed = 0;
	}
	rocket->X = x + 5;
	rocket->Y = y - 1;
	rocket->direction = direction;
	rocket->index = rocket_index;
	rockets[rocket_index] = rocket;

	pthread_t rocket_thread;
	pthread_create(&rocket_thread, NULL, rocket_thread_function, rocket);
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
	while (!game_over)
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
			mvprint(1, floor_level + 1, direction_str);

			char rocket_str[3];
			sprintf(rocket_str, "%02d", current_rocket);
			mvprint(4, floor_level + 1, rocket_str);

			if (ch == ' ' && current_rocket > 0 && !recharge_cooldown && !shoot_cooldown)
			{
				shoot_rocket(cannon_x, cannon_y, direction);
				current_rocket--;
				shoot_cooldown = 25;
			}
			if (ch == 'e' && current_rocket < rocket_capacity && !recharge_cooldown)
			{
				current_rocket++;
				recharge_cooldown = 40;
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
	while (!game_over)
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

		mvprint(x, y + 1, " ___+--+___");
		mvprint(x, y + 2, "/          \\");

		usleep(10000); // Small delay to reduce CPU usage
	}
	return NULL;
}

void plataform_left()
{
	int line;
	mvprint(0, floor_level - 20, "----------+\n");
	mvprint(0, floor_level - 19, "|         /\n");
	for (line = floor_level - 18; line < floor_level - 2; line++)
	{
		mvprint(0, line, "|        |\n");
	}
}

void plataform_right()
{
	int line;
	mvprint(108, floor_level - 20, "+-----------\n");
	mvprint(109, floor_level - 19, "\\         |\n");
	for (line = floor_level - 18; line < floor_level - 2; line++)
	{
		mvprint(109, line, " |        | \n");
	}
}

void bridge()
{
	mvprint(30, floor_level, "+-----------------------+\n");
	mvprint(30, floor_level + 1, "|         ---           |\n");
	mvprint(30, floor_level + 2, "|        /   \\          |\n");
	mvprint(30, floor_level + 3, "|       /     \\         |\n");
	mvprint(30, floor_level + 4, "|      /       \\        |\n");
}

void deposit()
{
	mvprint(0, floor_level - 7, "|\\|\\|\\|\\|\\|\\|\\|\\|\\|\\|\\");
	mvprint(0, floor_level - 6, "|                     |");
	mvprint(0, floor_level - 5, "|       +----+        |");
	mvprint(0, floor_level - 4, "|       |    |        |");
	mvprint(0, floor_level - 3, "|       |    |        |");
}

void configure_difficulty(int difficulty)
{
	switch (difficulty)
	{
	case 1:
		ship_speed = 0.25;
		total_ships = 2;
		rocket_capacity = 10;
		ship_spawn_time = 5;
		break;
	case 2:
		ship_speed = 0.5;
		total_ships = 6;
		rocket_capacity = 8;
		ship_spawn_time = 4;
		break;
	case 3:
		ship_speed = 0.75;
		total_ships = 12;
		rocket_capacity = 6;
		ship_spawn_time = 3;
		break;
	}
	current_rocket = rocket_capacity;
}

int main()
{
	srand(time(NULL));
	system("clear");
	printf("Welcome to the game!\n");
	printf("Please select the difficulty level (1-3): ");
	int difficulty = 1;
	scanf("%d", &difficulty);
	printf("You have selected difficulty level %d\n", difficulty);

	clock_t last_ship_update = clock();
	clock_t last_colision_update = clock();

	configure_difficulty(difficulty);

	int column = 5;
	system("clear");
	block_terminal_input();
	for (column = 0; column < 120; column++)
	{
		mvprint(0 + column, floor_level, "^");
	}
	bridge();
	deposit();
	plataform_left();
	plataform_right();

	pthread_t input_thread;
	pthread_t cannon_thread;

	pthread_create(&input_thread, NULL, input_thread_function, NULL);
	pthread_create(&cannon_thread, NULL, cannon_thread_function, NULL);

	unsigned count = 0;
	remaining_ships = total_ships;

	while (!game_over)
	{
		clock_t current_time = clock();
		double time_elapsed_ship = (double)(current_time - last_ship_update) / CLOCKS_PER_SEC;
		double time_elapsed_colision = (double)(current_time - last_colision_update) / CLOCKS_PER_SEC;

		if (time_elapsed_ship >= ship_spawn_time && remaining_ships > 0)
		{
			pthread_t ship_thread;
			pthread_create(&ship_thread, NULL, ship_thread_function, NULL);
			remaining_ships--;
			last_ship_update = current_time;
		}

		if (time_elapsed_colision >= COLISION_UPDATE_INTERVAL)
		{
			for (size_t ship_index = 0; ship_index < MAX_SHIPS; ship_index++)
			{
				SHIP_ARGS *ship = ships[ship_index];
				if (ship == NULL || ship->destroyed)
					continue;

				char ship_str[50];
				sprintf(ship_str, "Ship %d: X=%d Y=%d\n", (int)ship_index, ship->X, ship->Y);
				mvprint(50, ship_index + 1, ship_str);

				for (size_t rocket_index = 0; rocket_index < MAX_ROCKETS; rocket_index++)
				{
					ROCKET_ARGS *rocket = rockets[rocket_index];
					if (rocket == NULL || rocket->destroyed)
						continue;

					char rocket_str[50];
					sprintf(rocket_str, "Rocket %d: X=%d Y=%d\n", (int)rocket_index, rocket->X, rocket->Y);
					mvprint(0, rocket_index + 1, rocket_str);

					if (ship->X <= rocket->X && rocket->X <= ship->X + 2 &&
						ship->Y <= rocket->Y && rocket->Y <= ship->Y + 2)
					{
						char ship_str[50];
						sprintf(ship_str, "Ship %d: X=%d Y=%d\n", (int)ship_index, ship->X, ship->Y);
						mvprint(100, ship_index + 1, ship_str);

						char rocket_str[50];
						sprintf(rocket_str, "Rocket %d: X=%d Y=%d\n", (int)rocket_index, rocket->X, rocket->Y);
						mvprint(100, rocket_index + 1, rocket_str);

						rocket->destroyed = 1;
						ship->destroyed = 1;
					}
				}
			}

			if (total_ships_land / (float)total_ships >= 0.5)
			{
				game_over = 1;
				system("clear");
				printf("You lose!\n");
			}
			else if (total_ships_destroyed / (float)total_ships >= 0.5)
			{
				game_over = 1;
				system("clear");
				printf("You win!\n");
			}

			last_colision_update = current_time;
		}
	}

	pthread_join(input_thread, NULL);
	pthread_join(cannon_thread, NULL);

	return 0;
}
