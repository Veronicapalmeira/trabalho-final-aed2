/*
    Projeto Final - AED2 - INF/UFG 2025-1
    Tema: Sistema de Navegação Primitivo
    Back-end com Fila de Prioridade (Heap Mínima) e Lista de Adjacência
    Autores: Ana Luisa, Isadora, Lucas e Verônica
    Professor: André Luiz Moura
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define MAX_VERTICES 1000   // Limite máximo de vértices do grafo
#define INF 1e9             // Representação de "infinito" para distâncias

// ------------------------ ESTRUTURAS ------------------------ //
// Estrutura para uma aresta na lista de adjacência
typedef struct Aresta {
    int destino;           // Índice do vértice destino da aresta
    double peso;           // Peso (distância) da aresta
    struct Aresta* prox;   // Próxima aresta na lista encadeada
} Aresta;

// Estrutura para um vértice do grafo
typedef struct {
    int id;                // Identificador do vértice
    double x, y;           // Coordenadas do vértice no plano
    Aresta* lista;         // Lista encadeada de arestas adjacentes
} Vertice;

// Estrutura para armazenar um nó na heap mínima
typedef struct {
    int id;                // Índice do vértice
    double dist;           // Distância acumulada para Dijkstra
} NodoHeap;

// ------------------------ VARIÁVEIS GLOBAIS ------------------------ //
Vertice grafo[MAX_VERTICES];  // Vetor com todos os vértices do grafo
int totalVertices = 0;        // Quantidade atual de vértices
int totalArestas = 0;         // Quantidade atual de arestas

// ------------------------ FUNÇÕES AUXILIARES ------------------------ //
// Calcula distância Euclidiana entre dois vértices do grafo pelo índice
double distancia(int a, int b) {
    double dx = grafo[a].x - grafo[b].x;
    double dy = grafo[a].y - grafo[b].y;
    return sqrt(dx*dx + dy*dy);
}

// Adiciona uma aresta ao grafo (lista de adjacência)
// Se direcao == 0, a aresta é bidirecional (insere a inversa também)
void adicionarAresta(int origem, int destino, int direcao) {
    Aresta* nova = malloc(sizeof(Aresta));
    nova->destino = destino;
    nova->peso = distancia(origem, destino);
    nova->prox = grafo[origem].lista;
    grafo[origem].lista = nova;

    if (direcao == 0) {
        // Insere a aresta inversa para grafo não direcionado
        Aresta* inversa = malloc(sizeof(Aresta));
        inversa->destino = origem;
        inversa->peso = nova->peso;
        inversa->prox = grafo[destino].lista;
        grafo[destino].lista = inversa;
    }
}

// ------------------------ HEAP MÍNIMA ------------------------ //
// Estrutura e funções para gerenciar uma heap mínima para Dijkstra
NodoHeap heap[MAX_VERTICES];  // Array que representa a heap
int posHeap[MAX_VERTICES];    // Mapeia o índice do vértice para sua posição na heap
int tamHeap = 0;              // Tamanho atual da heap

// Troca dois elementos da heap e atualiza as posições no posHeap
void trocar(int i, int j) {
    NodoHeap tmp = heap[i];
    heap[i] = heap[j];
    heap[j] = tmp;
    posHeap[heap[i].id] = i;
    posHeap[heap[j].id] = j;
}

// Sobe um elemento na heap para manter a propriedade de heap mínima
void subir(int i) {
    while (i > 0 && heap[i].dist < heap[(i - 1) / 2].dist) {
        trocar(i, (i - 1) / 2);
        i = (i - 1) / 2;
    }
}

// Desce um elemento na heap para manter a propriedade de heap mínima
void descer(int i) {
    int menor = i;
    int esq = 2 * i + 1, dir = 2 * i + 2;
    if (esq < tamHeap && heap[esq].dist < heap[menor].dist) menor = esq;
    if (dir < tamHeap && heap[dir].dist < heap[menor].dist) menor = dir;
    if (menor != i) {
        trocar(i, menor);
        descer(menor);
    }
}

// Insere um novo vértice na heap com sua distância inicial
void inserirHeap(int id, double dist) {
    heap[tamHeap].id = id;
    heap[tamHeap].dist = dist;
    posHeap[id] = tamHeap;
    subir(tamHeap);
    tamHeap++;
}

// Remove e retorna o vértice com menor distância da heap
int extrairMin() {
    int min = heap[0].id;
    heap[0] = heap[--tamHeap];
    posHeap[heap[0].id] = 0;
    descer(0);
    return min;
}

// Atualiza a distância de um vértice já presente na heap e ajusta a posição
void diminuirChave(int id, double novaDist) {
    int i = posHeap[id];
    heap[i].dist = novaDist;
    subir(i);
}

// ------------------------ FUNÇÃO PARA LEITURA DO ARQUIVO .POLY ------------------------ //
// Lê o arquivo de entrada com vértices e arestas no formato .poly
// Preenche a estrutura do grafo com vértices e arestas
int lerArquivoPoly(char* nome) {
    FILE* f = fopen(nome, "r");
    if (!f) return 0;

    int lixo1, lixo2, lixo3;
    fscanf(f, "%d %d %d %d", &totalVertices, &lixo1, &lixo2, &lixo3);
    for (int i = 0; i < totalVertices; i++) {
        int id; double x, y;
        fscanf(f, "%d %lf %lf", &id, &x, &y);
        grafo[i].id = id;
        grafo[i].x = x;
        grafo[i].y = y;
        grafo[i].lista = NULL;
    }
    fscanf(f, "%d %d", &totalArestas, &lixo1);
    for (int i = 0; i < totalArestas; i++) {
        int id, o, d, dir;
        fscanf(f, "%d %d %d %d", &id, &o, &d, &dir);
        adicionarAresta(o, d, dir);
    }
    fclose(f);
    return 1;
}

// ------------------------ ALGORITMO DE DIJKSTRA COM HEAP ------------------------ //
// Executa o algoritmo de Dijkstra para encontrar o menor caminho
// origem: índice do vértice origem
// destino: índice do vértice destino
// nosExplorados: ponteiro para armazenar quantos nós foram explorados
// custoFinal: ponteiro para armazenar custo total do caminho
// anterior: vetor para armazenar o predecessor de cada nó no caminho
void dijkstra(int origem, int destino, int* nosExplorados, double* custoFinal, int* anterior) {
    double dist[MAX_VERTICES];
    int visitado[MAX_VERTICES] = {0};
    *nosExplorados = 0;

    // Inicialização: distâncias infinitas, predecessores inválidos
    for (int i = 0; i < totalVertices; i++) {
        dist[i] = INF;
        anterior[i] = -1;
    }
    dist[origem] = 0;

    // Inicializa a heap com o nó de origem
    tamHeap = 0;
    inserirHeap(origem, 0);

    while (tamHeap > 0) {
        int u = extrairMin();
        if (visitado[u]) continue; // Ignora vértices já visitados
        visitado[u] = 1;
        (*nosExplorados)++;

        // Relaxamento das arestas adjacentes a u
        for (Aresta* a = grafo[u].lista; a != NULL; a = a->prox) {
            int v = a->destino;
            if (!visitado[v] && dist[u] + a->peso < dist[v]) {
                dist[v] = dist[u] + a->peso;
                anterior[v] = u;
                inserirHeap(v, dist[v]);
            }
        }
    }

    *custoFinal = dist[destino];
}

// ------------------------ FUNÇÃO PRINCIPAL ------------------------ //
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Informe o nome do arquivo .poly como argumento\n");
        return 1;
    }
    if (!lerArquivoPoly(argv[1])) {
        printf("Erro ao ler o arquivo.\n");
        return 1;
    }

    // Lê origem e destino do arquivo entrada.txt
    FILE* entrada = fopen("entrada.txt", "r");
    if (!entrada) return 1;
    int origem, destino;
    fscanf(entrada, "%d", &origem);
    fscanf(entrada, "%d", &destino);
    fclose(entrada);

    // Executa Dijkstra e mede tempo de execução
    clock_t ini = clock();
    int nos = 0, anterior[MAX_VERTICES];
    double custo;
    dijkstra(origem, destino, &nos, &custo, anterior);
    clock_t fim = clock();
    double tempo = (double)(fim - ini) / CLOCKS_PER_SEC;

    // Escreve resultados no arquivo saida.txt
    FILE* saida = fopen("saida.txt", "w");
    if (!saida) return 1;

    if (custo >= INF) {
        fprintf(saida, "Não existe caminho entre %d e %d\n", origem, destino);
    } else {
        fprintf(saida, "Custo total do caminho: %.2lf unidades\n", custo);
        fprintf(saida, "Nós explorados: %d\n", nos);
        fprintf(saida, "\nCaminho (do início ao fim):\n");

        int caminho[MAX_VERTICES], tam = 0;
        // Reconstrução do caminho invertendo predecessores
        for (int v = destino; v != -1; v = anterior[v])
            caminho[tam++] = v;
        // Imprime o caminho do início para o fim
        for (int i = tam - 1; i >= 0; i--)
            fprintf(saida, "%d (x=%.3lf, y=%.3lf)\n", caminho[i], grafo[caminho[i]].x, grafo[caminho[i]].y);
    }
    fprintf(saida, "\nTempo de execução: %.4lf segundos\n", tempo);
    fclose(saida);

    return 0;
}

