# Sistema de Navegação Primitivo - INF/UFG 2025-1
# Front-end
# Autoras: Ana Luisa, Isadora, Lucas e Verônica

import os
import math
import platform
import subprocess
import io
import tkinter as tk
from tkinter import filedialog

import pygame
from PIL import Image

# Import para copiar imagem para clipboard no Windows
if platform.system() == "Windows":
    import win32clipboard
    from win32con import CF_DIB

# ----------------------------
# CONSTANTES E CONFIGURAÇÕES
# ----------------------------

LARGURA, ALTURA = 1360, 685
LARGURA_MENU = 320
ALTURA_CAIXA_TEXTO = 360

COR_VERTICE = (255, 105, 180)        # Rosa escuro (flat elegante)
COR_ARESTA = (136, 14, 79)          # Tom roxo escuro, discreto
COR_ARESTA_UNICA = (195, 65, 224)     # Roxo claro para destaque
COR_ORIGEM = (255, 255, 0)          # Amarelo para origem
COR_DESTINO = (38, 255, 0)           # Verde escuro para destino
COR_CAMINHO = (14, 21, 232)         # Azul visível sobre fundo
FUNDO = (252, 228, 236)              # Rosa claríssimo - fundo principal
COR_MENU = (248, 187, 208)           # Rosa opaco - menu lateral
COR_SCROLL = (240, 98, 146)          # Scroll bar rosa suave
COR_SCROLL_BG = (252, 228, 236)      # Igual ao fundo

# Botões e texto
COR_BOTAO = (244, 143, 177)         # Rosa flat
COR_BOTAO_HOVER = (236, 64, 122)    # Rosa forte
COR_BORDA_BOTAO = (173, 20, 87)     # Borda escura
COR_TEXTO = (62, 62, 62)            # Cinza escuro para textos
CAIXA_TEXTO_BG = (252, 228, 236)    # Rosa claro
CAIXA_TEXTO_BORDA = (173, 20, 87)   # Borda da caixa de texto

RAIO_VERTICE = 3

# ----------------------------
# VARIÁVEIS GLOBAIS
# ----------------------------

vertices = []           # Lista de tuplas (id, x, y) dos vértices
arestas = []            # Lista de tuplas (origem, destino, direção)
matriz_adj = []         # Matriz de adjacência com pesos (distâncias)
arquivo_poly = ""       # Nome do arquivo .poly carregado

origem_selecionada = None
destino_selecionada = None
caminho = []            # Lista de índices dos vértices no caminho calculado

info_text = "Carregue um arquivo .poly para começar."  # Texto informativo na interface
scroll_offset = 0       # Controle de scroll da caixa de texto
tamanho_ponto = RAIO_VERTICE  # Tamanho visual do ponto do vértice
escala = 1.0
offset_x = 0
offset_y = 0

mostrar_numeros = False
mostrar_pesos = False

# ----------------------------
# FUNÇÕES DE MANIPULAÇÃO DO GRAFO
# ----------------------------

def ler_poly(nome_arquivo):
    global vertices, arestas, matriz_adj, arquivo_poly, info_text
    global origem_selecionada, destino_selecionada, caminho
    try:
        arquivo_poly = nome_arquivo
        vertices.clear()
        arestas.clear()
        with open(nome_arquivo, 'r') as f:
            linha1 = f.readline().strip().split()
            if len(linha1) < 4:
                info_text = "Erro: Cabeçalho do arquivo .poly inválido."
                return False
            n_vertices, _, _, _ = map(int, linha1)

            for _ in range(n_vertices):
                linha_v = f.readline().strip().split()
                if len(linha_v) != 3:
                    info_text = "Erro: Formato incorreto de vértice no arquivo."
                    return False
                idv, x, y = linha_v
                vertices.append((int(idv), float(x), float(y)))

            linha2 = f.readline().strip().split()
            if len(linha2) < 2:
                info_text = "Erro: Cabeçalho das arestas inválido."
                return False
            n_arestas, _ = map(int, linha2)

            for _ in range(n_arestas):
                linha_a = f.readline().strip().split()
                if len(linha_a) != 4:
                    info_text = "Erro: Formato incorreto de aresta no arquivo."
                    return False
                _, o, d, dir_a = linha_a
                arestas.append((int(o), int(d), int(dir_a)))

        atualizar_matriz()
        origem_selecionada = None
        destino_selecionada = None
        caminho.clear()
        info_text = f"Arquivo carregado: {os.path.basename(nome_arquivo)}"
        ajustar_escala()
        return True
    except Exception as e:
        info_text = f"Erro ao ler arquivo: {str(e)}"
        return False

