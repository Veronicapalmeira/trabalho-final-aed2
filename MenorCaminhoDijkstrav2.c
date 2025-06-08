#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_VERTICES 10000
#define INF 1e9

typedef struct {
    int id;
    double x;
    double y;
} Vertice;

double matrizAdj[MAX_VERTICES][MAX_VERTICES];
Vertice vertices[MAX_VERTICES];
int totalVertices = 0;
int totalArestas = 0;

double calcDist(double x0, double y0, double x1, double y1) {
    return sqrt(pow(x0 - x1, 2.0) + pow(y0 - y1, 2.0));
}

void construirGrafo() {
    for (int i = 0; i < totalVertices; i++)
        for (int j = 0; j < totalVertices; j++)
            matrizAdj[i][j] = INF;
}

void carregarGrafoDeArquivo(const char* nomeArquivo) {
    FILE* f = fopen(nomeArquivo, "r");
    if (!f) {
        perror("Erro ao abrir o arquivo");
        exit(1);
    }

    int tipo1, tipo2;
    fscanf(f, "%d %d %d %d\n", &totalVertices, &tipo1, &tipo2, &tipo2);

    for (int i = 0; i < totalVertices; i++) {
        int id;
        double x, y;
        fscanf(f, "%d %lf %lf\n", &id, &x, &y);
        vertices[id].id = id;
        vertices[id].x = x;
        vertices[id].y = y;
    }

    construirGrafo();

    int arestas, tipo;
    fscanf(f, "%d %d\n", &arestas, &tipo);
    totalArestas = arestas;

    for (int i = 0; i < arestas; i++) {
        int id, a, b, sentido;
        fscanf(f, "%d %d %d %d\n", &id, &a, &b, &sentido);
        double dist = calcDist(vertices[a].x, vertices[a].y, vertices[b].x, vertices[b].y);
        matrizAdj[a][b] = dist;
        matrizAdj[b][a] = dist; // Se for grafo não-direcionado
    }

    fclose(f);
}

void dijkstra(int inicio, int fim) {
    double dist[MAX_VERTICES];
    int prev[MAX_VERTICES], visited[MAX_VERTICES];

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
                u = j;
                min = dist[j];
            }
        }

        if (u == -1) break;

        visited[u] = 1;

        for (int v = 0; v < totalVertices; v++) {
            if (matrizAdj[u][v] < INF && dist[u] + matrizAdj[u][v] < dist[v]) {
                dist[v] = dist[u] + matrizAdj[u][v];
                prev[v] = u;
            }
        }
    }

    if (dist[fim] == INF) {
        printf("Sem caminho entre %d e %d\n", inicio, fim);
        return;
    }

    printf("\nDistância total: %.2f u.m.\n", dist[fim]);

    // Reconstrói caminho
    int path[MAX_VERTICES], path_len = 0;
    for (int v = fim; v != -1; v = prev[v])
        path[path_len++] = v;

    printf("\nCaminho (do início ao fim):\n");
    for (int i = path_len - 1; i >= 0; i--)
        printf("%d (x=%f, y=%f)\n", path[i], vertices[path[i]].x, vertices[path[i]].y);
}

int main() {
    char nomeArquivo[] = "goianesia.poly";
    carregarGrafoDeArquivo(nomeArquivo);

    printf("Total de vértices: %d\n", totalVertices);
    printf("Total de arestas : %d\n", totalArestas);

    int origem, destino;
    printf("\nDigite o vértice de origem (0 a %d): ", totalVertices - 1);
    scanf("%d", &origem);
    printf("Digite o vértice de destino (0 a %d): ", totalVertices - 1);
    scanf("%d", &destino);

    if (origem >= 0 && origem < totalVertices && destino >= 0 && destino < totalVertices)
        dijkstra(origem, destino);
    else
        printf("Índices inválidos.\n");

    system("pause > nul");
    return 0;
}

