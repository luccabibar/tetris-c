#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#ifdef _WIN32

#include<conio.h>
#include<Windows.h>

#define clear "cls"

#else

#include <termios.h>
#include <fcntl.h>
#include <unistd.h> 

char getch(void)
{
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if(tcgetattr(0, &old) < 0){
        perror("tcsetattr()");
    }
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0){
        perror("tcsetattr ICANON");
    }
    if(read(0, &buf, 1) < 0){
        perror("read()");
    }
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0){
        perror("tcsetattr ~ICANON");
    }
    return buf;
}

//talvez funcione, nao testei
//https://www.vivaolinux.com.br/topico/C-C++/kbhit()-no-linux
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

    if(ch != EOF){
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

void Sleep(int ms){
    usleep(ms * 1000);
}

#define clear "clear"

#endif

#define bool int
#define true 1
#define false 0

//note que o tabuleiro tem so 20 de altura, os 4 extra (definidos por grid padding) sao para manusear as pecas acima da tela
#define GRID_HEIGHT 24 
#define GRID_PADDING 4
#define GRID_WIDTH 10

//define quantos MSs dura um frame, e por extensao, o framerate do jogo
#define FRAME 20

#define BLANK_SPACE ' '
#define CURRENT_BLOCK 'X'
#define PLACED_BLOCK '0'

/**
 * tipos:
 *  
 * 0: I
 * 1: O
 * 2: J
 * 3: L
 * 4: S
 * 5: Z
 * 6: T
 * 
 * direcoes:
 * 
 *     0 
 *  3     1 
 *     2
 */
  
typedef struct tijolo
{
    int tipo;
    int dir;

    int posX;
    int posY;
} tijolo;

//inicia o array que eh a grade do jogo, vazio
void initGrid(char grid[GRID_HEIGHT][GRID_WIDTH])
{
	int i, j;
    for(i = 0; i < GRID_HEIGHT; i++){   
        for(j = 0; j < GRID_WIDTH; j++){
            grid[i][j] = BLANK_SPACE;
        }
    }
}

//imprime a UI do jogo, a telinha
void printGameUi(char grid[GRID_HEIGHT][GRID_WIDTH], int pts, int lvl)
{
    int i, j;
    system(clear);

    printf("\n\n");

    for(i = GRID_PADDING; i < GRID_HEIGHT; i++){
        
        printf("    |");
        for(j = 0; j != GRID_WIDTH; j++){
            printf((j == GRID_WIDTH - 1) ? "%c|\n" : "%c ", grid[i][j]);
        }        
    }
    printf("    |- - - - - - - - - -|\n\n");
    printf("    Pontos: %d\n", pts);
    printf("    Nivel: %d", lvl);
}

//imprime a UI, mas imprime as linhas limpas com um efeito
void printGameUiClearFX(char grid[GRID_HEIGHT][GRID_WIDTH], int fxLines[4], int pts, int lvl)
{
    int i, j, k, cont = 0;
    bool fx = false;
    system(clear);

    printf("\n\n");

    for(i = GRID_PADDING; i != GRID_HEIGHT; i++){
        //se eh uma linha com efeito
        if(fxLines[cont] == i){
            fx = true;
            cont++;
        }

        printf("    |");
        for(j = 0; j != GRID_WIDTH; j++){
            if(fx == true){
                printf((j == GRID_WIDTH - 1) ? "%c|\n" : "%c ", CURRENT_BLOCK);
            }
            else{
                printf((j == GRID_WIDTH - 1) ? "%c|\n" : "%c ", grid[i][j]);
            }
        }

        fx = false;
    }
    printf("    |- - - - - - - - - -|\n\n");
    printf("    Pontos: %d\n", pts);
    printf("    Nivel: %d", lvl);
}

//imprime a UI com um texto de fim de jogo
void printGameUiOver(char grid[GRID_HEIGHT][GRID_WIDTH], int lns, int pts, int lvl)
{
    int i, j, k, cont = 0;
    //distancia do meio pra caber a caixa de dialogo
    int diagComeco = ((GRID_HEIGHT - GRID_PADDING) / 2) - 5 + GRID_PADDING;

    system(clear);

    printf("\n\n");

    for(i = GRID_PADDING; i != GRID_HEIGHT; i++){
        printf("    |");
        for(j = 0; j != GRID_WIDTH; j++){

            if(i == diagComeco || i == diagComeco + 7){
                printf((j == GRID_WIDTH - 1) ? "-|\n" : "- ");
            }
            else if(i == diagComeco + 1 || i == diagComeco + 3){
                printf((j == GRID_WIDTH - 1) ? " |\n" : "  ");
            }
            else if(i == diagComeco + 2){
                printf("   - GAME OVER -   |\n");
                break;
            }
            else if(i == diagComeco + 4){
                printf(" Nivel: %10d |\n", lvl);
                break;
            }
            else if(i == diagComeco + 5){
                printf(" Linhas: %9d |\n", lns);
                break;
            }
            else if(i == diagComeco + 6){
                printf(" Pontos: %9d |\n", pts);
                break;
            }
            else{
                printf((j == GRID_WIDTH - 1) ? "%c|\n" : "%c ", grid[i][j]);
            }
        }
    }
    printf("    |- - - - - - - - - -|\n\n");

}

//posiciona uma peca no jogo
bool placePeca(char grid[GRID_HEIGHT][GRID_WIDTH], tijolo peca)
{
    switch(peca.tipo){
    //I
    case 0:
        switch(peca.dir){
        case 0:
        case 2:
            grid[0 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; //     X
            grid[1 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; //     X
            grid[2 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; //     X
            grid[3 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; //     X
            break;
        
        case 1:
        case 3:
            grid[2 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; 
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; 
            grid[2 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; // X X X X 
            grid[2 + peca.posY][3 + peca.posX] = CURRENT_BLOCK;
            break;
        }
        break;
    
    //O
    case 1:
        grid[0 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; 
        grid[0 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; 
        grid[1 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; // x x
        grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; // x x
        break;
    
    //J
    case 2:
        switch(peca.dir){
        case 0:
            grid[1 + peca.posY][0 + peca.posX] = CURRENT_BLOCK;
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK;
            grid[1 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; // X X X
            grid[2 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; //     X
            break;

        case 1:
            grid[0 + peca.posY][1 + peca.posX] = CURRENT_BLOCK;
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            grid[2 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; // X X
            break;

        case 2:
            grid[1 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; 
            grid[2 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; 
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; // X
            grid[2 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; // X X X
            break;
        
        case 3:
            grid[0 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; 
            grid[0 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; //   X X 
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            break;
        }
        break;
    
    //L
    case 3:
        switch(peca.dir){
        case 0:
            grid[1 + peca.posY][0 + peca.posX] = CURRENT_BLOCK;
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK;
            grid[1 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; // X X X
            grid[2 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; // X
            break;

        case 1:
            grid[0 + peca.posY][0 + peca.posX] = CURRENT_BLOCK;
            grid[0 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; // X X
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            break;

        case 2:
            grid[1 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; 
            grid[2 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; 
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //     X
            grid[2 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; // X X X
            break;
        
        case 3:
            grid[0 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; 
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X 
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            grid[2 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; //   X X
            break;
        }
        break;
    
    //S
    case 4:
        switch(peca.dir){
        case 0:
        case 2:
            grid[2 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; 
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; 
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X X
            grid[1 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; // X X
            break;
        
        case 1:
        case 3:
            grid[0 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; 
            grid[1 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; // X
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; // X X
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            break;
        }
        break;
    
    //Z
    case 5:
        switch(peca.dir){
        case 0:
        case 2:
            grid[1 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; 
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; 
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; // X X
            grid[2 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; //   X X
            break;
        
        case 1:
        case 3:
            grid[0 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; 
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            grid[1 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; // X X
            grid[2 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; // X
            break;
        }
        break;
    
    // T
    case 6:
        switch(peca.dir){
        case 0:
            grid[1 + peca.posY][0 + peca.posX] = CURRENT_BLOCK;
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK;
            grid[1 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; // X X X
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            break;

        case 1:
            grid[0 + peca.posY][1 + peca.posX] = CURRENT_BLOCK;
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; // X X
            grid[1 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; //   X
            break;

        case 2:
            grid[0 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; 
            grid[1 + peca.posY][0 + peca.posX] = CURRENT_BLOCK; 
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X
            grid[1 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; // X X X
            break;
        
        case 3:
            grid[0 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; 
            grid[1 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X 
            grid[2 + peca.posY][1 + peca.posX] = CURRENT_BLOCK; //   X X
            grid[1 + peca.posY][2 + peca.posX] = CURRENT_BLOCK; //   X
            break;
        }
        break;
    }
}

//cria um obj de peca
tijolo createPeca(char grid[GRID_HEIGHT][GRID_WIDTH])
{
    tijolo peca;
    peca.tipo = rand() % 7;
    peca.dir = rand() % 4;

    switch(peca.tipo){
    //I
    case 0:
        peca.posY = 0;
        peca.posX = 3;
        break;
    
    //O
    case 1:
        peca.posY = 2;
        peca.posX = 4;
        break;
    
    //J
    case 2:
        peca.posY = 1;
        peca.posX = 3;
        break;
    
    //L
    case 3:
        peca.posY = 1;
        peca.posX = 3;
        break;
    
    //S
    case 4:
        peca.posY = 1;
        peca.posX = 3;
        break;
    
    //Z
    case 5:
        peca.posY = 1;
        peca.posX = 3;
        break;
    
    // T
    case 6:
        peca.posY = 1;
        peca.posX = 3;
        break;
    }
    
    return peca;
}

//checa se a peca pode descer
bool checkDescePeca(char grid[GRID_HEIGHT][GRID_WIDTH])
{
    int i, j, cont = 0;
    for(i = 0; i != GRID_HEIGHT; i++){   
        for(j = 0; j != GRID_WIDTH; j++){
            //encontrou peca
            if(grid[i][j] == CURRENT_BLOCK){
                //se eh o chao ou se tem peac em baixo
                if(grid[i + 1][j] == PLACED_BLOCK || i + 1 == GRID_HEIGHT){
                    return false;
                }
                else{
                    cont++;
                }
            }
        }

        if(cont == 4){
            return true;
        }
    }
}

//desce a peca
bool descePeca(char grid[GRID_HEIGHT][GRID_WIDTH], tijolo peca)
{
    if(checkDescePeca(grid) == false){
        return false;
    }

    int i, j, cont = 0;
    
    for(i = GRID_HEIGHT - 1; i != -1; i--){   
        for(j = 0; j < GRID_WIDTH; j++){
            //encontrou peca
            if(grid[i][j] == CURRENT_BLOCK){
                grid[i + 1][j] = CURRENT_BLOCK;
                grid[i][j] = BLANK_SPACE;

                    cont++;
            }
        }

        if(cont == 4){
            return true;
        }
    }

    return false;
}

//congela a peca (quando ela encosta no chao)
bool freezePeca(char grid[GRID_HEIGHT][GRID_WIDTH])
{
    if(checkDescePeca(grid) == true){
        return false;
    }

    int i, j, cont = 0;
    
    for(i = 0; i != GRID_HEIGHT; i++){   
        for(j = 0; j != GRID_WIDTH; j++){
            //encontrou peca
            if(grid[i][j] == CURRENT_BLOCK){
                grid[i][j] = PLACED_BLOCK;
                cont++;
            }
        }

        if(cont == 4){
            return true;
        }
    }
}

//checa se a peca pode se mover
bool checkMovePeca(char grid[GRID_HEIGHT][GRID_WIDTH], int dir)
{
    int i, j, cont = 0;

    for(j = 0; j != GRID_WIDTH; j++){
        for(i = 0; i != GRID_HEIGHT; i++){
            //encontrou peca
            if(grid[i][j] == CURRENT_BLOCK){
                //se nao da pra mover
                if(grid[i][j + dir] == PLACED_BLOCK || j + dir == GRID_WIDTH || j + dir == -1){
                    return false;
                }

                cont++;
            }

            if(cont == 4){
                return true;
            }   
        }    
    }

    return false;
}

//move a peca pra direita
bool movePecaD(char grid[GRID_HEIGHT][GRID_WIDTH], tijolo peca)
{
    int i, j, cont = 0;
    bool check = checkMovePeca(grid, +1);

    if(check == false){
        return false;
    }

    for(j = GRID_WIDTH - 1; j != -1; j--){
        for(i = 0; i != GRID_HEIGHT; i++){
            //encontrou peca
            if(grid[i][j] == CURRENT_BLOCK){
                grid[i][j] = BLANK_SPACE;
                grid[i][j + 1] = CURRENT_BLOCK;

                cont++;
            }

            if(cont == 4){
                return true;
            }   
        }    
    }
    return false;
}

//move a peca pra esquerda
bool movePecaE(char grid[GRID_HEIGHT][GRID_WIDTH], tijolo peca)
{
    int i, j, cont = 0;
    bool check = checkMovePeca(grid, -1);

    if(check == false){
        return false;
    }

    for(j = 0; j != GRID_WIDTH; j++){
        for(i = 0; i != GRID_HEIGHT; i++){
            //encontrou peca
            if(grid[i][j] == CURRENT_BLOCK){
                grid[i][j] = BLANK_SPACE;
                grid[i][j - 1] = CURRENT_BLOCK;

                cont++;
            }

            if(cont == 4){
                return true;
            }   
        }    
    }
    return false;
}

//checa se alguma linha foi limpa
int checkLineClear(char grid[GRID_HEIGHT][GRID_WIDTH], int lines[4])
{
    int i, j, cont = 0;
    bool isClear = false;

    for(i = 0; i != GRID_HEIGHT; i++){
        isClear = true;
        for(j = 0; j != GRID_WIDTH; j++){
            //se nao or igual a blocos posicionados
            if(grid[i][j] != PLACED_BLOCK){
                isClear = false;
                break;
            }
        }

        //o bacana eh que esse array eh preenchido em ordem
        //fica de baixo pra cima, no caso
        if(isClear == true){
            lines[cont] = i;
            cont++;
        }
        if(cont == 4){
            return 4;
        }
    }

    return cont;
}

//limpa a linha
int lineClear(char grid[GRID_HEIGHT][GRID_WIDTH], int pts, int lvl)
{
    int lines[4] = { -1, -1, -1, -1 };
    int linesQtd = checkLineClear(grid, lines);
    if(linesQtd == 0){
        return 0;
    }

    int i, j, cont = 0;

    //efeitinho bonito
    printGameUiClearFX(grid, lines, pts, lvl);
    Sleep(100);
    printGameUi(grid, pts, lvl);
    Sleep(100);
    printGameUiClearFX(grid, lines, pts, lvl);
    Sleep(100);

    for(i = GRID_HEIGHT - 1; i != -1; i--){   
        //se for linha de clear
        if(i == lines[linesQtd - 1 - cont]){
            for(j = 0; j < GRID_WIDTH; j++){
                grid[i][j] = BLANK_SPACE;
            }
            cont++;
        }
        //move linha pra baixo caso ja tenha caido
        else if(cont > 0){
            for(j = 0; j < GRID_WIDTH; j++){
                grid[i + cont][j] = grid[i][j];
                grid[i][j] = BLANK_SPACE;
            }
        }
    }

    return linesQtd;
}


//cehca se da pra peca girar
//sim eu odeio essa funcaoo ela eh uma macaroca nojenta
int checkGiraPeca(char grid[GRID_HEIGHT][GRID_WIDTH], tijolo peca, bool tryAgain, int *corrX , int *corrY)
{
    int check = (peca.dir + 1) % 4;
    //esses valores servem pra checar as pecas com algum offset
    //no caso, testando se consegue jogar ela 1 pra cima
    *corrX = 0;
    *corrY = (tryAgain == false) ? -1 : 0;
    int distCanto = 0;

    switch(peca.tipo){
    //I
    case 0:
        switch(check){
        case 0:
        case 2:
            //CRISTO AMADO
            if(grid[0 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK || 
               grid[1 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK || 
               grid[2 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK || 
               grid[3 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK){

                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;            
            break;
        
        case 1:
        case 3:
            //*correcao para posicionamento nos cantos
            if(peca.posX < 0){
                distCanto = - peca.posX;
            }
            else if(peca.posX + 3 >= GRID_WIDTH){
                distCanto = GRID_WIDTH - (peca.posX + 3) - 1;
            }
            
            *corrX += distCanto;

            if(grid[2 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][3 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;
        }
        break;
    
    //O
    case 1:
        return 1;
        break;
    
    //J
    case 2:
        switch(check){
        case 0:
            //*correcao para posicionamento nos cantos
            if(peca.posX < 0){
                distCanto = - peca.posX;
            }
            *corrX += distCanto;

            if(grid[1 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;

        case 1:
            if(grid[0 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;

        case 2:
            //*correcao para posicionamento nos cantos
            if(peca.posX + 2 >= GRID_WIDTH){
                distCanto = GRID_WIDTH - (peca.posX + 2) - 1;
            }
            *corrX += distCanto;

            if(grid[1 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;
        
        case 3:
            if(grid[0 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[0 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;
        }
        break;
    
    //L
    case 3:
        switch(check){
        case 0:
            //*correcao para posicionamento nos cantos
            if(peca.posX < 0){
                distCanto = - peca.posX;
            }
            *corrX += distCanto;

            if(grid[1 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;

        case 1:
            if(grid[0 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[0 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;

        case 2:
            //*correcao para posicionamento nos cantos
            if(peca.posX + 2 >= GRID_WIDTH){
                distCanto = GRID_WIDTH - (peca.posX + 2) - 1;
            }
            *corrX += distCanto;

            if(grid[1 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;
        
        case 3:
            if(grid[0 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;
        }
        break;
    
    //S
    case 4:
        switch(check){
        case 0:
        case 2:
            //*correcao para posicionamento nos cantos
            if(peca.posX + 2 >= GRID_WIDTH){
                distCanto = GRID_WIDTH - (peca.posX + 2) - 1;
            }
            *corrX += distCanto; 

            if(grid[2 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;
        
        case 1:
        case 3:
            if(grid[0 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;
        }
        break;
    
    //Z
    case 5:
        switch(check){
        case 0:
        case 2:
            //*correcao para posicionamento nos cantos
            if(peca.posX + 2 >= GRID_WIDTH){
                distCanto = GRID_WIDTH - (peca.posX + 2) - 1;
            }
            *corrX += distCanto; 

            if(grid[1 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;
        
        case 1:
        case 3:
            if(grid[0 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;
        }
        break;
    
    //T
    case 6:
        switch(check){
        case 0:
            //*correcao para posicionamento nos cantos
            if(peca.posX < 0){
                distCanto = - peca.posX;
            }
            *corrX += distCanto;
            
            if(grid[1 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;

        case 1:
            if(grid[0 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;

        case 2:
            //*correcao para posicionamento nos cantos
            if(peca.posX + 2 >= GRID_WIDTH){
                distCanto = GRID_WIDTH - (peca.posX + 2) - 1;
            }
            *corrX += distCanto;

            if(grid[0 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][0 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;
        
        case 3:
            if(grid[0 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[2 + peca.posY + *corrY][1 + peca.posX + *corrX] == PLACED_BLOCK ||
               grid[1 + peca.posY + *corrY][2 + peca.posX + *corrX] == PLACED_BLOCK){
              
                return (tryAgain == true) ? checkGiraPeca(grid, peca, false, corrX, corrY) : false;
            }
            return (tryAgain == true) ? 1 : 2;
            break;
        }
        break;
    }
}

//gira a peca de fato
tijolo giraPeca(char grid[GRID_HEIGHT][GRID_WIDTH], tijolo peca)
{
    //O
    if(peca.tipo == 1){
        peca.dir = (peca.dir != 3) ? peca.dir + 1 : 0;
        return peca;
    }

    int corrX, corrY;
    int res = checkGiraPeca(grid, peca, true, &corrX, &corrY);

    if(res == 0){
        return peca;
    }
    int i, j, cont = 0;

    //limnpa a peca
    for(i = 0; i != GRID_HEIGHT; i++){   
        for(j = 0; j != GRID_WIDTH; j++){
            //encontrou peca
            if(grid[i][j] == CURRENT_BLOCK){
                grid[i][j] = BLANK_SPACE;
                cont++;
            }
        }

        if(cont == 4){
            break;
        }
    }

    //posiciona ela
    peca.posX += corrX;
    if(res == 2){
        peca.posY += -1;
    }
    peca.dir = (peca.dir + 1) % 4;
    placePeca(grid, peca);

    return peca;
}

//checa o nivel atual, com base no numero de linhas ja limpas
int checkCurrLevel(int x)
{
    if(x <= 2) return 0;
    if(x <= 4) return 1;
    if(x <= 8) return 2;
    if(x <= 10) return 3;
    if(x <= 48) return 4;
    //essa ultima dificuldade eh quse como um easter egg
    return 5;

}

//checa se o jogo acabou
bool checkGameOver(char grid[GRID_HEIGHT][GRID_WIDTH])
{
    int i, j;

    for(i = 0; i != GRID_PADDING; i++){
        for(j = 0; j != GRID_WIDTH; j++){
            //se encontrou um bloco la no topo da tela
            if(grid[i][j] == PLACED_BLOCK){
                return true;
            }
        }
    }

    //nao acabou o jogo
    return false;
}

//jogo
void game()
{
    char grid[GRID_HEIGHT][GRID_WIDTH];
    char inp;
    int i, lines;
    bool isActive = false, isStopping = false, hasChanged = true, gameOver = false;
    bool res;
    tijolo peca;

    int lineClearCount = 0;
    int pontos = 0;
    /**PONTOS
     * descer 1 peca: +1
     * limpar 1 linha: +100 
     * limpar 2 linhas: +300 
     * limpar 3 linhas: +500 
     * limpar 4 linhas: +700
     * 
     * todos os pontos sao multiplicados pelo nivel
     */

    int frameCount = 0;
    int  currLevel = 0;
    int levels[6] = { 24, 16, 16, 8, 4, 1 };

    initGrid(grid);

    //cada iteracao no loop eh um "frame"
    //a ideia eh que a cada N frames ele desca a peca um coiso automaticamente

    while(1){
        if(isActive == false){
            peca = createPeca(grid);
            placePeca(grid, peca);
            isActive = true;
            isStopping = false;
        }
        
        if(hasChanged == true){
        	printGameUi(grid, pontos, currLevel + 1);
            hasChanged = false;        	
		}

        if(kbhit()){
            inp = getch();
            hasChanged = true;
		}

        switch(inp){
        //roda peca
        case 'w':
        case 'W':
            peca = giraPeca(grid, peca);
            break;
        
        //move pra esquerda
        case 'a':
        case 'A':
            peca.posX += (movePecaE(grid, peca)) ? -1 : 0;
            break;
            
        //move pra direita
        case 'd':
        case 'D':
            peca.posX += (movePecaD(grid, peca)) ? 1 : 0;
            break;

        //desce peca
        case 's':
        case 'S':
            res = descePeca(grid, peca);
            if(res == true){
                isStopping = false;
                peca.posY += 1;    

                pontos += currLevel + 1;
            }
            else{
                isStopping = true;
            }
            break;
        }

        inp = ' ';

        //dorme pelo periodo de um frame
        //isso serve mais pro jogo rodar em uma taxa constante
        Sleep(FRAME);

        frameCount++;
        if(frameCount >= levels[currLevel]){
            res = descePeca(grid, peca);
            hasChanged = true;
            if(res == true){
                isStopping = false;
                peca.posY += 1;    
            }
            else{
                isStopping = true;
            }

            frameCount = 0;
        }

        //se a peca esta parando
        if(isStopping == true){
            //se a peca nao parou
            if(freezePeca(grid) == false){
                isStopping = false;
            }

            hasChanged = true;
            isActive = false;
            lines = lineClear(grid, pontos, currLevel + 1);
            lineClearCount += lines;
            currLevel = checkCurrLevel(lineClearCount);

            pontos += (lines > 0) ? (((lines - 1) * 200) + 100) * (currLevel + 1) : 0;

            if(checkGameOver(grid) == true){
                break;
            }
        }
    }

    printGameUiOver(grid, lineClearCount, pontos, currLevel + 1);
    getch();
}

//explicacao do jogo
void comoGame()
{
    system(clear);
    printf("\n\n");
    printf("     _____  ___  _____  ___  ___  ___\n");
    printf("    |_   _|| __||_   _|| _ \\|_ _|/ __|\n");
    printf("      | |  | _|   | |  |   / | | \\__ \\\n");
    printf("      |_|  |___|  |_|  |_|_\\|___||___/\n");

    printf("\n\n\n");
    printf("    Tetris, o classico jogo que vem do desejo de criar ordem a\n");
    printf("    partir do caos, agora no terminal de texto de seu computador!\n\n");
    printf("    Pecas cairao do topo da tela, e voce deve organiza-las para\n");
    printf("    que ocupem o menor espaco possivel\n\n");
    printf("    - Mova as pecas horizontalmente com A e D\n");
    printf("    - Gire-as com W\n");
    printf("    - Adiante a queda delas com S\n\n");
    printf("    Quando todos os espacos de uma linha forem preenchidos, esta\n");
    printf("    linha sera limpa, efetivamente ganhando uma linha a mais\n\n");
    printf("    O jogo termina quando nao couberem mais pecas na tela\n\n");
    printf("    Aperte qualquer tecla para continuar...\n");

    getch();

}

int main()
{
    srand(time(NULL));
    int i = 0;
    char inp;
    bool exit = false;
    
    while(1){
        system(clear);
        printf("\n\n");
        printf("     _____  ___  _____  ___  ___  ___\n");
        printf("    |_   _|| __||_   _|| _ \\|_ _|/ __|\n");
        printf("      | |  | _|   | |  |   / | | \\__ \\\n");
        printf("      |_|  |___|  |_|  |_|_\\|___||___/\n");

        printf("\n\n\n");
        printf("  %c Jogar\n", (i == 0) ? '>' : ' ');
        printf("  %c Como jogar\n", (i == 1) ? '>' : ' ');
        printf("  %c Sair\n", (i == 2) ? '>' : ' ');
        printf("\n    aperte W ou S para mover o cursor, ESPACO ou ENTER para selecionar\n");
        
        inp = getch();
        switch(inp){
        case 'W':
        case 'w':
            i = (i - 1 < 0) ? 2 : i - 1;
            break;
        
        case 'S':
        case 's':
            i = (i + 1 > 2) ? 0 : i + 1;
            break;
        
        case 10:
        case 13:
        case ' ':

            switch(i){
            case 0:
                game();
                break;
            
            case 1:
                comoGame();
                break;

            case 2:
                exit == true;
                break;
            }
            break;
        }

        if(exit == true){
            break;
        }
    }

    return 0;
}
