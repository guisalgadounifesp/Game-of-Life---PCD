// Guilherme Salgado Alves
// Lucas Gabriel Camargo

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#define N 2048
#define N_THREADS 8

typedef struct
{
    int **GridAtual;
    int **NovoGrid;
    int start;
} grid_struct;

void FillGlider(int **grid)
{
    // GLIDER
    int lin = 1, col = 1;

    grid[lin][col + 1] = 1;
    grid[lin + 1][col + 2] = 1;
    grid[lin + 2][col] = 1;
    grid[lin + 2][col + 1] = 1;
    grid[lin + 2][col + 2] = 1;
}

void FillRPentonimo(int **grid)
{
    // R-pentomino
    int lin = 10;
    int col = 30;
    grid[lin][col + 1] = 1;
    grid[lin][col + 2] = 1;
    grid[lin + 1][col] = 1;
    grid[lin + 1][col + 1] = 1;
    grid[lin + 2][col + 1] = 1;
}

void PrintGrid(int **grid)
{
    int i, j;
    char gridCell;
    for (i = 0; i < 50; i++)
    {
        for (j = 0; j < 50; j++)
        {
            if (grid[i][j] == 1){
                gridCell = '*';
            }
            else {
                gridCell = '.';
            }
            printf("%c", gridCell);
        }

        printf("\n");
    }
    printf("\n\n");
}

int GetVizinhosVivos(int **grid, int line, int column)
{
    int Vivo = 0;
    int j = 0;
    int Atual = 0;
    int acima = 0;

    // Pega indice da linha de cima
    if (line == 0){
        acima = N - 1;
    }else{
        acima = line - 1;
    }
    /* Percorre as 3 células vizinhas na linha de cima, 1 imediatamente acima e 2 diagonais */
    for (j = column - 1; j <= column + 1; j++)
    {
        // Checa borda infinita
        Atual = j;
        if (Atual < 0)
            Atual = N - 1;

        if (Atual > N - 1)
            Atual = 0;

        if (grid[acima][Atual] == 1)
            Vivo++;
    }

    // Pega linha de baixo
    int abaixo = (line + 1) % N;

    /* Percorre as 3 células vizinhas na linha de baixo, 1 imediatamente abaixo e 2 diagonais */
    for (j = column - 1; j <= column + 1; j++)
    {
        // Checa borda infinita
        Atual = j;
        if (Atual < 0)
            Atual = N - 1;

        if (Atual > N - 1)
            Atual = 0;

        if (grid[abaixo][Atual] == 1)
            Vivo++;
    }

    // Checa se célula à esquerda está viva
    int esq = 0;
    if (column > 0){
        esq = column -1;
    }else {
        esq = N - 1;
    }
    if (grid[line][esq] == 1)
        Vivo++;

    // Checa se célula à direita está viva
    int dir = 0;
    if (column < N - 1) {
        dir = column +1;
    }else{
        dir = 0;
    }
    if (grid[line][dir] == 1)
        Vivo++;

    return Vivo;
}

int GetNovoEstado(int **grid, int line, int column)
{
    int vizinhos = GetVizinhosVivos(grid, line, column);

    // Verifica se a célula está viva, se grid = 1
    if (grid[line][column] == 1)
    {
        // Se há 2 ou 3 vizinhos vivos, mantém-se viva
        if (vizinhos == 2 || vizinhos == 3)
            return 1;

        // Caso contrário, morre
        return 0;
    }

    /* Caso a célula não esteja viva, verifica se há vizinhos 
    suficientes para viver, ou seja, 3 */
    if (vizinhos == 3)
        return 1;

    return 0;
}

void *Thread_ProccessGeneration(void *arg)
{
    int i, j;
    grid_struct *data = (grid_struct *)arg;

    /* Separa as posições de grid que serão executadas por cada Thread */
    for (i = data->start; i < N; i += N_THREADS)
    {
        for (j = 0; j < N; j++)
        {
            data->NovoGrid[i][j] = GetNovoEstado(data->GridAtual, i, j);
        }
    }

    pthread_exit(NULL);
}


int **GetProximoGrid(int **gridA, int **gridB, int iteracao)
{
    if (iteracao % 2 == 0)
        return gridB;
    else
        return gridA;
}

int **GetGridAtual(int **gridA, int **gridB, int iteracao)
{
    if (iteracao % 2 == 0)
        return gridA;
    else
        return gridB;
}



void PlayJogoVida(int **gridA, int **gridB, int iteracao)
{
    int i, k;
    int num_thread = N_THREADS;
    printf("Numero de threads: %d\n\n", num_thread);
    pthread_t threads[num_thread];
    grid_struct datas[num_thread];
    for (k = 0; k < iteracao; k++)
    {
        int **ProximoGrid = GetProximoGrid(gridA, gridB, k);
        int **GridAtual = GetGridAtual(gridA, gridB, k);
        /*if (k < 5)
            PrintGrid(GridAtual);*/
        for (i = 0; i < num_thread; i++)
        {
            datas[i].GridAtual = GridAtual;
            datas[i].NovoGrid = ProximoGrid;
            datas[i].start = i;
            pthread_create(&threads[i], NULL, Thread_ProccessGeneration, (void *)&datas[i]);
        }

        for (i = 0; i < num_thread; i++)
        {
            pthread_join(threads[i], NULL);
        }
    }
}

int GetSobreviventes(int **grid)
{
    int Vivo = 0;
    int i, j;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            Vivo += grid[i][j];
        }
    }

    return Vivo;
}



void PrintaGeracao(int **grid, int GeracaoAtual)
{
    printf("Geração %d: %d\n", GeracaoAtual, GetSobreviventes(grid));
}



int main()
{
    int **gridA, **gridB;
    gridA = (int **)malloc(N * sizeof(int *));
    gridB = (int **)malloc(N * sizeof(int *));
    struct timeval start, end;
    double time_ms;
    int i = 0, j = 0;
    for (i = 0; i < N; i++)
    {
        gridA[i] = (int *)malloc(N * sizeof(int));
        gridB[i] = (int *)malloc(N * sizeof(int));
        for (j = 0; j < N; j++)
        {
            gridA[i][j] = 0;
            gridB[i][j] = 0;
        }
    }
    FillGlider(gridA);
    FillRPentonimo(gridA);
    printf("Condicao inicial: %d\n", GetSobreviventes(gridA));
    gettimeofday(&start, NULL);
    PlayJogoVida(gridA, gridB, 2000);
    gettimeofday(&end, NULL);
    time_ms = (int)(1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000);
    printf("Ultima geracao de sobreviventes: %d\n", GetSobreviventes(gridB));
    printf("Tempo de execucao: %lf s", time_ms / 1000);
    pthread_exit(NULL);
    return 0;
}