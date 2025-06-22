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

#define MAX_VERTICES 10000   // Limite máximo de vértices do grafo, para suportar grafos grandes
#define INF 1e9             // Valor usado para representar distância "infinita" (inacessível)

// ------------------------ ESTRUTURAS ------------------------ //
// Representa uma aresta na lista encadeada de adjacência
typedef struct Aresta {
    int destino;           // Índice do vértice destino da aresta
    double peso;           // Peso da aresta, aqui calculado pela distância Euclidiana
    struct Aresta* prox;   // Ponteiro para a próxima aresta na lista encadeada
} Aresta;

// Representa um vértice do grafo
typedef struct {
    int id;                // ID original do vértice conforme o arquivo .poly
    double x, y;           // Coordenadas do vértice no plano cartesiano
    Aresta* lista;         // Ponteiro para a lista encadeada de arestas adjacentes (lista de adjacência)
} Vertice;

// Nodo para a heap mínima utilizada no algoritmo de Dijkstra
typedef struct {
    int id;                // Índice do vértice no grafo
    double dist;           // Distância acumulada do vértice origem até este vértice
} NodoHeap;

// ------------------------ VARIÁVEIS GLOBAIS ------------------------ //
Vertice grafo[MAX_VERTICES];  // Vetor que armazena todos os vértices do grafo
int totalVertices = 0;        // Quantidade atual de vértices carregados no grafo
int totalArestas = 0;         // Quantidade atual de arestas carregadas no grafo

// Mapeamento do ID original dos vértices (do arquivo .poly) para índice interno no vetor 'grafo'
// Inicializado com -1 para detectar IDs inválidos ou não mapeados
int idParaIndice[MAX_VERTICES];

// ------------------------ FUNÇÕES AUXILIARES ------------------------ //
// Calcula a distância Euclidiana entre dois vértices dados pelos seus índices internos
double distancia(int a, int b) {
    double dx = grafo[a].x - grafo[b].x;  // Diferença em x
    double dy = grafo[a].y - grafo[b].y;  // Diferença em y
    return sqrt(dx*dx + dy*dy);            // Distância Euclidiana (hipotenusa)
}

// Adiciona uma aresta no grafo, criando uma lista de adjacência para o vértice origem
// Se direcao == 0, a aresta é bidirecional, então adiciona também a aresta inversa no destino
void adicionarAresta(int origem, int destino, int direcao) {
    // Cria nova aresta para o vértice origem
    Aresta* nova = malloc(sizeof(Aresta));
    nova->destino = destino;
    nova->peso = distancia(origem, destino);  // Peso é a distância Euclidiana calculada
    nova->prox = grafo[origem].lista;         // Insere no início da lista encadeada
    grafo[origem].lista = nova;

    if (direcao == 0) {
        // Cria a aresta inversa para grafo não direcionado (bidirecional)
        Aresta* inversa = malloc(sizeof(Aresta));
        inversa->destino = origem;
        inversa->peso = nova->peso;            // Peso igual à aresta original
        inversa->prox = grafo[destino].lista;
        grafo[destino].lista = inversa;
    }
}

// ------------------------ HEAP MÍNIMA ------------------------ //
// Array que representa a heap mínima para organizar os vértices conforme distância acumulada
NodoHeap heap[MAX_VERTICES];

// posHeap mapeia o índice do vértice para sua posição atual dentro da heap
// usado para encontrar a posição rápida de um vértice e atualizar sua distância
int posHeap[MAX_VERTICES];

// Tamanho atual da heap
int tamHeap = 0;

// Troca os elementos na heap nas posições i e j, e atualiza posHeap para manter coerência
void trocar(int i, int j) {
    NodoHeap tmp = heap[i];
    heap[i] = heap[j];
    heap[j] = tmp;
    posHeap[heap[i].id] = i;
    posHeap[heap[j].id] = j;
}

// Sobe um elemento na heap para garantir a propriedade de heap mínima
// enquanto o elemento estiver menor que o seu pai, troca com o pai
void subir(int i) {
    while (i > 0 && heap[i].dist < heap[(i - 1) / 2].dist) {
        trocar(i, (i - 1) / 2);
        i = (i - 1) / 2;
    }
}

// Desce um elemento na heap para garantir a propriedade de heap mínima
// compara com filhos e troca com o menor deles se necessário
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

// Insere um novo vértice na heap, colocando-o no final e subindo para a posição correta
void inserirHeap(int id, double dist) {
    heap[tamHeap].id = id;
    heap[tamHeap].dist = dist;
    posHeap[id] = tamHeap;
    subir(tamHeap);
    tamHeap++;
}

