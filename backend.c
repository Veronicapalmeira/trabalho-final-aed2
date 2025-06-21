/*
	Instituição: INF/UFG
	Cursos: BSI e BES
	Disciplina: Algoritmos e Estruturas de Dados 2 - 2025-1
	Professor: André Luiz Moura
	Finalidade: Implementar o algoritmo do menor caminho (Dijkstra)
	       lendo arquivo .poly do SetorGoiania2.poly
	       Recebe o nome do .poly como argumento.
	       Lê origem e destino de entrada.txt.
	       Grava saída em saida.txt.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define MAX_VERTICES 1000
#define INF 1e9

typedef struct {
    int id;
    double x;
    double y;
} Vertice;

typedef struct {
    int origem;
    int destino;
    int direcao; // 0 para não direcionado, 1 para direcionado
} Aresta;

Vertice vertices[MAX_VERTICES];
Aresta arestas[MAX_VERTICES];
double matrizAdj[MAX_VERTICES][MAX_VERTICES];

int totalVertices = 0;
int totalArestas = 0;

double calcDist(double x0, double y0, double x1, double y1) {
    return sqrt(pow(x0 - x1, 2.0) + pow(y0 - y1, 2.0));
}

void construirGrafo() {
    for (int i = 0; i < totalVertices; i++) {
        for (int j = 0; j < totalVertices; j++) {
            matrizAdj[i][j] = INF;
        }
    }
    for (int i = 0; i < totalArestas; i++) {
        int o = arestas[i].origem;
        int d = arestas[i].destino;
        double dist = calcDist(vertices[o].x, vertices[o].y, vertices[d].x, vertices[d].y);
        matrizAdj[o][d] = dist;
        if (arestas[i].direcao == 0) {
            matrizAdj[d][o] = dist;
        }
    }
}

int lerArquivoPoly(char *nomeArquivo) {
    FILE *fp = fopen(nomeArquivo, "r");
    if (!fp) {
        printf("Erro ao abrir arquivo %s\n", nomeArquivo);
        return 0;
    }

    int nVertices, lixo1, lixo2, lixo3;
    if (fscanf(fp, "%d %d %d %d", &nVertices, &lixo1, &lixo2, &lixo3) != 4) {
        printf("Erro ao ler a primeira linha do arquivo\n");
        fclose(fp);
        return 0;
    }
    totalVertices = nVertices;

    for (int i = 0; i < totalVertices; i++) {
        int id;
        double x, y;
        if (fscanf(fp, "%d %lf %lf", &id, &x, &y) != 3) {
            printf("Erro ao ler vértice %d\n", i);
            fclose(fp);
            return 0;
        }
        vertices[i].id = id;
        vertices[i].x = x;
        vertices[i].y = y;
    }

    int nArestas, extra;
    if (fscanf(fp, "%d %d", &nArestas, &extra) != 2) {
        printf("Erro ao ler número de arestas\n");
        fclose(fp);
        return 0;
    }
    totalArestas = nArestas;

    for (int i = 0; i < totalArestas; i++) {
        int id, o, d, dir;
        if (fscanf(fp, "%d %d %d %d", &id, &o, &d, &dir) != 4) {
            printf("Erro ao ler aresta %d\n", i);
            fclose(fp);
            return 0;
        }
        arestas[i].origem = o;
        arestas[i].destino = d;
        arestas[i].direcao = dir;
    }

    fclose(fp);
    return 1;
}

void dijkstra(int inicio, int fim, int *nosExplorados, double *distFinal, int *prev) {
    double dist[MAX_VERTICES];
    int visited[MAX_VERTICES];
    *nosExplorados = 0;

    for (int i = 0; i < totalVertices; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
    }
    dist[inicio] = 0;

    for (int i = 0; i < totalVertices; i++) {
        int u = -1;
        double min = INF;
        for (int j = 0; j < totalVertices; j++) {
            if (!visited[j] && dist[j] < min) {
                min = dist[j];
                u = j;
            }
        }
        if (u == -1)
            break;

        visited[u] = 1;
        (*nosExplorados)++;

        for (int v = 0; v < totalVertices; v++) {
            if (matrizAdj[u][v] < INF && dist[u] + matrizAdj[u][v] < dist[v]) {
                dist[v] = dist[u] + matrizAdj[u][v];
                prev[v] = u;
            }
        }
    }

    *distFinal = dist[fim];
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Erro: informe o nome do arquivo .poly como argumento\n");
        return 1;
    }

    char nomeArquivo[256];
    strcpy(nomeArquivo, argv[1]);

    if (!lerArquivoPoly(nomeArquivo)) {
        return 1;
    }

    construirGrafo();

    FILE *entrada = fopen("entrada.txt", "r");
    if (!entrada) {
        printf("Erro ao abrir entrada.txt\n");
        return 1;
    }
    int origem, destino;
    if (fscanf(entrada, "%d", &origem) != 1 || fscanf(entrada, "%d", &destino) != 1) {
        printf("Erro ao ler origem e destino em entrada.txt\n");
        fclose(entrada);
        return 1;
    }
    fclose(entrada);

    if (origem < 0 || origem >= totalVertices || destino < 0 || destino >= totalVertices) {
        printf("Origem ou destino inválidos.\n");
        return 1;
    }

    clock_t start = clock();

    int nosExplorados = 0;
    double distFinal = INF;
    int prev[MAX_VERTICES];

    dijkstra(origem, destino, &nosExplorados, &distFinal, prev);

    clock_t end = clock();
    double tempoExec = (double)(end - start) / CLOCKS_PER_SEC;

    FILE *saida = fopen("saida.txt", "w");
    if (!saida) {
        printf("Erro ao criar saida.txt\n");
        return 1;
    }

    if (distFinal == INF) {
        fprintf(saida, "Não existe caminho entre %d e %d\n", origem, destino);
    } else {
        fprintf(saida, "Custo total do caminho: %.2lf unidades\n", distFinal);
        fprintf(saida, "Nós explorados: %d\n", nosExplorados);
        fprintf(saida, "Caminho (do início ao fim):\n");

        int path[MAX_VERTICES], path_len = 0;
        for (int v = destino; v != -1; v = prev[v]) {
            path[path_len++] = v;
        }
        for (int i = path_len - 1; i >= 0; i--) {
            fprintf(saida, "%d (x=%.3lf, y=%.3lf)\n", path[i], vertices[path[i]].x, vertices[path[i]].y);
        }
    }

    fprintf(saida, "  - Tempo de execução: %.4lf segundos\n", tempoExec);
    fclose(saida);

    return 0;
}

