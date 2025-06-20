import pygame
import math
import time
import sys

# Configurações iniciais
LARGURA, ALTURA = 1000, 700
RAIO = 10
COR_VERTICE = (0, 102, 204)         # Azul
COR_ARESTA = (0, 0, 0)              # Preto
COR_ARESTA_UNICA = (255, 0, 0)      # Vermelho
COR_ORIGEM = (0, 255, 0)            # Verde
COR_DESTINO = (255, 0, 0)           # Vermelho
COR_CAMINHO = (255, 255, 0)         # Amarelo
FUNDO = (255, 255, 255)             # Branco

# Dados do grafo
vertices = []
arestas = []
matriz_adj = []
direcoes = []

origem_selecionada = None
destino_selecionado = None
caminho = []

# Função para ler arquivo .poly
def ler_poly(nome_arquivo):
    global vertices, arestas, matriz_adj, direcoes
    vertices = []
    arestas = []

    with open(nome_arquivo, 'r') as f:
        # Leitura do header de vértices
        linha = f.readline()
        n_vertices, _, _, _ = map(int, linha.strip().split())
        for _ in range(n_vertices):
            idv, x, y = f.readline().strip().split()
            vertices.append( (int(idv), float(x), float(y)) )

        # Leitura da linha que indica número de arestas
        linha = f.readline()
        n_arestas, _ = map(int, linha.strip().split())

        for _ in range(n_arestas):
            id_a, o, d, dir_a = f.readline().strip().split()
            arestas.append( (int(o), int(d), int(dir_a)) )

    # Construir matriz adjacente com INF
    INF = float('inf')
    n = len(vertices)
    matriz_adj = [ [INF]*n for _ in range(n)]

    # Calcular distâncias e preencher matriz adjacente
    for o, d, dir_a in arestas:
        xo, yo = vertices[o][1], vertices[o][2]
        xd, yd = vertices[d][1], vertices[d][2]
        dist = math.sqrt( (xo - xd)**2 + (yo - yd)**2 )
        matriz_adj[o][d] = dist
        if dir_a == 0:
            matriz_adj[d][o] = dist
    return matriz_adj

# Dijkstra
def dijkstra(inicio, fim, matriz):
    n = len(matriz)
    INF = float('inf')
    dist = [INF]*n
    prev = [-1]*n
    visited = [False]*n

    dist[inicio] = 0
    nos_explorados = 0

    start_time = time.time()

    for _ in range(n):
        u = -1
        min_dist = INF
        for i in range(n):
            if not visited[i] and dist[i] < min_dist:
                min_dist = dist[i]
                u = i
        if u == -1:
            break
        visited[u] = True
        nos_explorados += 1
        for v in range(n):
            if matriz[u][v] < INF and dist[u] + matriz[u][v] < dist[v]:
                dist[v] = dist[u] + matriz[u][v]
                prev[v] = u

    end_time = time.time()
    tempo_exec = end_time - start_time

    if dist[fim] == INF:
        return None, dist[fim], nos_explorados, tempo_exec

    # Reconstruir caminho
    path = []
    atual = fim
    while atual != -1:
        path.append(atual)
        atual = prev[atual]
    path.reverse()

    return path, dist[fim], nos_explorados, tempo_exec

# Função para desenhar seta na mão única
def desenhar_seta(tela, inicio, fim, cor, tamanho=12):
    dx = fim[0] - inicio[0]
    dy = fim[1] - inicio[1]
    ang = math.atan2(dy, dx)
    # Ponto final da linha (menos um pouco para a ponta)
    fim_seta = (fim[0] - RAIO * math.cos(ang), fim[1] - RAIO * math.sin(ang))
    pygame.draw.line(tela, cor, inicio, fim_seta, 2)
    # Desenha triângulo da ponta
    p1 = (fim_seta[0] - tamanho * math.cos(ang - math.pi/6), fim_seta[1] - tamanho * math.sin(ang - math.pi/6))
    p2 = (fim_seta[0] - tamanho * math.cos(ang + math.pi/6), fim_seta[1] - tamanho * math.sin(ang + math.pi/6))
    pygame.draw.polygon(tela, cor, [fim, p1, p2])

# Checar se clicou dentro do vértice
def clique_em_vertice(pos):
    x, y = pos
    for i, (_, vx, vy) in enumerate(vertices):
        # Transformar as coordenadas do grafo para a tela (escala e deslocamento)
        sx, sy = transformar_coordenadas(vx, vy)
        if (x - sx)**2 + (y - sy)**2 <= RAIO**2:
            return i
    return None