def atualizar_matriz():
    global matriz_adj
    n = len(vertices)
    matriz_adj = [[float('inf')] * n for _ in range(n)]
    for o, d, dir_a in arestas:
        dist = math.dist(vertices[o][1:], vertices[d][1:])
        matriz_adj[o][d] = dist
        if dir_a == 0:
            matriz_adj[d][o] = dist

def ajustar_escala():
    global escala, offset_x, offset_y
    if not vertices:
        return

    xs = [x for _, x, _ in vertices]
    ys = [y for _, _, y in vertices]
    min_x, max_x = min(xs), max(xs)
    min_y, max_y = min(ys), max(ys)

    largura_grafo = max_x - min_x
    altura_grafo = max_y - min_y

    largura_disp = LARGURA - LARGURA_MENU - 40
    altura_disp = ALTURA - 40

    if largura_grafo == 0 or altura_grafo == 0:
        escala = 1.0
    else:
        escala = min(largura_disp / largura_grafo, altura_disp / altura_grafo)

    centro_grafo_x = (min_x + max_x) / 2
    centro_grafo_y = (min_y + max_y) / 2
    centro_tela_x = LARGURA_MENU + largura_disp / 2
    centro_tela_y = ALTURA / 2

    offset_x = centro_tela_x - centro_grafo_x * escala
    offset_y = centro_tela_y - centro_grafo_y * escala

def transformar(x, y):
    return int(x * escala + offset_x), int(y * escala + offset_y)

def clique(pos):
    x, y = pos
    for i, (_, vx, vy) in enumerate(vertices):
        sx, sy = transformar(vx, vy)
        if (x - sx) ** 2 + (y - sy) ** 2 <= tamanho_ponto ** 2:
            return i
    return None

# ----------------------------
# FUNÇÕES DE SALVAR E COPIAR IMAGEM
# ----------------------------

def salvar_grafo_imagem(tela):
    pygame.image.save(tela, "grafo_salvo.png")

def copiar_imagem_para_clipboard(tela):
    if platform.system() != "Windows":
        raise Exception("Copiar imagem para clipboard implementado apenas para Windows.")

    data = pygame.image.tostring(tela, 'RGB')
    size = tela.get_size()
    image = Image.frombytes('RGB', size, data)

    output = io.BytesIO()
    image.convert('RGB').save(output, 'BMP')
    data = output.getvalue()[14:]

    win32clipboard.OpenClipboard()
    win32clipboard.EmptyClipboard()
    win32clipboard.SetClipboardData(CF_DIB, data)
    win32clipboard.CloseClipboard()

# ----------------------------
# FUNÇÃO DE EXECUÇÃO DO BACKEND
# ----------------------------

def rodar_dijkstra_backend(origem, destino):
    global caminho
    with open("entrada.txt", "w") as f:
        f.write(f"{origem}\n{destino}\n")

    backend_exe = "backend.exe" if os.name == "nt" else "./backend"
    if not os.path.exists(backend_exe):
        return "Erro: backend não encontrado. Compile o backend."

    try:
        subprocess.run([backend_exe, arquivo_poly], check=True)
    except subprocess.CalledProcessError:
        return "Erro na execução do backend."

    caminho.clear()
    resultado_texto = ""
    with open("saida.txt", "r") as f:
        linhas = f.readlines()
        for linha in linhas:
            resultado_texto += linha
            if linha.startswith("Caminho"):
                continue
            if "(" in linha and "x=" in linha and "y=" in linha:
                idx = int(linha.split()[0])
                caminho.append(idx)

    return resultado_texto

