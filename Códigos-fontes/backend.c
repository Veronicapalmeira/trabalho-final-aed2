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

#define MAX_VERTICES 10000   // Limite m�ximo de v�rtices do grafo, para suportar grafos grandes
#define INF 1e9             // Valor usado para representar dist�ncia "infinita" (inacess�vel)

// ------------------------ ESTRUTURAS ------------------------ //
// Representa uma aresta na lista encadeada de adjac�ncia
typedef struct Aresta {
    int destino;           // �ndice do v�rtice destino da aresta
    double peso;           // Peso da aresta, aqui calculado pela dist�ncia Euclidiana
    struct Aresta* prox;   // Ponteiro para a pr�xima aresta na lista encadeada
} Aresta;

// Representa um v�rtice do grafo
typedef struct {
    int id;                // ID original do v�rtice conforme o arquivo .poly
    double x, y;           // Coordenadas do v�rtice no plano cartesiano
    Aresta* lista;         // Ponteiro para a lista encadeada de arestas adjacentes (lista de adjac�ncia)
} Vertice;

// Nodo para a heap m�nima utilizada no algoritmo de Dijkstra
typedef struct {
    int id;                // �ndice do v�rtice no grafo
    double dist;           // Dist�ncia acumulada do v�rtice origem at� este v�rtice
} NodoHeap;

// ------------------------ VARI�VEIS GLOBAIS ------------------------ //
Vertice grafo[MAX_VERTICES];  // Vetor que armazena todos os v�rtices do grafo
int totalVertices = 0;        // Quantidade atual de v�rtices carregados no grafo
int totalArestas = 0;         // Quantidade atual de arestas carregadas no grafo

// Mapeamento do ID original dos v�rtices (do arquivo .poly) para �ndice interno no vetor 'grafo'
// Inicializado com -1 para detectar IDs inv�lidos ou n�o mapeados
int idParaIndice[MAX_VERTICES];

// ------------------------ FUN��ES AUXILIARES ------------------------ //
// Calcula a dist�ncia Euclidiana entre dois v�rtices dados pelos seus �ndices internos
double distancia(int a, int b) {
    double dx = grafo[a].x - grafo[b].x;  // Diferen�a em x
    double dy = grafo[a].y - grafo[b].y;  // Diferen�a em y
    return sqrt(dx*dx + dy*dy);            // Dist�ncia Euclidiana (hipotenusa)
}

// Adiciona uma aresta no grafo, criando uma lista de adjac�ncia para o v�rtice origem
// Se direcao == 0, a aresta � bidirecional, ent�o adiciona tamb�m a aresta inversa no destino
void adicionarAresta(int origem, int destino, int direcao) {
    // Cria nova aresta para o v�rtice origem
    Aresta* nova = malloc(sizeof(Aresta));
    nova->destino = destino;
    nova->peso = distancia(origem, destino);  // Peso � a dist�ncia Euclidiana calculada
    nova->prox = grafo[origem].lista;         // Insere no in�cio da lista encadeada
    grafo[origem].lista = nova;

    if (direcao == 0) {
        // Cria a aresta inversa para grafo n�o direcionado (bidirecional)
        Aresta* inversa = malloc(sizeof(Aresta));
        inversa->destino = origem;
        inversa->peso = nova->peso;            // Peso igual � aresta original
        inversa->prox = grafo[destino].lista;
        grafo[destino].lista = inversa;
    }
}

// ------------------------ HEAP M�NIMA ------------------------ //
// Array que representa a heap m�nima para organizar os v�rtices conforme dist�ncia acumulada
NodoHeap heap[MAX_VERTICES];

// posHeap mapeia o �ndice do v�rtice para sua posi��o atual dentro da heap
// usado para encontrar a posi��o r�pida de um v�rtice e atualizar sua dist�ncia
int posHeap[MAX_VERTICES];

// Tamanho atual da heap
int tamHeap = 0;

// Troca os elementos na heap nas posi��es i e j, e atualiza posHeap para manter coer�ncia
void trocar(int i, int j) {
    NodoHeap tmp = heap[i];
    heap[i] = heap[j];
    heap[j] = tmp;
    posHeap[heap[i].id] = i;
    posHeap[heap[j].id] = j;
}

// Sobe um elemento na heap para garantir a propriedade de heap m�nima
// enquanto o elemento estiver menor que o seu pai, troca com o pai
void subir(int i) {
    while (i > 0 && heap[i].dist < heap[(i - 1) / 2].dist) {
        trocar(i, (i - 1) / 2);
        i = (i - 1) / 2;
    }
}