# Transformar coordenadas reais para tela (escala e deslocamento)
def transformar_coordenadas(x, y):
    # Ajustar para caber na tela, escala linear simples
    # Definir escala e offset (pode ajustar para seu mapa)
    escala_x = 0.6
    escala_y = 0.6
    offset_x = 50
    offset_y = 50
    sx = int(x * escala_x) + offset_x
    sy = int(y * escala_y) + offset_y
    return sx, sy

def main():
    global origem_selecionada, destino_selecionado, caminho

    pygame.init()
    tela = pygame.display.set_mode((LARGURA, ALTURA))
    pygame.display.set_caption("Sistema de Navegação Primitivo - INF/UFG")
    fonte = pygame.font.SysFont(None, 24)

    matriz = ler_poly("SetorGoiania2.poly")

    rodando = True
    clock = pygame.time.Clock()

    info_text = ""

    while rodando:
        tela.fill(FUNDO)

        # Desenhar arestas
        for o, d, dir_a in arestas:
            xo, yo = transformar_coordenadas(vertices[o][1], vertices[o][2])
            xd, yd = transformar_coordenadas(vertices[d][1], vertices[d][2])
            cor_aresta = COR_ARESTA_UNICA if dir_a == 1 else COR_ARESTA
            if dir_a == 0:
                # mão dupla linha simples
                pygame.draw.line(tela, cor_aresta, (xo, yo), (xd, yd), 2)
            else:
                # mão única com seta
                desenhar_seta(tela, (xo, yo), (xd, yd), cor_aresta, tamanho=12)

        # Desenhar vértices
        for i, (_, x, y) in enumerate(vertices):
            sx, sy = transformar_coordenadas(x, y)
            cor = COR_VERTICE
            if origem_selecionada == i:
                cor = COR_ORIGEM
            elif destino_selecionado == i:
                cor = COR_DESTINO
            pygame.draw.circle(tela, cor, (sx, sy), RAIO)
            # Número do vértice
            texto = fonte.render(str(i), True, (0, 0, 0))
            tela.blit(texto, (sx - RAIO, sy - RAIO - 15))

        # Desenhar caminho mínimo em amarelo
        if caminho and len(caminho) > 1:
            for i in range(len(caminho) - 1):
                o = caminho[i]
                d = caminho[i + 1]
                xo, yo = transformar_coordenadas(vertices[o][1], vertices[o][2])
                xd, yd = transformar_coordenadas(vertices[d][1], vertices[d][2])
                pygame.draw.line(tela, COR_CAMINHO, (xo, yo), (xd, yd), 4)

        # Mostrar texto de info
        y_text = 10
        for linha in info_text.split('\n'):
            render = fonte.render(linha, True, (0, 0, 0))
            tela.blit(render, (10, y_text))
            y_text += 25

        for evento in pygame.event.get():
            if evento.type == pygame.QUIT:
                rodando = False
            elif evento.type == pygame.MOUSEBUTTONDOWN and evento.button == 1:
                pos = pygame.mouse.get_pos()
                vert = clique_em_vertice(pos)
                if vert is not None:
                    if origem_selecionada is None:
                        origem_selecionada = vert
                        info_text = f"Origem selecionada: {vert}"
                    elif destino_selecionado is None:
                        destino_selecionado = vert
                        info_text = f"Destino selecionado: {vert}"
                        # Rodar Dijkstra para calcular caminho
                        caminho_calc, custo, nos_exp, tempo_exec = dijkstra(origem_selecionada, destino_selecionado, matriz)
                        if caminho_calc is None:
                            info_text = f"Não existe caminho entre {origem_selecionada} e {destino_selecionado}"
                            caminho = []
                        else:
                            caminho = caminho_calc
                            info_text = (f"Custo total do caminho: {custo:.2f} unidades\n"
                                         f"Nós explorados: {nos_exp}\n"
                                         f"Tempo de execução: {tempo_exec:.4f} segundos")
                    else:
                        # Resetar seleção para novo cálculo
                        origem_selecionada = vert
                        destino_selecionado = None
                        caminho = []
                        info_text = f"Origem selecionada: {vert}"

        pygame.display.flip()
        clock.tick(30)

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