# ----------------------------
# FUNÇÃO PRINCIPAL DA INTERFACE
# ----------------------------

def quebrar_linhas(texto, fonte, largura_max):
    palavras = texto.split(' ')
    linhas = []
    linha_atual = ""
    for palavra in palavras:
        teste_linha = linha_atual + palavra + " "
        largura_texto, _ = fonte.size(teste_linha)
        if largura_texto <= largura_max:
            linha_atual = teste_linha
        else:
            linhas.append(linha_atual.rstrip())
            linha_atual = palavra + " "
    if linha_atual:
        linhas.append(linha_atual.rstrip())
    return linhas

# ----------------------------
# FUNÇÃO PARA DESENHAR LEGENDA NO CANTO INFERIOR DIREITO
# ----------------------------
def desenhar_legenda_caixa(tela, fonte):
    largura_caixa = 280
    altura_caixa = 130
    margem = 15

    x = LARGURA - largura_caixa - margem
    y = ALTURA - altura_caixa - margem

    pygame.draw.rect(tela, (248, 187, 208), (x, y, largura_caixa, altura_caixa), border_radius=10)
    pygame.draw.rect(tela, COR_BORDA_BOTAO, (x, y, largura_caixa, altura_caixa), 2, border_radius=10)

    espacamento_y = 25
    texto_x = x + 45
    texto_y = y + 7

    itens = [
        ((255, 255, 0), "Origem (ponto amarelo)"),
        ((38, 255, 0), "Destino (ponto verde)"),
        ((14, 21, 232), "Caminho (linha azul)"),
        ((136, 14, 79), "Mão dupla (linha roxa escura)"),
        ((195, 65, 224), "Mão única (linha roxa claro)"),
    ]

    for cor, texto in itens:
        if "linha" not in texto:
            pygame.draw.circle(tela, cor, (x + 20, texto_y + 10), 8)
        else:
            pygame.draw.line(tela, cor, (x + 10, texto_y + 10), (x + 30, texto_y + 10), 5)

        texto_render = fonte.render(texto, True, COR_TEXTO)
        tela.blit(texto_render, (texto_x, texto_y - 2))
        texto_y += espacamento_y


