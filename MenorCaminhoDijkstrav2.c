/*
	Instituição: INF/UFG
	Cursos: BSI e BES
	Disciplina: Algoritmos e Estruturas de Dados 2 - 2025-1
	Professor: André Luiz Moura
	Finalidade: Implementar o algoritmo do menor caminho (Dijkstra)
	       lendo arquivo .poly do SetorGoiania2.poly
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
    // Inicializa matriz de adjacência com INF
    for (int i = 0; i < totalVertices; i++) {
        for (int j = 0; j < totalVertices; j++) {
            matrizAdj[i][j] = INF;
        }
    }

    // Preenche matriz com as distâncias das arestas
    for (int i = 0; i < totalArestas; i++) {
        int o = arestas[i].origem;
        int d = arestas[i].destino;
        double dist = calcDist(vertices[o].x, vertices[o].y, vertices[d].x, vertices[d].y);
        matrizAdj[o][d] = dist;
        if (arestas[i].direcao == 0) { // aresta não direcionada
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

    // Lê número de vértices e outros parâmetros
    int nVertices, lixo1, lixo2, lixo3;
    if (fscanf(fp, "%d %d %d %d", &nVertices, &lixo1, &lixo2, &lixo3) != 4) {
        printf("Erro ao ler a primeira linha do arquivo\n");
        fclose(fp);
        return 0;
    }
    totalVertices = nVertices;

    // Lê os vértices
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

    // Lê linha extra que tem número de arestas e um valor extra
    int nArestas, extra;
    if (fscanf(fp, "%d %d", &nArestas, &extra) != 2) {
        printf("Erro ao ler número de arestas\n");
        fclose(fp);
        return 0;
    }
    totalArestas = nArestas;

    // Lê as arestas
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

void imprimirArestas() {
    printf("Arestas carregadas:\n");
    for (int i = 0; i < totalArestas; i++) {
        printf("%d: %d -> %d (direcao: %d)\n", i, arestas[i].origem, arestas[i].destino, arestas[i].direcao);
    }
}

void dijkstra(int inicio, int fim) {
    double dist[MAX_VERTICES];
    int prev[MAX_VERTICES];
    int visited[MAX_VERTICES];
    int nosExplorados = 0;

    for (int i = 0; i < totalVertices; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
    }
    dist[inicio] = 0;

    clock_t start = clock();

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
        nosExplorados++;

        for (int v = 0; v < totalVertices; v++) {
            if (matrizAdj[u][v] < INF && dist[u] + matrizAdj[u][v] < dist[v]) {
                dist[v] = dist[u] + matrizAdj[u][v];
                prev[v] = u;
            }
        }
    }

    clock_t end = clock();
    double tempoExec = (double)(end - start) / CLOCKS_PER_SEC;

    if (dist[fim] == INF) {
        printf("\nNão existe caminho entre %d e %d\n", inicio, fim);
        printf("  - Tempo de execução: %.4f segundos\n", tempoExec);
        return;
    }

    printf("\nCusto total do caminho: %.2f unidades\n", dist[fim]);
    printf("Nós explorados: %d\n", nosExplorados);

    // Reconstruir caminho
    int path[MAX_VERTICES], path_len = 0;
    for (int v = fim; v != -1; v = prev[v]) {
        path[path_len++] = v;
    }

    printf("Caminho (do início ao fim):\n");
    for (int i = path_len - 1; i >= 0; i--) {
        printf("%d (x=%.3f, y=%.3f)\n", path[i], vertices[path[i]].x, vertices[path[i]].y);
    }

    printf("  - Tempo de execução: %.4f segundos\n", tempoExec);
}

int main() {
    printf("=== Sistema de Navegação Primitivo (INF/UFG) ===\n");

    if (!lerArquivoPoly("SetorGoiania2.poly")) {
        return 1;
    }
    printf("Arquivo carregado: SetorGoiania2.poly\n");
    printf("Total de vértices: %d\n", totalVertices);
    printf("Total de arestas : %d\n", totalArestas);

    construirGrafo();

    imprimirArestas();

    int origem, destino;
    printf("\nDigite o vértice de origem (0 a %d): ", totalVertices - 1);
    scanf("%d", &origem);
    printf("Digite o vértice de destino (0 a %d): ", totalVertices - 1);
    scanf("%d", &destino);

    if (origem < 0 || origem >= totalVertices || destino < 0 || destino >= totalVertices) {
        printf("Índices inválidos.\n");
        return 1;
    }

    dijkstra(origem, destino);

    return 0;
}

