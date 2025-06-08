/*
    Projeto Final - AED2 - INF/UFG 2025-1
    Tema: Sistema de Navegação Primitivo
    Back-end completo (sem interface gráfica)
    Autora: Verônica (e grupo)
    Professor: André Luiz Moura
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define MAX_VERTICES 10000
#define INF 1e9

// Estrutura de vértice com coordenadas
typedef struct {
    int id;
    double x, y;
} Vertice;

// Estrutura para lista de adjacência
typedef struct vizinho {
    int id;
    double peso;
    struct vizinho* prox;
} Vizinho;

Vertice vertices[MAX_VERTICES];
Vizinho* listaAdj[MAX_VERTICES];
int totalVertices = 0;
int totalArestas = 0;

// Função para calcular distância Euclidiana entre dois pontos
double calcDist(double x0, double y0, double x1, double y1) {
    return sqrt(pow(x0 - x1, 2.0) + pow(y0 - y1, 2.0));
}

// Adiciona aresta na lista de adjacência
void adicionarAresta(int origem, int destino, double peso) {
    Vizinho* novo = (Vizinho*) malloc(sizeof(Vizinho));
    novo->id = destino;
    novo->peso = peso;
    novo->prox = listaAdj[origem];
    listaAdj[origem] = novo;
}

// Lê grafo a partir de arquivo .poly
void carregarGrafo(const char* nomeArquivo) {
    FILE* f = fopen(nomeArquivo, "r");
    if (!f) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    int dummy;
    fscanf(f, "%d %d %d %d", &totalVertices, &dummy, &dummy, &dummy);

    for (int i = 0; i < totalVertices; i++) {
        int id;
        double x, y;
        fscanf(f, "%d %lf %lf", &id, &x, &y);
        vertices[id].id = id;
        vertices[id].x = x;
        vertices[id].y = y;
        listaAdj[id] = NULL;
    }

    fscanf(f, "%d %d", &totalArestas, &dummy);
    for (int i = 0; i < totalArestas; i++) {
        int id, a, b, sentido;
        fscanf(f, "%d %d %d %d", &id, &a, &b, &sentido);
        double peso = calcDist(vertices[a].x, vertices[a].y, vertices[b].x, vertices[b].y);
        adicionarAresta(a, b, peso);
        if (sentido == 0) // mão dupla
            adicionarAresta(b, a, peso);
    }

    int fimMarcador;
    fscanf(f, "%d", &fimMarcador); // leitura do zero final
    fclose(f);
}

// Dijkstra com lista de adjacência (sem heap ainda)
void dijkstra(int origem, int destino) {
    double dist[MAX_VERTICES];
    int prev[MAX_VERTICES];
    int visitado[MAX_VERTICES];

    for (int i = 0; i < totalVertices; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visitado[i] = 0;
    }

    dist[origem] = 0;

    int explorados = 0;
    clock_t inicio = clock();

    for (int i = 0; i < totalVertices; i++) {
        int u = -1;
        double menor = INF;
        for (int j = 0; j < totalVertices; j++) {
            if (!visitado[j] && dist[j] < menor) {
                menor = dist[j];
                u = j;
            }
        }
        if (u == -1) break;

        visitado[u] = 1;
        explorados++;

        for (Vizinho* v = listaAdj[u]; v != NULL; v = v->prox) {
            if (dist[u] + v->peso < dist[v->id]) {
                dist[v->id] = dist[u] + v->peso;
                prev[v->id] = u;
            }
        }
    }

    clock_t fim = clock();
    double tempoExec = (double)(fim - inicio) / CLOCKS_PER_SEC;

    if (dist[destino] == INF) {
        printf("\nNao ha caminho entre os vertices %d e %d.\n", origem, destino);
        return;
    }

    // Exibir estatísticas (RF07)
    printf("\nCusto total: %.2f unidades\n", dist[destino]);
    printf("Nos explorados: %d\n", explorados);
    printf("Tempo de execucao: %.4f segundos\n", tempoExec);

    // Reconstruir caminho
    int caminho[MAX_VERTICES], tam = 0;
    for (int v = destino; v != -1; v = prev[v])
        caminho[tam++] = v;

    printf("\nCaminho (origem -> destino):\n");
    for (int i = tam - 1; i >= 0; i--)
        printf("%d (x=%.2f, y=%.2f)\n", caminho[i], vertices[caminho[i]].x, vertices[caminho[i]].y);
}

int main() {
    char nomeArquivo[] = "goianesia.poly"; // alterar se necessario
    carregarGrafo(nomeArquivo);

    printf("\nTotal de vertices: %d\n", totalVertices);
    printf("Total de arestas : %d\n", totalArestas);

    int origem, destino;
    printf("\nDigite o vertice de origem (0 a %d): ", totalVertices - 1);
    scanf("%d", &origem);
    printf("Digite o vertice de destino (0 a %d): ", totalVertices - 1);
    scanf("%d", &destino);

    if (origem >= 0 && origem < totalVertices && destino >= 0 && destino < totalVertices)
        dijkstra(origem, destino);
    else
        printf("Indices invalidos.\n");

    system("pause");
    return 0;
}