// Desce um elemento na heap para garantir a propriedade de heap m�nima
// compara com filhos e troca com o menor deles se necess�rio
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

// Insere um novo v�rtice na heap, colocando-o no final e subindo para a posi��o correta
void inserirHeap(int id, double dist) {
    heap[tamHeap].id = id;
    heap[tamHeap].dist = dist;
    posHeap[id] = tamHeap;
    subir(tamHeap);
    tamHeap++;
}

// Remove e retorna o v�rtice com menor dist�ncia acumulada da heap (raiz da heap)
int extrairMin() {
    int min = heap[0].id;         // V�rtice da raiz da heap (menor dist�ncia)
    heap[0] = heap[--tamHeap];    // Move �ltimo elemento para a raiz
    posHeap[heap[0].id] = 0;      // Atualiza posi��o do elemento que foi para a raiz
    descer(0);                    // Ajusta a heap descendo o elemento
    return min;                   // Retorna o v�rtice removido (menor dist�ncia)
}

// Atualiza a dist�ncia de um v�rtice na heap e sobe ele para a posi��o correta
void diminuirChave(int id, double novaDist) {
    int i = posHeap[id];
    heap[i].dist = novaDist;
    subir(i);
}

// ------------------------ LEITURA DO ARQUIVO .POLY ------------------------ //
// L� o arquivo .poly com formato esperado e popula o grafo com v�rtices e arestas
// Retorna 1 se leitura bem sucedida, 0 se erro
int lerArquivoPoly(char* nome) {
    FILE* f = fopen(nome, "r");
    if (!f) return 0;

    // Inicializa o mapeamento dos IDs com -1 (n�o mapeados)
    for (int i = 0; i < MAX_VERTICES; i++) {
        idParaIndice[i] = -1;
    }

    // L� o cabe�alho do arquivo .poly (n�mero de v�rtices e alguns par�metros n�o usados)
    int lixo1, lixo2, lixo3;
    if (fscanf(f, "%d %d %d %d", &totalVertices, &lixo1, &lixo2, &lixo3) != 4) {
        fclose(f);
        return 0;
    }
    if (totalVertices > MAX_VERTICES) {
        printf("Erro: n�mero de v�rtices excede limite m�ximo (%d)\n", MAX_VERTICES);
        fclose(f);
        return 0;
    }

    // L� os v�rtices: id, coordenadas x e y
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
        grafo[i].lista = NULL;  // Inicializa lista de adjac�ncia vazia

        // Valida ID para evitar �ndice inv�lido no array
        if (id < 0 || id >= MAX_VERTICES) {
            printf("Erro: ID de v�rtice inv�lido: %d\n", id);
            fclose(f);
            return 0;
        }
        idParaIndice[id] = i;  // Mapeia ID para �ndice interno
    }

    // L� n�mero de arestas (e outro par�metro n�o usado)
    if (fscanf(f, "%d %d", &totalArestas, &lixo1) != 2) {
        fclose(f);
        return 0;
    }

    // L� as arestas: id da aresta, origem, destino, dire��o (0 = bidirecional, 1 = direcionado)
    for (int i = 0; i < totalArestas; i++) {
        int id, o, d, dir;
        if (fscanf(f, "%d %d %d %d", &id, &o, &d, &dir) != 4) {
            fclose(f);
            return 0;
        }

        // Converte os IDs de v�rtices para �ndices internos do vetor grafo
        int o_idx = (o >= 0 && o < MAX_VERTICES) ? idParaIndice[o] : -1;
        int d_idx = (d >= 0 && d < MAX_VERTICES) ? idParaIndice[d] : -1;

        // Verifica se os v�rtices de origem e destino existem no grafo
        if (o_idx == -1 || d_idx == -1) {
            printf("Erro: aresta com v�rtice inexistente (origem %d, destino %d)\n", o, d);
            fclose(f);
            return 0;
        }
        adicionarAresta(o_idx, d_idx, dir);  // Adiciona a aresta no grafo
    }

    fclose(f);
    return 1;  // Sucesso na leitura do arquivo
}

