# Sistema de Navegação Primitivo - INF/UFG 2025-1
# Front-end com scroll dinâmico no resultado
# Autoras: Ana Luisa, Isadora, Lucas e Verônica

import pygame
import math
import tkinter as tk
from tkinter import filedialog
import subprocess
import os

LARGURA, ALTURA = 1360, 685
LARGURA_MENU = 320
ALTURA_CAIXA_TEXTO = 350
COR_VERTICE = (0, 102, 204)
COR_ARESTA = (0, 0, 0)
COR_ARESTA_UNICA = (255, 0, 0)
COR_ORIGEM = (0, 255, 0)
COR_DESTINO = (255, 0, 0)
COR_CAMINHO = (255, 255, 0)
FUNDO = (255, 255, 255)
COR_MENU = (245, 245, 245)
COR_SCROLL = (180, 180, 180)
COR_SCROLL_BG = (220, 220, 220)
RAIO_VERTICE = 10

vertices = []
arestas = []
matriz_adj = []
arquivo_poly = ""
origem_selecionada = None
destino_selecionada = None
caminho = []
info_text = "Carregue um arquivo .poly para começar."
scroll_offset = 0
tamanho_ponto = RAIO_VERTICE

def ler_poly(nome_arquivo):
    global vertices, arestas, matriz_adj, arquivo_poly
    arquivo_poly = nome_arquivo
    vertices = []
    arestas = []
    with open(nome_arquivo, 'r') as f:
        n_vertices, _, _, _ = map(int, f.readline().strip().split())
        for _ in range(n_vertices):
            idv, x, y = f.readline().strip().split()
            vertices.append((int(idv), float(x), float(y)))
        n_arestas, _ = map(int, f.readline().strip().split())
        for _ in range(n_arestas):
            _, o, d, dir_a = f.readline().strip().split()
            arestas.append((int(o), int(d), int(dir_a)))
    atualizar_matriz()

def atualizar_matriz():
    global matriz_adj
    n = len(vertices)
    matriz_adj = [[float('inf')] * n for _ in range(n)]
    for o, d, dir_a in arestas:
        dist = math.dist(vertices[o][1:], vertices[d][1:])
        matriz_adj[o][d] = dist
        if dir_a == 0:
            matriz_adj[d][o] = dist

def transformar(x, y):
    escala = 0.46
    return int(x * escala + LARGURA_MENU + 300), int(y * escala + 30)

def clique(pos):
    x, y = pos
    for i, (_, vx, vy) in enumerate(vertices):
        sx, sy = transformar(vx, vy)
        if (x - sx) ** 2 + (y - sy) ** 2 <= tamanho_ponto ** 2:
            return i
    return None

def salvar_grafo_imagem(tela):
    pygame.image.save(tela, "grafo_salvo.png")

def rodar_dijkstra_backend(origem, destino):
    with open("entrada.txt", "w") as f:
        f.write(f"{origem}\n{destino}\n")
    backend_exe = "backend.exe" if os.name == "nt" else "./backend"
    if not os.path.exists(backend_exe):
        return "Erro: backend não encontrado. Compile o backend."
    try:
        subprocess.run([backend_exe, arquivo_poly], check=True)
    except subprocess.CalledProcessError:
        return "Erro na execução do backend."
    with open("saida.txt", "r") as f:
        return f.read()

