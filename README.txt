🗺️ SISTEMA DE NAVEGAÇÃO PRIMITIVO
Instituto de Informártica/UFG – AED2 – 2025/1
Prof. André Luiz Moura
Autores: Ana Luisa Pereira dos Santos, Isadora Yasmim da Silva, Lucas Costa de Alvarenga e Verônica Ribeiro Oliveira Palmeira

📝 DESCRIÇÃO DO PROJETO
Este projeto implementa um Sistema de Navegação Primitivo com interface gráfica.
Dado um mapa em formato .poly, o usuário pode selecionar dois pontos (origem e destino) e calcular o menor caminho entre eles utilizando o algoritmo de Dijkstra, implementado em C com Fila de Prioridade (Heap Mínima).

⚙️ TECNOLOGIAS UTILIZADAS
Linguagem C: back-end (algoritmo de Dijkstra e leitura do grafo)
Python (Pygame e Tkinter): front-end gráfico
Bibliotecas auxiliares:
Pillow (PIL)
pywin32 (win32clipboard) – para cópia de imagem no Windows
📋 REQUISITOS PARA EXECUÇÃO
Python 3 instalado

Instalar as dependências do Python com o comando:

pip install pygame pillow pywin32
Compilar o back-end (caso não esteja usando o backend.exe pronto):

No Windows:
gcc backend.c -o backend.exe -lm
👾 COMO EXECUTAR O SISTEMA
Abra o terminal na pasta do projeto

Execute o front-end com:

python navegacao_primitiva_pygame.py
Na interface:

Clique em "Carregar .poly" para carregar o mapa
Clique em dois vértices do grafo para definir origem e destino
Clique em "Traçar menor caminho" para executar o algoritmo
O resultado será exibido na interface e também salvo no arquivo saida.txt (rota, custo, tempo e nós explorados).

📂 ARQUIVOS DO PROJETO
backend.c : Código do algoritmo de Dijkstra em C
backend.exe : Versão compilada do back-end
navegacao_primitiva_pygame.py : Interface gráfica em Python
entrada.txt : Arquivo gerado pelo front-end com os vértices escolhidos
saida.txt : Arquivo de saída com resultado da execução do algoritmo
SetorGoiania2.poly: Arquivo de mapa com definição dos vértices e arestas
iniciar.bat : Script de inicialização rápida (opcional, Windows)
📁 Estrutura do Projeto
📁 Códigos-fontes
📄 ConverteMapaParaCoordCartesianas.c
📄 backend.c
📄 navegacao_primitiva_pygame.py
📁 SistemaNavegacao_InstaladorDoPrograma
📄 README.md
📄 SetorGoiania2.poly
📄 backend.exe
📄 entrada.txt
📄 grafo_salvo.png
📄 iniciar.bat
📄 navegacao_primitiva_pygame.py
📄 saida.txt
📄 Documentação.pdf
📄 README.txt
📄 SetorGoiania2.poly
🔎 OBSERVAÇÕES FINAIS
O projeto suporta grafos com vias de mão única e mão dupla (com distinção visual)
É possível adicionar vértices randômicos e ajustar visualmente os pontos
Compatível com Windows
Código modular, organizado e documentado para facilitar manutenção
DATA DE ENTREGA: 21/06/2025