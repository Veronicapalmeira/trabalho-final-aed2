/*
    Projeto Final - AED2 - INF/UFG 2025-1
    Tema: Sistema de Navega��o Primitivo
    Back-end com Fila de Prioridade (Heap M�nima) e Lista de Adjac�ncia
    Autores: Ana Luisa, Isadora, Lucas e Ver�nica
    Professor: Andr� Luiz Moura
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define MAX_VERTICES 1000   // Limite m�ximo de v�rtices do grafo
#define INF 1e9             // Representa��o de "infinito" para dist�ncias

// ------------------------ ESTRUTURAS ------------------------ //
// Estrutura para uma aresta na lista de adjac�ncia
typedef struct Aresta {
    int destino;           // �ndice do v�rtice destino da aresta
    double peso;           // Peso (dist�ncia) da aresta
    struct Aresta* prox;   // Pr�xima aresta na lista encadeada
} Aresta;

// Estrutura para um v�rtice do grafo
typedef struct {
    int id;                // Identificador do v�rtice
    double x, y;           // Coordenadas do v�rtice no plano
    Aresta* lista;         // Lista encadeada de arestas adjacentes
} Vertice;

// Estrutura para armazenar um n� na heap m�nima
typedef struct {
    int id;                // �ndice do v�rtice
    double dist;           // Dist�ncia acumulada para Dijkstra
} NodoHeap;

// ------------------------ VARI�VEIS GLOBAIS ------------------------ //
Vertice grafo[MAX_VERTICES];  // Vetor com todos os v�rtices do grafo
int totalVertices = 0;        // Quantidade atual de v�rtices
int totalArestas = 0;         // Quantidade atual de arestas

// ------------------------ FUN��ES AUXILIARES ------------------------ //
// Calcula dist�ncia Euclidiana entre dois v�rtices do grafo pelo �ndice
double distancia(int a, int b) {
    double dx = grafo[a].x - grafo[b].x;
    double dy = grafo[a].y - grafo[b].y;
    return sqrt(dx*dx + dy*dy);
}

// Adiciona uma aresta ao grafo (lista de adjac�ncia)
// Se direcao == 0, a aresta � bidirecional (insere a inversa tamb�m)
void adicionarAresta(int origem, int destino, int direcao) {
    Aresta* nova = malloc(sizeof(Aresta));
    nova->destino = destino;
    nova->peso = distancia(origem, destino);
    nova->prox = grafo[origem].lista;
    grafo[origem].lista = nova;

    if (direcao == 0) {
        // Insere a aresta inversa para grafo n�o direcionado
        Aresta* inversa = malloc(sizeof(Aresta));
        inversa->destino = origem;
        inversa->peso = nova->peso;
        inversa->prox = grafo[destino].lista;
        grafo[destino].lista = inversa;
    }
}

// ------------------------ HEAP M�NIMA ------------------------ //
// Estrutura e fun��es para gerenciar uma heap m�nima para Dijkstra
NodoHeap heap[MAX_VERTICES];  // Array que representa a heap
int posHeap[MAX_VERTICES];    // Mapeia o �ndice do v�rtice para sua posi��o na heap
int tamHeap = 0;              // Tamanho atual da heap

// Troca dois elementos da heap e atualiza as posi��es no posHeap
void trocar(int i, int j) {
    NodoHeap tmp = heap[i];
    heap[i] = heap[j];
    heap[j] = tmp;
    posHeap[heap[i].id] = i;
    posHeap[heap[j].id] = j;
}

// Sobe um elemento na heap para manter a propriedade de heap m�nima
void subir(int i) {
    while (i > 0 && heap[i].dist < heap[(i - 1) / 2].dist) {
        trocar(i, (i - 1) / 2);
        i = (i - 1) / 2;
    }
}

// Desce um elemento na heap para manter a propriedade de heap m�nima
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

// Insere um novo v�rtice na heap com sua dist�ncia inicial
void inserirHeap(int id, double dist) {
    heap[tamHeap].id = id;
    heap[tamHeap].dist = dist;
    posHeap[id] = tamHeap;
    subir(tamHeap);
    tamHeap++;
}

// Remove e retorna o v�rtice com menor dist�ncia da heap
int extrairMin() {
    int min = heap[0].id;
    heap[0] = heap[--tamHeap];
    posHeap[heap[0].id] = 0;
    descer(0);
    return min;
}

// Atualiza a dist�ncia de um v�rtice j� presente na heap e ajusta a posi��o
void diminuirChave(int id, double novaDist) {
    int i = posHeap[id];
    heap[i].dist = novaDist;
    subir(i);
}

// ------------------------ FUN��O PARA LEITURA DO ARQUIVO .POLY ------------------------ //
// L� o arquivo de entrada com v�rtices e arestas no formato .poly
// Preenche a estrutura do grafo com v�rtices e arestas
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
// origem: �ndice do v�rtice origem
// destino: �ndice do v�rtice destino
// nosExplorados: ponteiro para armazenar quantos n�s foram explorados
// custoFinal: ponteiro para armazenar custo total do caminho
// anterior: vetor para armazenar o predecessor de cada n� no caminho
void dijkstra(int origem, int destino, int* nosExplorados, double* custoFinal, int* anterior) {
    double dist[MAX_VERTICES];
    int visitado[MAX_VERTICES] = {0};
    *nosExplorados = 0;

    // Inicializa��o: dist�ncias infinitas, predecessores inv�lidos
    for (int i = 0; i < totalVertices; i++) {
        dist[i] = INF;
        anterior[i] = -1;
    }
    dist[origem] = 0;

    // Inicializa a heap com o n� de origem
    tamHeap = 0;
    inserirHeap(origem, 0);

    while (tamHeap > 0) {
        int u = extrairMin();
        if (visitado[u]) continue; // Ignora v�rtices j� visitados
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

// ------------------------ FUN��O PRINCIPAL ------------------------ //
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Informe o nome do arquivo .poly como argumento\n");
        return 1;
    }
    if (!lerArquivoPoly(argv[1])) {
        printf("Erro ao ler o arquivo.\n");
        return 1;
    }

    // L� origem e destino do arquivo entrada.txt
    FILE* entrada = fopen("entrada.txt", "r");
    if (!entrada) return 1;
    int origem, destino;
    fscanf(entrada, "%d", &origem);
    fscanf(entrada, "%d", &destino);
    fclose(entrada);

    // Executa Dijkstra e mede tempo de execu��o
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
        fprintf(saida, "N�o existe caminho entre %d e %d\n", origem, destino);
    } else {
        fprintf(saida, "Custo total do caminho: %.2lf unidades\n", custo);
        fprintf(saida, "N�s explorados: %d\n", nos);
        fprintf(saida, "\nCaminho (do in�cio ao fim):\n");

        int caminho[MAX_VERTICES], tam = 0;
        // Reconstru��o do caminho invertendo predecessores
        for (int v = destino; v != -1; v = anterior[v])
            caminho[tam++] = v;
        // Imprime o caminho do in�cio para o fim
        for (int i = tam - 1; i >= 0; i--)
            fprintf(saida, "%d (x=%.3lf, y=%.3lf)\n", caminho[i], grafo[caminho[i]].x, grafo[caminho[i]].y);
    }
    fprintf(saida, "\nTempo de execu��o: %.4lf segundos\n", tempo);
    fclose(saida);

    return 0;
}