def desenhar_interface():
    global origem_selecionada, destino_selecionada, caminho, info_text, scroll_offset, tamanho_ponto
    pygame.init()
    tela = pygame.display.set_mode((LARGURA, ALTURA))
    pygame.display.set_caption("Sistema de Navegação Primitivo - INF/UFG")
    fonte = pygame.font.SysFont(None, 19)

    botoes = [
        ("Carregar .poly", 20),
        ("Adicionar ponto randômico", 60),
        ("Traçar menor caminho", 100),
        ("Resetar seleção", 140),
        ("Salvar imagem do grafo", 180),
        ("+ Tamanho ponto", 220),
        ("- Tamanho ponto", 260)
    ]

    rodando = True
    while rodando:
        tela.fill(FUNDO)
        pygame.draw.rect(tela, COR_MENU, (0, 0, LARGURA_MENU, ALTURA))

        for txt, y in botoes:
            pygame.draw.rect(tela, (200, 200, 200), (20, y, 280, 30))
            tela.blit(fonte.render(txt, True, (0, 0, 0)), (30, y + 5))

        for o, d, dir_a in arestas:
            x1, y1 = transformar(*vertices[o][1:])
            x2, y2 = transformar(*vertices[d][1:])
            cor = COR_ARESTA_UNICA if dir_a == 1 else COR_ARESTA
            pygame.draw.line(tela, cor, (x1, y1), (x2, y2), 2)

        for i, (_, x, y) in enumerate(vertices):
            sx, sy = transformar(x, y)
            cor = COR_VERTICE
            if i == origem_selecionada:
                cor = COR_ORIGEM
            elif i == destino_selecionada:
                cor = COR_DESTINO
            pygame.draw.circle(tela, cor, (sx, sy), tamanho_ponto)
            tela.blit(fonte.render(str(i), True, (0, 0, 0)), (sx - 10, sy - 20))

        # Caixa de texto
        caixa_rect = pygame.Rect(20, 310, 280, ALTURA_CAIXA_TEXTO)
        pygame.draw.rect(tela, (230, 230, 230), caixa_rect)

        linhas = info_text.split('\n')
        altura_total_texto = len(linhas) * 18
        max_scroll = max(0, altura_total_texto - ALTURA_CAIXA_TEXTO)

        if altura_total_texto > ALTURA_CAIXA_TEXTO:
            scroll_bar_height = max(20, int(ALTURA_CAIXA_TEXTO * ALTURA_CAIXA_TEXTO / altura_total_texto))
            scroll_bar_y = int(scroll_offset / max_scroll * (ALTURA_CAIXA_TEXTO - scroll_bar_height))
            pygame.draw.rect(tela, COR_SCROLL_BG, (caixa_rect.right - 10, caixa_rect.top, 10, ALTURA_CAIXA_TEXTO))
            pygame.draw.rect(tela, COR_SCROLL, (caixa_rect.right - 10, caixa_rect.top + scroll_bar_y, 10, scroll_bar_height))

        y_text = 320 - scroll_offset
        for linha in linhas:
            if 320 <= y_text <= 310 + ALTURA_CAIXA_TEXTO - 18:
                tela.blit(fonte.render(linha, True, (0, 0, 0)), (25, y_text))
            y_text += 18

        for ev in pygame.event.get():
            if ev.type == pygame.QUIT:
                rodando = False
            elif ev.type == pygame.MOUSEBUTTONDOWN:
                if ev.button == 4:
                    scroll_offset = max(0, scroll_offset - 20)
                elif ev.button == 5:
                    scroll_offset = min(max_scroll, scroll_offset + 20)
                x, y = ev.pos
                if x < LARGURA_MENU:
                    for idx, (_, by) in enumerate(botoes):
                        if by <= y <= by + 30:
                            if idx == 0:
                                root = tk.Tk()
                                root.withdraw()
                                file = filedialog.askopenfilename(filetypes=[("Poly Files", "*.poly")])
                                if file:
                                    ler_poly(file)
                                    origem_selecionada = None
                                    destino_selecionada = None
                                    caminho = []
                                    info_text = f"Arquivo carregado: {os.path.basename(file)}"
                                    scroll_offset = 0
                            elif idx == 1:
                                if vertices:
                                    import random
                                    idv = len(vertices)
                                    x_rand = random.uniform(0, 900)
                                    y_rand = random.uniform(0, 600)
                                    vertices.append((idv, x_rand, y_rand))
                                    atualizar_matriz()
                                    info_text = "Ponto aleatório adicionado."
                                    scroll_offset = 0
                            elif idx == 2:
                                if origem_selecionada is not None and destino_selecionada is not None:
                                    info_text = "Executando algoritmo no back-end..."
                                    pygame.display.flip()
                                    resultado = rodar_dijkstra_backend(origem_selecionada, destino_selecionada)
                                    info_text = resultado
                                    scroll_offset = 0
                                else:
                                    info_text = "Selecione origem e destino antes."
                            elif idx == 3:
                                origem_selecionada = None
                                destino_selecionada = None
                                caminho = []
                                info_text = "Seleção reiniciada."
                                scroll_offset = 0
                            elif idx == 4:
                                salvar_grafo_imagem(tela)
                                info_text = "Imagem salva como grafo_salvo.png."
                                scroll_offset = 0
                            elif idx == 5:
                                tamanho_ponto += 2
                            elif idx == 6:
                                tamanho_ponto = max(2, tamanho_ponto - 2)
                else:
                    v = clique(ev.pos)
                    if v is not None:
                        if origem_selecionada is None:
                            origem_selecionada = v
                            info_text = f"Origem: {v}"
                        elif destino_selecionada is None:
                            destino_selecionada = v
                            info_text = f"Destino: {v}"
                        scroll_offset = 0

        pygame.display.flip()
    pygame.quit()

if __name__ == "__main__":
    backend_exe = "backend.exe" if os.name == "nt" else "./backend"
    if not os.path.exists(backend_exe):
        print("Compilando o back-end...")
        try:
            subprocess.run(["gcc", "backend.c", "-o", backend_exe], check=True)
            print("Compilação concluída.")
        except Exception as e:
            print("Erro ao compilar o backend:", e)
            exit(1)

    desenhar_interface()