def desenhar_interface():
    global origem_selecionada, destino_selecionada, caminho, info_text, scroll_offset, tamanho_ponto
    global mostrar_numeros, mostrar_pesos, offset_x, offset_y, escala

    pygame.init()
    tela = pygame.display.set_mode((LARGURA, ALTURA))
    pygame.display.set_caption("Sistema de Navegação Primitivo - INF/UFG")

    fonte_botoes = pygame.font.SysFont("Segoe UI Symbol", 15)
    fonte_numeros = pygame.font.SysFont("Segoe UI Symbol", 10)
    fonte_pesos = pygame.font.SysFont("Segoe UI Symbol", 10)
    fonte_simbolos = pygame.font.SysFont("Segoe UI", 25)

    MARGEM_LATERAL = 20
    LARGURA_BOTAO = 280
    ALTURA_BOTAO = 30
    ESPACO_ENTRE_BOTOES = 15

    botoes_textos = [
        "Carregar .poly",
        "Traçar menor caminho",
        "Salvar imagem do grafo",
        "Alterar tamanho do ponto",
    ]

    botoes = []
    y_inicial = 20
    for i, texto in enumerate(botoes_textos):
        botoes.append((texto, y_inicial + i * (ALTURA_BOTAO + ESPACO_ENTRE_BOTOES)))

    # Posicionamento dos checkboxes (mostrar números e mostrar pesos) embaixo dos botões
    y_checkboxes = y_inicial + len(botoes_textos) * (ALTURA_BOTAO + ESPACO_ENTRE_BOTOES) + 10

    largura_checkbox_1 = int((LARGURA_MENU - 2 * MARGEM_LATERAL - 10) * 0.54)
    largura_checkbox_2 = (LARGURA_MENU - 2 * MARGEM_LATERAL - 10) - largura_checkbox_1
    altura_checkbox_botao = ALTURA_BOTAO
    espacamento_entre_checkboxes = 10
    largura_caixinha = 18

    rect_checkbox_1 = pygame.Rect(MARGEM_LATERAL, y_checkboxes, largura_checkbox_1, altura_checkbox_botao)
    rect_checkbox_2 = pygame.Rect(MARGEM_LATERAL + largura_checkbox_1 + espacamento_entre_checkboxes, y_checkboxes,
                                  largura_checkbox_2, altura_checkbox_botao)

    rodando = True
    while rodando:
        tela.fill(FUNDO)
        pygame.draw.rect(tela, COR_MENU, (0, 0, LARGURA_MENU, ALTURA))

        mouse_x, mouse_y = pygame.mouse.get_pos()

        # Cursor do mouse sobre botão ou checkbox
        mouse_hover_sobre_botao = any(
            pygame.Rect(MARGEM_LATERAL, y, LARGURA_BOTAO, ALTURA_BOTAO).collidepoint(mouse_x, mouse_y) for _, y in botoes
        )
        hover_checkbox_1 = rect_checkbox_1.collidepoint(mouse_x, mouse_y)
        hover_checkbox_2 = rect_checkbox_2.collidepoint(mouse_x, mouse_y)

        pygame.mouse.set_system_cursor(
            pygame.SYSTEM_CURSOR_HAND if (mouse_hover_sobre_botao or hover_checkbox_1 or hover_checkbox_2)
            else pygame.SYSTEM_CURSOR_ARROW)

        # Desenha botões normais
        for i, (txt, y) in enumerate(botoes):
            if i == 3:
                continue  # pula o "Alterar tamanho do ponto"
            botao_rect = pygame.Rect(MARGEM_LATERAL, y, LARGURA_BOTAO, ALTURA_BOTAO)
            hover = botao_rect.collidepoint(mouse_x, mouse_y)
            pygame.draw.rect(tela, COR_BOTAO_HOVER if hover else COR_BOTAO, botao_rect, border_radius=8)
            pygame.draw.rect(tela, COR_BORDA_BOTAO, botao_rect, 2, border_radius=8)
            texto_render = fonte_botoes.render(txt, True, COR_TEXTO)
            texto_rect = texto_render.get_rect(center=botao_rect.center)
            tela.blit(texto_render, texto_rect)

            # Botão especial: "Alterar tamanho do ponto" com "+" e "−"
            txt, y = botoes[3]
            botao_rect = pygame.Rect(MARGEM_LATERAL, y, LARGURA_BOTAO, ALTURA_BOTAO)
            pygame.draw.rect(tela, COR_BOTAO, botao_rect, border_radius=8)
            pygame.draw.rect(tela, COR_BORDA_BOTAO, botao_rect, 2, border_radius=8)

            # Texto à esquerda
            texto_alterar = fonte_botoes.render("Alterar tamanho do ponto", True, COR_TEXTO)
            tela.blit(texto_alterar, (botao_rect.x + 10, botao_rect.y + (ALTURA_BOTAO - texto_alterar.get_height()) // 2))

            # Mini botões "+" e "−"
            largura_mini = 25
            espaco = 8
            x_mais = botao_rect.right - largura_mini * 2 - espaco - 10
            x_menos = botao_rect.right - largura_mini - 10
            y_mini = botao_rect.y + (ALTURA_BOTAO - 20) // 2

            botao_mais_rect = pygame.Rect(x_mais, y_mini, largura_mini, 20)
            botao_menos_rect = pygame.Rect(x_menos, y_mini, largura_mini, 20)

            hover_mais = botao_mais_rect.collidepoint(mouse_x, mouse_y)
            hover_menos = botao_menos_rect.collidepoint(mouse_x, mouse_y)

            pygame.draw.rect(tela, COR_BOTAO_HOVER if hover_mais else FUNDO, botao_mais_rect, border_radius=3)
            pygame.draw.rect(tela, COR_BORDA_BOTAO, botao_mais_rect, 2, border_radius=3)
            pygame.draw.rect(tela, COR_BOTAO_HOVER if hover_menos else FUNDO, botao_menos_rect, border_radius=3)
            pygame.draw.rect(tela, COR_BORDA_BOTAO, botao_menos_rect, 2, border_radius=3)

            # Texto dos botões
            texto_mais = fonte_simbolos.render("+", True, COR_TEXTO)
            texto_menos = fonte_simbolos.render("−", True, COR_TEXTO)
            rect_mais = texto_mais.get_rect(center=botao_mais_rect.center)
            rect_menos = texto_menos.get_rect(center=botao_menos_rect.center)
            rect_mais.centery -= 3  # sobe 3 pixels
            rect_menos.centery -= 3
            tela.blit(texto_mais, rect_mais)
            tela.blit(texto_menos, rect_menos)

        # Desenha os checkboxes (caixa + texto) no local original
        # Checkbox 1: Mostrar números
        pygame.draw.rect(tela,
                         COR_BOTAO_HOVER if hover_checkbox_1 else COR_BOTAO,
                         rect_checkbox_1,
                         border_radius=4)
        pygame.draw.rect(tela, COR_BORDA_BOTAO, rect_checkbox_1, 2, border_radius=4)

        caixa_chk_1 = pygame.Rect(rect_checkbox_1.x + 4,
                                  rect_checkbox_1.y + (altura_checkbox_botao - largura_caixinha) // 2,
                                  largura_caixinha, largura_caixinha)
        pygame.draw.rect(tela, FUNDO, caixa_chk_1, border_radius=4)
        pygame.draw.rect(tela, COR_BORDA_BOTAO, caixa_chk_1, 2, border_radius=4)
        if mostrar_numeros:
            pygame.draw.line(tela, COR_BORDA_BOTAO, caixa_chk_1.topleft, caixa_chk_1.bottomright, 3)
            pygame.draw.line(tela, COR_BORDA_BOTAO, caixa_chk_1.topright, caixa_chk_1.bottomleft, 3)

        texto_num = fonte_botoes.render("Mostrar números", True, COR_TEXTO)
        tela.blit(texto_num, (caixa_chk_1.right + 3, rect_checkbox_1.y + (altura_checkbox_botao - texto_num.get_height()) // 2))

        # Checkbox 2: Mostrar pesos
        pygame.draw.rect(tela,
                         COR_BOTAO_HOVER if hover_checkbox_2 else COR_BOTAO,
                         rect_checkbox_2,
                         border_radius=4)
        pygame.draw.rect(tela, COR_BORDA_BOTAO, rect_checkbox_2, 2, border_radius=4)

        caixa_chk_2 = pygame.Rect(rect_checkbox_2.x + 4,
                                  rect_checkbox_2.y + (altura_checkbox_botao - largura_caixinha) // 2,
                                  largura_caixinha, largura_caixinha)
        pygame.draw.rect(tela, FUNDO, caixa_chk_2, border_radius=4)
        pygame.draw.rect(tela, COR_BORDA_BOTAO, caixa_chk_2, 2, border_radius=4)
        if mostrar_pesos:
            pygame.draw.line(tela, COR_BORDA_BOTAO, caixa_chk_2.topleft, caixa_chk_2.bottomright, 3)
            pygame.draw.line(tela, COR_BORDA_BOTAO, caixa_chk_2.topright, caixa_chk_2.bottomleft, 3)

        texto_pes = fonte_botoes.render("Mostrar pesos", True, COR_TEXTO)
        tela.blit(texto_pes, (caixa_chk_2.right + 3, rect_checkbox_2.y + (altura_checkbox_botao - texto_pes.get_height()) // 2))

        # Limita o desenho do grafo na área branca (não ultrapassa o menu)
        area_grafo_rect = pygame.Rect(LARGURA_MENU, 0, LARGURA - LARGURA_MENU, ALTURA)
        tela.set_clip(area_grafo_rect)

        # Desenha arestas
        for o, d, dir_a in arestas:
            x1, y1 = transformar(*vertices[o][1:])
            x2, y2 = transformar(*vertices[d][1:])
            cor = COR_ARESTA_UNICA if dir_a == 1 else COR_ARESTA
            pygame.draw.line(tela, cor, (x1, y1), (x2, y2), 2)
            if mostrar_pesos:
                dist = matriz_adj[o][d]
                mx, my = (x1 + x2) // 2, (y1 + y2) // 2
                texto_peso = fonte_pesos.render(f"{dist:.2f}", True, (0, 0, 0))
                texto_rect = texto_peso.get_rect(center=(mx, my))
                tela.blit(texto_peso, texto_rect)

        # Desenha caminho
        if caminho:
            for i in range(len(caminho) - 1):
                x1, y1 = transformar(*vertices[caminho[i]][1:])
                x2, y2 = transformar(*vertices[caminho[i + 1]][1:])
                pygame.draw.line(tela, COR_CAMINHO, (x1, y1), (x2, y2), 5)

        # Desenha vértices
        for i, (_, x, y) in enumerate(vertices):
            sx, sy = transformar(x, y)
            if i == origem_selecionada:
                 # Círculo maior atrás para destaque da origem
                pygame.draw.circle(tela, (178, 181, 9), (sx, sy), tamanho_ponto + 2)
                pygame.draw.circle(tela, COR_ORIGEM, (sx, sy), tamanho_ponto)
            elif i == destino_selecionada:
                # Círculo maior atrás para destaque do destino
                pygame.draw.circle(tela, (11, 176, 74), (sx, sy), tamanho_ponto + 2)
                pygame.draw.circle(tela, COR_DESTINO, (sx, sy), tamanho_ponto)
            else:
                pygame.draw.circle(tela, COR_VERTICE, (sx, sy), tamanho_ponto)
            if mostrar_numeros:
                texto_num = fonte_numeros.render(str(i), True, (0, 0, 0))
                tela.blit(texto_num, (sx - 10, sy - 20))

        # Libera clipping para UI
        tela.set_clip(None)

        # Desenha caixa de texto abaixo dos botões e checkboxes
        caixa_texto_y = y_checkboxes + altura_checkbox_botao + ESPACO_ENTRE_BOTOES
        caixa_rect = pygame.Rect(MARGEM_LATERAL, caixa_texto_y, LARGURA_BOTAO, ALTURA_CAIXA_TEXTO)
        pygame.draw.rect(tela, CAIXA_TEXTO_BG, caixa_rect, border_radius=8)
        pygame.draw.rect(tela, CAIXA_TEXTO_BORDA, caixa_rect, 2, border_radius=8)

        linhas_brutas = info_text.split('\n')
        linhas = []
        for linha_bruta in linhas_brutas:
            linhas.extend(quebrar_linhas(linha_bruta, fonte_botoes, LARGURA_BOTAO - 20))
        altura_total_texto = len(linhas) * 20
        max_scroll = max(0, altura_total_texto - caixa_rect.height)
        scroll_offset = max(0, min(scroll_offset, max_scroll))

        tela.set_clip(caixa_rect)
        y_texto = caixa_rect.y + 10 - scroll_offset
        for linha in linhas:
            texto_render = fonte_botoes.render(linha, True, COR_TEXTO)
            tela.blit(texto_render, (caixa_rect.x + 10, y_texto))
            y_texto += 20
        tela.set_clip(None)

        # Botão "Resetar seleção" DEBAIXO da caixa de texto
        botao_resetar_y = caixa_rect.bottom + ESPACO_ENTRE_BOTOES
        botao_resetar_rect = pygame.Rect(MARGEM_LATERAL, botao_resetar_y, LARGURA_BOTAO, ALTURA_BOTAO)
        hover_resetar = botao_resetar_rect.collidepoint(mouse_x, mouse_y)
        pygame.draw.rect(tela, COR_BOTAO_HOVER if hover_resetar else COR_BOTAO, botao_resetar_rect, border_radius=8)
        pygame.draw.rect(tela, COR_BORDA_BOTAO, botao_resetar_rect, 2, border_radius=8)
        texto_resetar = fonte_botoes.render("Resetar seleção", True, COR_TEXTO)
        texto_resetar_rect = texto_resetar.get_rect(center=botao_resetar_rect.center)
        tela.blit(texto_resetar, texto_resetar_rect)

        if vertices:
            desenhar_legenda_caixa(tela, fonte_botoes)

        pygame.display.flip()

        for evento in pygame.event.get():
            if evento.type == pygame.QUIT:
                rodando = False

            elif evento.type == pygame.MOUSEWHEEL:
                mx, my = pygame.mouse.get_pos()
                if caixa_rect.collidepoint(mx, my):
                    if evento.y > 0:
                        scroll_offset -= 20
                    elif evento.y < 0:
                        scroll_offset += 20
                else:
                    zoom_factor = 0.1
                    gx = (mx - offset_x) / escala
                    gy = (my - offset_y) / escala
                    if evento.y > 0:
                        escala *= 1 + zoom_factor
                    elif evento.y < 0:
                        escala /= 1 + zoom_factor
                    escala = max(0.1, min(escala, 10))
                    offset_x = mx - gx * escala
                    offset_y = my - gy * escala

            elif evento.type == pygame.MOUSEBUTTONDOWN:
                if evento.button == 1:
                    x, y = evento.pos
                    if botao_mais_rect.collidepoint(x, y):
                        tamanho_ponto = min(tamanho_ponto + 1, 15)
                    elif botao_menos_rect.collidepoint(x, y):
                        tamanho_ponto = max(tamanho_ponto - 1, 1)

                    # Verifica clique nos botões normais
                    for idx, (txt, by) in enumerate(botoes):
                        botao_rect = pygame.Rect(MARGEM_LATERAL, by, LARGURA_BOTAO, ALTURA_BOTAO)
                        if botao_rect.collidepoint(x, y):
                            if txt.startswith("Carregar .poly"):
                                root = tk.Tk()
                                root.withdraw()
                                caminho_arquivo = filedialog.askopenfilename(
                                    filetypes=[("Poly files", "*.poly")]
                                )
                                root.destroy()
                                if caminho_arquivo:
                                    ler_poly(caminho_arquivo)
                            elif txt.startswith("Traçar menor caminho"):
                                if origem_selecionada is None or destino_selecionada is None:
                                    info_text = "Selecione origem e destino clicando nos pontos."
                                else:
                                    info_text = rodar_dijkstra_backend(origem_selecionada, destino_selecionada)
                                    scroll_offset = 0
                            elif txt.startswith("Salvar imagem"):
                                salvar_grafo_imagem(tela)
                                try:
                                    copiar_imagem_para_clipboard(tela)
                                    info_text = "Imagem salva como 'grafo_salvo.png' e copiada para o clipboard."
                                except Exception as e:
                                    info_text = f"Imagem salva, mas falha ao copiar para clipboard: {str(e)}"

                            elif txt.startswith("+ Tamanho ponto"):
                                tamanho_ponto = min(tamanho_ponto + 1, 15)
                            elif txt.startswith("- Tamanho ponto"):
                                tamanho_ponto = max(tamanho_ponto - 1, 1)
                            break
                    else:
                        # Cliques nos checkboxes
                        if rect_checkbox_1.collidepoint(x, y):
                            mostrar_numeros = not mostrar_numeros
                        elif rect_checkbox_2.collidepoint(x, y):
                            mostrar_pesos = not mostrar_pesos
                        # Clique no botão "Resetar seleção"
                        elif botao_resetar_rect.collidepoint(x, y):
                            origem_selecionada = None
                            destino_selecionada = None
                            caminho.clear()
                            info_text = "Seleção resetada."
                        else:
                            # Clique na área do grafo
                            if x > LARGURA_MENU:
                                idx_vertice = clique((x, y))
                                if idx_vertice is not None:
                                    if origem_selecionada is None:
                                        origem_selecionada = idx_vertice
                                        info_text = f"Origem selecionada: {idx_vertice}"
                                    elif destino_selecionada is None:
                                        destino_selecionada = idx_vertice
                                        info_text = f"Destino selecionado: {idx_vertice}"
                                    else:
                                        origem_selecionada = idx_vertice
                                        destino_selecionada = None
                                        caminho.clear()
                                        info_text = f"Origem selecionada: {idx_vertice}. Selecione destino."

    pygame.quit()

# ----------------------------
# EXECUTA A INTERFACE
# ----------------------------

if __name__ == "__main__":
    desenhar_interface()