// Remove e retorna o vértice com menor distância acumulada da heap (raiz da heap)
int extrairMin() {
    int min = heap[0].id;         // Vértice da raiz da heap (menor distância)
    heap[0] = heap[--tamHeap];    // Move último elemento para a raiz
    posHeap[heap[0].id] = 0;      // Atualiza posição do elemento que foi para a raiz
    descer(0);                    // Ajusta a heap descendo o elemento
    return min;                   // Retorna o vértice removido (menor distância)
}

// Atualiza a distância de um vértice na heap e sobe ele para a posição correta
void diminuirChave(int id, double novaDist) {
    int i = posHeap[id];
    heap[i].dist = novaDist;
    subir(i);
}

// ------------------------ LEITURA DO ARQUIVO .POLY ------------------------ //
// Lê o arquivo .poly com formato esperado e popula o grafo com vértices e arestas
// Retorna 1 se leitura bem sucedida, 0 se erro
int lerArquivoPoly(char* nome) {
    FILE* f = fopen(nome, "r");
    if (!f) return 0;

    // Inicializa o mapeamento dos IDs com -1 (não mapeados)
    for (int i = 0; i < MAX_VERTICES; i++) {
        idParaIndice[i] = -1;
    }

    // Lê o cabeçalho do arquivo .poly (número de vértices e alguns parâmetros não usados)
    int lixo1, lixo2, lixo3;
    if (fscanf(f, "%d %d %d %d", &totalVertices, &lixo1, &lixo2, &lixo3) != 4) {
        fclose(f);
        return 0;
    }
    if (totalVertices > MAX_VERTICES) {
        printf("Erro: número de vértices excede limite máximo (%d)\n", MAX_VERTICES);
        fclose(f);
        return 0;
    }

    // Lê os vértices: id, coordenadas x e y
    for (int i = 0; i < totalVertices; i++) {
        int id; 
        double x, y;
        if (fscanf(f, "%d %lf %lf", &id, &x, &y) != 3) {
            fclose(f);
            return 0;
        }
        grafo[i].id = id;
        grafo[i].x = x;
        grafo[i].y = y;
        grafo[i].lista = NULL;  // Inicializa lista de adjacência vazia

        // Valida ID para evitar índice inválido no array
        if (id < 0 || id >= MAX_VERTICES) {
            printf("Erro: ID de vértice inválido: %d\n", id);
            fclose(f);
            return 0;
        }
        idParaIndice[id] = i;  // Mapeia ID para índice interno
    }

    // Lê número de arestas (e outro parâmetro não usado)
    if (fscanf(f, "%d %d", &totalArestas, &lixo1) != 2) {
        fclose(f);
        return 0;
    }

    // Lê as arestas: id da aresta, origem, destino, direção (0 = bidirecional, 1 = direcionado)
    for (int i = 0; i < totalArestas; i++) {
        int id, o, d, dir;
        if (fscanf(f, "%d %d %d %d", &id, &o, &d, &dir) != 4) {
            fclose(f);
            return 0;
        }

        // Converte os IDs de vértices para índices internos do vetor grafo
        int o_idx = (o >= 0 && o < MAX_VERTICES) ? idParaIndice[o] : -1;
        int d_idx = (d >= 0 && d < MAX_VERTICES) ? idParaIndice[d] : -1;

        // Verifica se os vértices de origem e destino existem no grafo
        if (o_idx == -1 || d_idx == -1) {
            printf("Erro: aresta com vértice inexistente (origem %d, destino %d)\n", o, d);
            fclose(f);
            return 0;
        }
        adicionarAresta(o_idx, d_idx, dir);  // Adiciona a aresta no grafo
    }

    fclose(f);
    return 1;  // Sucesso na leitura do arquivo
}

