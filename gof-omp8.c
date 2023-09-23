// Guilherme Salgado Alves
// Lucas Gabriel Camargo

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 2048
#define N_THREADS 8

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
    char gridcell;
    for (i = 0; i < 50; i++)
    {
        for (j = 0; j < 50; j++)
        {
            if(grid[i][j]==1){
                gridcell = '*';
            }else{
                gridcell = '.';
            }
            printf("%c", gridcell);
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
    int Acima = 0;

    // Pega linha de cima
    if (line == 0) {
        Acima = N - 1;
    }else{
        Acima = line - 1;
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
        if (grid[Acima][Atual] == 1)
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
     int Esq = 0;
     if (column > 0) {
        Esq = column -1;
     }else{
        Esq = N - 1;
     }
    if (grid[line][Esq] == 1)
        Vivo++;

    // Checa se célula à direita está viva
    int Dir = 0;
    if ( column < N - 1){
        Dir = column + 1;
    }else{
        Dir = 0;
    }
    if (grid[line][Dir] == 1)
        Vivo++;

    return Vivo;
}

int GetNovoEstado(int **grid, int line, int column)
{
    int Vizinhos = GetVizinhosVivos(grid, line, column);

    // Verifica se a célula está viva, se grid = 1
    if (grid[line][column] == 1)
    {
        // Se há 2 ou 3 vizinhos vivos, mantém-se viva
        if (Vizinhos == 2 || Vizinhos == 3)
            return 1;

        // Caso contrário, morre
        return 0;
    }

    /* Caso a célula não esteja viva, verifica se há vizinhos 
    suficientes para viver, ou seja, 3 */
    if (Vizinhos == 3)
        return 1;

    return 0;
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
    int i, j, k;
    int thread_id;
    int print_thread_num = 0;
    for (k = 0; k < iteracao; k++)
    {
        int **ProximoGrid = GetProximoGrid(gridA, gridB, k);
        int **GridAtual = GetGridAtual(gridA, gridB, k);

        /*if (k < 5)
            PrintGrid(GridAtual);*/

#pragma omp parallel default(none) shared(ProximoGrid, GridAtual, print_thread_num) private(i, j, thread_id)
        {
            thread_id = omp_get_thread_num();
            if (thread_id == 0 && print_thread_num == 0)
            {
                printf("Numero de threads: %d\n\n", omp_get_num_threads());
                print_thread_num = 1;
            }

#pragma omp for
            for (i = 0; i < N; i++)
            {
                for (j = 0; j < N; j++)
                {
                    ProximoGrid[i][j] = GetNovoEstado(GridAtual, i, j);
                }
            }
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

int main()
{
    int **gridA, **gridB;
    gridA = (int **)malloc(N * sizeof(int *));
    gridB = (int **)malloc(N * sizeof(int *));
    double start, end;
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
    omp_set_num_threads(N_THREADS);
    printf("Condicao inicial: %d\n", GetSobreviventes(gridA));
    start = omp_get_wtime();
    PlayJogoVida(gridA, gridB, 2001);
    end = omp_get_wtime();
    printf("Ultima geracao (2000 iteracoes): %d\n", GetSobreviventes(gridB));
    printf("Tempo execucao: %f ms\n", end - start);
    return 0;
}
