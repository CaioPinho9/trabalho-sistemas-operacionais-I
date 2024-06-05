#include <stdio.h>
#include <stdlib.h>
// commented the windows.h lib call, because I'm using linux
//#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

// my adaptation of the COORD type that came from windows.h, aparently
typedef struct  {
    int X;
    int Y;
} COORD;


int x, missao=0;
int origemCanhao0 = 53, origemCanhao1 = 93;
int contaRefem = 10;
int contaResgatado = 0;
int posicaoHelice = 0;
int posicaRoda[2] = {0, 0};
int canhaoMovendo[2] = {0, 0};



//Função gotoxy
void gotoxy(int x, int y)
{
  COORD coord;
  coord.X = x;
  coord.Y = y;
  printf("\033[%d;%dH", coord.Y, coord.X);
}

void explode_bomba(int x, int y){
	gotoxy(x, y);
	printf("*");
	sleep(1);
	printf(" ");
	gotoxy(x, y-1);
	printf("O");
	gotoxy(x-1, y);
	printf("O O");
	gotoxy(x, y+1);
	printf("O");
	sleep(1);
	gotoxy(x, y-1);
	printf(" ");
	gotoxy(x-1, y);
	printf("   ");
	gotoxy(x, y+1);
	printf(" ");


	gotoxy(x, y-2);
	printf("o");
	gotoxy(x-2, y);
	printf("o   o");
	gotoxy(x, y+2);
	printf("o");
	sleep(1);
	gotoxy(x, y-2);
	printf(" ");
	gotoxy(x-2, y);
	printf("     ");
	gotoxy(x, y+2);
	printf(" ");
}


void bomba(int x, int y, int posicao){
	for (; y > 0; y--){
		switch(posicao){
			case -2: x -= 2; y++; break;
    		case -1: x--; break;
    		case 0: x; break;
    		case 1: x++; break;
			case 2: x+=3; y++;
		}
		gotoxy(x, y);
		printf("o\n");
		sleep(1);
		gotoxy(x, y);
		printf(" \n");
   }
}

void inicializaCanhao(int x, int y, int canhao){
	time_t  t;
	gotoxy(x, y+1);
	printf(" ___+--+___");
	gotoxy(x, y+2);
	printf("/          \\");
	gotoxy(x, y);
	printf("     ||");
}

void canhaoAtira(int x, int y){
	int posicao;
	time_t  t;
	gotoxy(x, y+1);
	printf(" ___+--+___");
	gotoxy(x, y+2);
	printf("/          \\");
	gotoxy(x, y);
	srand((unsigned) time(&t));
	for (int foguete = 3; foguete > 0; foguete--){
		posicao =  (rand() % 5)-2;
		gotoxy(0, 28);
		printf("%6d  %6d\n", foguete, posicao);
		gotoxy(x, y);
    	switch(posicao){
    		case -2: printf("    -||\n"); break;
    		case -1: printf("     \\\\\n"); break;
    		case 0: printf("     ||"); break;
    		case 1: printf("     //"); break;
    		case 2: printf("     ||-");
		}
		bomba(x + 5, y - 1, posicao);
	}
}

void plataformaE(void){
	int linha;
	gotoxy(0, 7);
	printf("----------+\n");
	gotoxy(0, 8);
	printf("|         /\n");
	for(linha=9; linha < 25; linha++){
		gotoxy(0, linha);
		printf("|        |\n");
	}
}

void plataformaD(void){
	int linha;
	gotoxy(108, 7);
	printf("+-----------\n");
	gotoxy(109, 8);
	printf("\\         \n");
	for(linha=9; linha < 25; linha++){
		gotoxy(109, linha);
		printf(" |        | \n");
	}
}

void ponte(void){
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
void deposito(void){
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

int main(){
  int coluna = 5;
  int linha  = 3;
  int k=0;
  system("clear");
  gotoxy(0, 25);
  for(coluna=0; coluna<120; coluna++){
  	printf("^");
  }
  inicializaCanhao(origemCanhao0, 22, 0);
  ponte();
  deposito();
  plataformaE();
  plataformaD();

  canhaoAtira(origemCanhao0, 22); // Canhao Dispara
  sleep(1); // Carregando canhao 0
  canhaoAtira(origemCanhao0, 22); // Canhao 0 Dispara

  for(int i = 0; i < 7; i++){ // Bombas explodem no ar
  	 explode_bomba((rand() % 70)+10, (rand() % 10)+4);
  	 sleep(1);
  }
}