// ------------------------ ALGORITMO DE DIJKSTRA ------------------------ //
// Implementa o algoritmo para encontrar o caminho m�nimo entre v�rtices
// origem e destino s�o �ndices internos dos v�rtices no grafo
// nosExplorados armazena a quantidade de n�s visitados
// custoFinal recebe o custo total do caminho encontrado
// anterior � vetor que armazena o v�rtice predecessor para reconstru��o do caminho
void dijkstra(int origem, int destino, int* nosExplorados, double* custoFinal, int* anterior) {
    double dist[MAX_VERTICES];         // Dist�ncias m�nimas acumuladas da origem
    int visitado[MAX_VERTICES] = {0}; // Marca se o v�rtice j� foi visitado
    *nosExplorados = 0;                // Inicializa contador de n�s explorados

    // Inicializa dist�ncias com infinito e predecessores inv�lidos (-1)
    for (int i = 0; i < totalVertices; i++) {
        dist[i] = INF;
        anterior[i] = -1;
    }
    dist[origem] = 0;  // Dist�ncia do v�rtice origem para ele mesmo � zero

    tamHeap = 0;       // Inicializa heap vazia
    inserirHeap(origem, 0);  // Insere v�rtice origem na heap

    while (tamHeap > 0) {
        int u = extrairMin();  // Remove v�rtice com menor dist�ncia acumulada
        if (visitado[u]) continue;  // Se j� visitado, ignora
        visitado[u] = 1;
        (*nosExplorados)++;  // Incrementa contador de n�s explorados

        // Relaxa todas as arestas adjacentes a u
        for (Aresta* a = grafo[u].lista; a != NULL; a = a->prox) {
            int v = a->destino;
            // Se n�o visitado e novo caminho menor, atualiza dist�ncia e predecessor
            if (!visitado[v] && dist[u] + a->peso < dist[v]) {
                dist[v] = dist[u] + a->peso;
                anterior[v] = u;
                inserirHeap(v, dist[v]);  // Insere ou atualiza heap com nova dist�ncia
            }
        }
    }

    *custoFinal = dist[destino];  // Define o custo final encontrado para o destino
}

// ------------------------ FUN��O PRINCIPAL ------------------------ //
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
        printf("Erro: arquivo entrada.txt n�o encontrado\n");
        return 1;
    }

    int origemId, destinoId;
    // L� os IDs de origem e destino
    if (fscanf(entrada, "%d", &origemId) != 1 || fscanf(entrada, "%d", &destinoId) != 1) {
        fclose(entrada);
        printf("Erro: origem ou destino inv�lidos em entrada.txt\n");
        return 1;
    }
    fclose(entrada);

    // Converte IDs originais para �ndices internos do vetor grafo
    int origem = idParaIndice[origemId];
    int destino = idParaIndice[destinoId];
    if (origem == -1 || destino == -1) {
        printf("Erro: origem ou destino n�o encontrados no grafo\n");
        return 1;
    }

    // Executa Dijkstra e mede tempo de execu��o
    clock_t ini = clock();
    int nos = 0, anterior[MAX_VERTICES];
    double custo;
    dijkstra(origem, destino, &nos, &custo, anterior);
    clock_t fim = clock();
    double tempo = (double)(fim - ini) / CLOCKS_PER_SEC;

    // Abre arquivo saida.txt para salvar resultados do caminho encontrado
    FILE* saida = fopen("saida.txt", "w");
    if (!saida) {
        printf("Erro: n�o foi poss�vel criar saida.txt\n");
        return 1;
    }

    // Se custo for infinito, n�o existe caminho
    if (custo >= INF) {
        fprintf(saida, "N�o existe caminho entre %d e %d\n", origemId, destinoId);
    } else {
        // Escreve custo total, n�s explorados e caminho detalhado no arquivo
        fprintf(saida, "Custo total do caminho: %.2lf unidades\n", custo);
        fprintf(saida, "N�s explorados: %d\n", nos);
        fprintf(saida, "\nCaminho (do in�cio ao fim):\n");

        int caminho[MAX_VERTICES], tam = 0;
        // Reconstr�i o caminho de destino at� a origem usando o vetor anterior
        for (int v = destino; v != -1; v = anterior[v])
            caminho[tam++] = v;

        // Imprime o caminho na ordem correta: origem at� destino
        for (int i = tam - 1; i >= 0; i--) {
            int idx = caminho[i];
            fprintf(saida, "%d (x=%.3lf, y=%.3lf)\n", grafo[idx].id, grafo[idx].x, grafo[idx].y);
        }
    }
    fprintf(saida, "\nTempo de execu��o: %.4lf segundos\n", tempo);
    fclose(saida);

    return 0;
}