// ------------------------ ALGORITMO DE DIJKSTRA ------------------------ //
// Implementa o algoritmo para encontrar o caminho mínimo entre vértices
// origem e destino são índices internos dos vértices no grafo
// nosExplorados armazena a quantidade de nós visitados
// custoFinal recebe o custo total do caminho encontrado
// anterior é vetor que armazena o vértice predecessor para reconstrução do caminho
void dijkstra(int origem, int destino, int* nosExplorados, double* custoFinal, int* anterior) {
    double dist[MAX_VERTICES];         // Distâncias mínimas acumuladas da origem
    int visitado[MAX_VERTICES] = {0}; // Marca se o vértice já foi visitado
    *nosExplorados = 0;                // Inicializa contador de nós explorados

    // Inicializa distâncias com infinito e predecessores inválidos (-1)
    for (int i = 0; i < totalVertices; i++) {
        dist[i] = INF;
        anterior[i] = -1;
    }
    dist[origem] = 0;  // Distância do vértice origem para ele mesmo é zero

    tamHeap = 0;       // Inicializa heap vazia
    inserirHeap(origem, 0);  // Insere vértice origem na heap

    while (tamHeap > 0) {
        int u = extrairMin();  // Remove vértice com menor distância acumulada
        if (visitado[u]) continue;  // Se já visitado, ignora
        visitado[u] = 1;
        (*nosExplorados)++;  // Incrementa contador de nós explorados

        // Relaxa todas as arestas adjacentes a u
        for (Aresta* a = grafo[u].lista; a != NULL; a = a->prox) {
            int v = a->destino;
            // Se não visitado e novo caminho menor, atualiza distância e predecessor
            if (!visitado[v] && dist[u] + a->peso < dist[v]) {
                dist[v] = dist[u] + a->peso;
                anterior[v] = u;
                inserirHeap(v, dist[v]);  // Insere ou atualiza heap com nova distância
            }
        }
    }

    *custoFinal = dist[destino];  // Define o custo final encontrado para o destino
}

// ------------------------ FUNÇÃO PRINCIPAL ------------------------ //
int main(int argc, char* argv[]) {
    // Verifica se arquivo .poly foi passado como argumento
    if (argc < 2) {
        printf("Informe o nome do arquivo .poly como argumento\n");
        return 1;
    }

    // Tenta ler o arquivo .poly, caso falhe termina o programa
    if (!lerArquivoPoly(argv[1])) {
        printf("Erro ao ler o arquivo.\n");
        return 1;
    }

    // Abre arquivo entrada.txt para ler origem e destino em IDs originais
    FILE* entrada = fopen("entrada.txt", "r");
    if (!entrada) {
        printf("Erro: arquivo entrada.txt não encontrado\n");
        return 1;
    }

    int origemId, destinoId;
    // Lê os IDs de origem e destino
    if (fscanf(entrada, "%d", &origemId) != 1 || fscanf(entrada, "%d", &destinoId) != 1) {
        fclose(entrada);
        printf("Erro: origem ou destino inválidos em entrada.txt\n");
        return 1;
    }
    fclose(entrada);

    // Converte IDs originais para índices internos do vetor grafo
    int origem = idParaIndice[origemId];
    int destino = idParaIndice[destinoId];
    if (origem == -1 || destino == -1) {
        printf("Erro: origem ou destino não encontrados no grafo\n");
        return 1;
    }

    // Executa Dijkstra e mede tempo de execução
    clock_t ini = clock();
    int nos = 0, anterior[MAX_VERTICES];
    double custo;
    dijkstra(origem, destino, &nos, &custo, anterior);
    clock_t fim = clock();
    double tempo = (double)(fim - ini) / CLOCKS_PER_SEC;

    // Abre arquivo saida.txt para salvar resultados do caminho encontrado
    FILE* saida = fopen("saida.txt", "w");
    if (!saida) {
        printf("Erro: não foi possível criar saida.txt\n");
        return 1;
    }

    // Se custo for infinito, não existe caminho
    if (custo >= INF) {
        fprintf(saida, "Não existe caminho entre %d e %d\n", origemId, destinoId);
    } else {
        // Escreve custo total, nós explorados e caminho detalhado no arquivo
        fprintf(saida, "Custo total do caminho: %.2lf unidades\n", custo);
        fprintf(saida, "Nós explorados: %d\n", nos);
        fprintf(saida, "\nCaminho (do início ao fim):\n");

        int caminho[MAX_VERTICES], tam = 0;
        // Reconstrói o caminho de destino até a origem usando o vetor anterior
        for (int v = destino; v != -1; v = anterior[v])
            caminho[tam++] = v;

        // Imprime o caminho na ordem correta: origem até destino
        for (int i = tam - 1; i >= 0; i--) {
            int idx = caminho[i];
            fprintf(saida, "%d (x=%.3lf, y=%.3lf)\n", grafo[idx].id, grafo[idx].x, grafo[idx].y);
        }
    }
    fprintf(saida, "\nTempo de execução: %.4lf segundos\n", tempo);
    fclose(saida);

    return 0;
}

