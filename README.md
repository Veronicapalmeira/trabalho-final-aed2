### 🗺️ SISTEMA DE NAVEGAÇÃO PRIMITIVO

**Instituto de Informártica/UFG – AED2 – 2025/1**  
**Prof. André Luiz Moura**  
**Autores**: Ana Luísa Pereira dos Santos, Isadora Yasmim da Silva, Lucas Costa de Alvarenga e Verônica Ribeiro Oliveira Palmeira

---

### 📝 DESCRIÇÃO DO PROJETO

Este projeto implementa um **Sistema de Navegação Primitivo com interface gráfica**.  
Dado um mapa em formato `.poly`, o usuário pode selecionar dois pontos (**origem** e **destino**) e calcular o menor caminho entre eles utilizando o **algoritmo de Dijkstra**, implementado em **C com Fila de Prioridade (Heap Mínima)**.


---

### ⚙️ TECNOLOGIAS UTILIZADAS

- **Linguagem C**: back-end (algoritmo de Dijkstra e leitura do grafo)
- **Python (Pygame e Tkinter)**: front-end gráfico
- **Bibliotecas auxiliares**:
  - Pillow (PIL)
  - [pywin32 (win32clipboard)](https://github.com/mhammond/pywin32) – para cópia de imagem no Windows

---

### 📋 REQUISITOS PARA EXECUÇÃO

1.  [Python 3](https://www.python.org/downloads/) instalado 
2. Instalar as dependências do Python com o comando:
   ```
   pip install pygame pillow pywin32
   ```

3. Compilar o back-end (caso não esteja usando o `backend.exe` pronto):
   - No Windows:
     ```
     gcc backend.c -o backend.exe -lm
     ```
### 👾 COMO EXECUTAR O SISTEMA

1. Abra o terminal na pasta do projeto  
2. Execute o front-end com:
   ```
   python navegacao_primitiva_pygame.py
   ```
3. Na interface:
   - Clique em **"Carregar .poly"** para carregar o mapa
   - Clique em dois vértices do grafo para definir **origem** e **destino**
   - Clique em **"Traçar menor caminho"** para executar o algoritmo

4. O resultado será exibido na interface e também salvo no arquivo `saida.txt` (rota, custo, tempo e nós explorados).

--------------------------------------------
### 📂 ARQUIVOS DO PROJETO
- `Anicums.poly`: Arquivo de mapa com definição dos vértices e arestas
- `ConverteMapaParaCoordCartesianas.c `: Código em C para converter dados do mapa em coordenadas cartesianas utilizáveis no sistema
- `backend.c`         : Código do algoritmo de Dijkstra em C
- `navegacao_primitiva_pygame.py` : Interface gráfica em Python  
- `backend.exe`       : Versão compilada do back-end  
- `iniciar.bat`       : Script de inicialização rápida (opcional, Windows)

<h2>📁 Estrutura do Projeto</h2>

<ul>
  <li>📁 <strong>Arquivos.poly</strong>
    <ul>
      <li>📄Anicums.poly </li>
      <li>📄 SetorGoiania2.poly</li>
      <li>📄 SetorPedro.poly</li>
      <li>📄 flores.poly</li>
    </ul>
  <li>📁 <strong>Códigos-fontes</strong>
    <ul>
      <li>📄 ConverteMapaParaCoordCartesianas.c </li>
      <li>📄 backend.c</li>
      <li>📄 navegacao_primitiva_pygame.py</li>
    </ul>
  </li>
  <li>📁 <strong>SistemaNavegacao_InstaladorDoPrograma</strong>
    <ul>
      <li>📄 backend.exe</li>
      <li>📄 iniciar.bat</li>
      <li>📄 navegacao_primitiva_pygame.py</li>
    </ul>
  </li>
  <li>📄 Documentação.pdf</li>
  <li>📄 README.txt</li>
</ul>

--------------------------------------------
### 🔎 OBSERVAÇÕES FINAIS

- O projeto suporta grafos com vias de mão única e mão dupla (com distinção visual)
- É possível adicionar vértices randômicos e ajustar visualmente os pontos
- Compatível com Windows 
- Código modular, organizado e documentado para facilitar manutenção

--------------------------------------------
### DATA DE ENTREGA: 22/06/2025
