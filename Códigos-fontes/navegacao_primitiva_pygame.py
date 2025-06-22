# Sistema de Navegação Primitivo - INF/UFG 2025-1
# Front-end com scroll dinâmico no resultado
# Autoras: Ana Luisa, Isadora, Lucas e Verônica

import pygame
import math
import tkinter as tk
from tkinter import filedialog
import subprocess
import os
import io
from PIL import Image
import platform

# Import para copiar imagem para clipboard no Windows
if platform.system() == "Windows":
    import win32clipboard
    from win32con import CF_DIB

# ----------------------------
# CONSTANTES E CONFIGURAÇÕES
# ----------------------------

LARGURA, ALTURA = 1360, 685
LARGURA_MENU = 320
ALTURA_CAIXA_TEXTO = 290

COR_VERTICE = (0, 102, 204)
COR_ARESTA = (0, 0, 0)
COR_ARESTA_UNICA = (255, 0, 0)
COR_ORIGEM = (0, 255, 0)
COR_DESTINO = (255, 0, 0)
COR_CAMINHO = (255, 255, 0)
FUNDO = (255, 255, 255)
COR_MENU = (245, 245, 245)
COR_SCROLL = (180, 180, 180)
COR_SCROLL_BG = (200, 200, 200)

RAIO_VERTICE = 10

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

# Controle de exibição para requisito RF02
mostrar_numeros = False
mostrar_pesos = False

# ----------------------------
# FUNÇÕES DE MANIPULAÇÃO DO GRAFO
# ----------------------------

def ler_poly(nome_arquivo):
    """
    Lê um arquivo .poly e carrega vértices e arestas.
    Atualiza variáveis globais e matriz de adjacência.
    """
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

            # Leitura dos vértices
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

            # Leitura das arestas
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
        return True
    except Exception as e:
        info_text = f"Erro ao ler arquivo: {str(e)}"
        return False


def atualizar_matriz():
    """
    Atualiza a matriz de adjacência baseada em vértices e arestas.
    """
    global matriz_adj
    n = len(vertices)
    matriz_adj = [[float('inf')] * n for _ in range(n)]
    for o, d, dir_a in arestas:
        dist = math.dist(vertices[o][1:], vertices[d][1:])
        matriz_adj[o][d] = dist
        if dir_a == 0:  # Aresta bidirecional
            matriz_adj[d][o] = dist


def transformar(x, y):
    """
    Transforma coordenadas do grafo para coordenadas da tela.
    """
    escala = 0.46
    return int(x * escala + LARGURA_MENU + 300), int(y * escala + 30)


def clique(pos):
    """
    Retorna índice do vértice clicado, se houver.
    """
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
    """
    Salva a imagem atual da tela em 'grafo_salvo.png'.
    """
    pygame.image.save(tela, "grafo_salvo.png")


def copiar_imagem_para_clipboard(tela):
    """
    Copia a imagem atual da tela para a área de transferência no Windows.
    """
    if platform.system() != "Windows":
        raise Exception("Copiar imagem para clipboard implementado apenas para Windows.")

    data = pygame.image.tostring(tela, 'RGB')
    size = tela.get_size()
    image = Image.frombytes('RGB', size, data)

    output = io.BytesIO()
    image.convert('RGB').save(output, 'BMP')
    data = output.getvalue()[14:]  # Remove cabeçalho BMP para formato DIB

    win32clipboard.OpenClipboard()
    win32clipboard.EmptyClipboard()
    win32clipboard.SetClipboardData(CF_DIB, data)
    win32clipboard.CloseClipboard()


# ----------------------------
# FUNÇÃO DE EXECUÇÃO DO BACKEND
# ----------------------------

def rodar_dijkstra_backend(origem, destino):
    """
    Executa o backend para cálculo do menor caminho e lê o resultado.
    """
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

def desenhar_interface():
    """
    Cria e controla a interface gráfica do sistema.
    """
    global origem_selecionada, destino_selecionada, caminho, info_text, scroll_offset, tamanho_ponto
    global mostrar_numeros, mostrar_pesos

    pygame.init()
    tela = pygame.display.set_mode((LARGURA, ALTURA))
    pygame.display.set_caption("Sistema de Navegação Primitivo - INF/UFG")
    fonte = pygame.font.SysFont("Segoe UI Symbol", 15)

    botoes = [
        ("Carregar .poly", 20),
        ("Adicionar ponto randômico", 60),
        ("Traçar menor caminho", 100),
        ("Resetar seleção", 140),
        ("Salvar imagem do grafo", 180),
        ("+ Tamanho ponto", 220),
        ("- Tamanho ponto", 260),
        # Botões para RF02
        (f"{'✔ ' if mostrar_numeros else '❌ '}Mostrar números", 300),
        (f"{'✔ ' if mostrar_pesos else '❌ '}Mostrar pesos", 340),
    ]

    def quebrar_linhas(texto, fonte, largura_max):
        """
        Quebra texto em várias linhas para caber na largura máxima dada.
        """
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

    rodando = True
    while rodando:
        tela.fill(FUNDO)
        pygame.draw.rect(tela, COR_MENU, (0, 0, LARGURA_MENU, ALTURA))

        # Atualiza os textos dos botões RF02 dinamicamente
        botoes[-2] = (f"{'✔ ' if mostrar_numeros else '❌ '}Mostrar números", 300)
        botoes[-1] = (f"{'✔ ' if mostrar_pesos else '❌ '}Mostrar pesos", 340)

        mouse_x, mouse_y = pygame.mouse.get_pos()
        # Atualizar o cursor do mouse com base em hover
        mouse_hover_sobre_botao = any(
            pygame.Rect(20, y, 280, 30).collidepoint(mouse_x, mouse_y) for _, y in botoes
        )

        if mouse_hover_sobre_botao:
            pygame.mouse.set_system_cursor(pygame.SYSTEM_CURSOR_HAND)
        else:
            pygame.mouse.set_system_cursor(pygame.SYSTEM_CURSOR_ARROW)

        # Desenha os botões com estilo
        mouse_x, mouse_y = pygame.mouse.get_pos()
        for txt, y in botoes:
            botao_rect = pygame.Rect(20, y, 280, 30)
            
            # Verifica se o mouse está em cima do botão
            hover = botao_rect.collidepoint(mouse_x, mouse_y)
            
            # Define cores
            cor_base = (230, 230, 230)
            cor_hover = (170, 200, 230)
            cor_borda = (100, 100, 100)

            # Desenha botão com hover
            pygame.draw.rect(tela, cor_hover if hover else cor_base, botao_rect, border_radius=8)
            
            # Desenha borda
            pygame.draw.rect(tela, cor_borda, botao_rect, 2, border_radius=8)

            # Renderiza texto centralizado
            texto_render = fonte.render(txt, True, (0, 0, 0))
            texto_rect = texto_render.get_rect(center=botao_rect.center)
            tela.blit(texto_render, texto_rect)


        # Desenha as arestas do grafo
        for o, d, dir_a in arestas:
            x1, y1 = transformar(*vertices[o][1:])
            x2, y2 = transformar(*vertices[d][1:])
            cor = COR_ARESTA_UNICA if dir_a == 1 else COR_ARESTA
            pygame.draw.line(tela, cor, (x1, y1), (x2, y2), 2)

            # Exibe peso da aresta no meio se habilitado
            if mostrar_pesos:
                dist = matriz_adj[o][d]
                mx, my = (x1 + x2) // 2, (y1 + y2) // 2
                texto_peso = fonte.render(f"{dist:.2f}", True, (0, 0, 0))
                texto_rect = texto_peso.get_rect(center=(mx, my))
                tela.blit(texto_peso, texto_rect)

        # Desenha caminho calculado em destaque
        if caminho:
            for i in range(len(caminho) - 1):
                x1, y1 = transformar(*vertices[caminho[i]][1:])
                x2, y2 = transformar(*vertices[caminho[i + 1]][1:])
                pygame.draw.line(tela, COR_CAMINHO, (x1, y1), (x2, y2), 4)

        # Desenha vértices, destacando origem e destino
        for i, (_, x, y) in enumerate(vertices):
            sx, sy = transformar(x, y)
            cor = COR_VERTICE
            if i == origem_selecionada:
                cor = COR_ORIGEM
            elif i == destino_selecionada:
                cor = COR_DESTINO
            pygame.draw.circle(tela, cor, (sx, sy), tamanho_ponto)

            # Exibe número do vértice se habilitado
            if mostrar_numeros:
                tela.blit(fonte.render(str(i), True, (0, 0, 0)), (sx - 10, sy - 20))

        # Caixa de texto com sombra, borda e cantos arredondados
        caixa_rect = pygame.Rect(20, 380, 280, ALTURA_CAIXA_TEXTO)
        sombra_rect = caixa_rect.move(2, 2)

        # Fundo
        pygame.draw.rect(tela, (230, 230, 230), caixa_rect, border_radius=8)

        # Borda
        pygame.draw.rect(tela, (120, 120, 120), caixa_rect, 2, border_radius=8)


        linhas_brutas = info_text.split('\n')
        linhas = []
        largura_caixa_texto = 280 - 20  # padding para texto dentro da caixa

        for linha_bruta in linhas_brutas:
            linhas.extend(quebrar_linhas(linha_bruta, fonte, largura_caixa_texto))

        altura_total_texto = len(linhas) * 20
        max_scroll = max(0, altura_total_texto - ALTURA_CAIXA_TEXTO)
        scroll_offset = max(0, min(scroll_offset, max_scroll))

        # Define clipping para texto ficar dentro da caixa
        tela.set_clip(caixa_rect)

        y_texto = 390 - scroll_offset
        for linha in linhas:
            texto_render = fonte.render(linha, True, (0, 0, 0))
            tela.blit(texto_render, (30, y_texto))
            y_texto += 20

        tela.set_clip(None)  # Remove clipping

        # Barra de scroll vertical (com trilha menor)
        if max_scroll > 0:
            largura_scroll = 8
            padding_direita = 4
            scroll_x = caixa_rect.right - largura_scroll - padding_direita

            # NOVO: trilha menor
            trilha_altura = int(ALTURA_CAIXA_TEXTO * 0.95)  # 70% da altura da caixa
            trilha_y = caixa_rect.top + (ALTURA_CAIXA_TEXTO - trilha_altura) // 2  # centraliza trilha

            # Calcula altura do thumb
            scroll_altura = max(30, trilha_altura * (ALTURA_CAIXA_TEXTO / altura_total_texto))
            scroll_pos = (trilha_altura - scroll_altura) * (scroll_offset / max_scroll)

            pygame.draw.rect(tela, COR_SCROLL_BG, (scroll_x, trilha_y, largura_scroll, trilha_altura), border_radius=4)
            pygame.draw.rect(tela, COR_SCROLL, (scroll_x, trilha_y + scroll_pos, largura_scroll, scroll_altura), border_radius=4)



        pygame.display.flip()

        # Tratamento de eventos
        for evento in pygame.event.get():
            if evento.type == pygame.QUIT:
                rodando = False

            elif evento.type == pygame.MOUSEBUTTONDOWN:
                mx, my = evento.pos
                if evento.button == 4:  # scroll up
                    scroll_offset -= 20
                elif evento.button == 5:  # scroll down
                    scroll_offset += 20
                else:
                    # Verifica clique nos botões
                    for idx, (_, ybot) in enumerate(botoes):
                        if 20 <= mx <= 300 and ybot <= my <= ybot + 30:
                            if idx == 0:
                                # Abrir diálogo para carregar arquivo .poly
                                root = tk.Tk()
                                root.withdraw()
                                caminho_arquivo = filedialog.askopenfilename(filetypes=[("Arquivos Poly", "*.poly")])
                                if caminho_arquivo:
                                    ler_poly(caminho_arquivo)
                                    scroll_offset = 0

                            elif idx == 1:
                                # Adicionar ponto randômico
                                import random
                                n = len(vertices)
                                x = random.uniform(0, 1000)
                                y = random.uniform(0, 1000)
                                vertices.append((n, x, y))
                                atualizar_matriz()
                                info_text = "Ponto randômico adicionado."
                                scroll_offset = 0

                            elif idx == 2:
                                # Traçar menor caminho
                                if origem_selecionada is not None and destino_selecionada is not None:
                                    info_text = rodar_dijkstra_backend(origem_selecionada, destino_selecionada)
                                else:
                                    info_text = "Selecione origem e destino clicando nos pontos."
                                scroll_offset = 0

                            elif idx == 3:
                                # Resetar seleção
                                origem_selecionada = None
                                destino_selecionada = None
                                caminho.clear()
                                info_text = "Seleção resetada."
                                scroll_offset = 0

                            elif idx == 4:
                                # Salvar e copiar imagem
                                salvar_grafo_imagem(tela)
                                try:
                                    copiar_imagem_para_clipboard(tela)
                                    info_text = "Imagem salva como 'grafo_salvo.png' e copiada para área de transferência."
                                except Exception as e:
                                    info_text = f"Imagem salva, mas erro ao copiar para área de transferência: {e}"
                                scroll_offset = 0

                            elif idx == 5:
                                # Aumentar tamanho do ponto
                                tamanho_ponto += 1
                                info_text = f"Tamanho do ponto aumentado para {tamanho_ponto}."
                                scroll_offset = 0

                            elif idx == 6:
                                # Diminuir tamanho do ponto
                                if tamanho_ponto > 1:
                                    tamanho_ponto -= 1
                                    info_text = f"Tamanho do ponto diminuído para {tamanho_ponto}."
                                else:
                                    info_text = "Tamanho mínimo do ponto atingido."
                                scroll_offset = 0

                            elif idx == 7:
                                # Alternar mostrar números
                                mostrar_numeros = not mostrar_numeros
                                info_text = f"Números dos vértices {'ativados' if mostrar_numeros else 'desativados'}."
                                scroll_offset = 0

                            elif idx == 8:
                                # Alternar mostrar pesos
                                mostrar_pesos = not mostrar_pesos
                                info_text = f"Pesos das arestas {'ativados' if mostrar_pesos else 'desativados'}."
                                scroll_offset = 0

                    # Selecionar vértice clicado
                    v = clique(evento.pos)
                    if v is not None:
                        if origem_selecionada is None:
                            origem_selecionada = v
                            info_text = f"Origem selecionada: {v}"
                        elif destino_selecionada is None and v != origem_selecionada:
                            destino_selecionada = v
                            info_text = f"Destino selecionado: {v}"
                        else:
                            if v == origem_selecionada:
                                origem_selecionada = None
                                info_text = "Origem desselecionada."
                            elif v == destino_selecionada:
                                destino_selecionada = None
                                info_text = "Destino desselecionado."
                            else:
                                info_text = "Seleção completa. Resetar para escolher novamente."
                        scroll_offset = 0

    pygame.quit()


if __name__ == "__main__":
    desenhar_interface()
