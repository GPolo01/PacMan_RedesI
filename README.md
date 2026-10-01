# PacMan_RedesI

Projeto prático que implementa uma versão remota cliente-servidor do PacMan "no escuro". O projeto é desenvolvido de forma que o cliente e o servidor operem em computadores separados, conectados diretamente por meio de um cabo de rede.

---

## Informações Gerais

* **Modalidade:** Cliente-servidor remoto.
* **Equipe:** Em duplas (desconto de 20% para trabalhos individuais). O projeto deve obrigatoriamente ser apresentado pelos dois membros da equipe.
* **Infraestrutura:** O Cliente deve rodar em um computador e o Servidor em outro, conectados diretamente via cabo de rede.
* **Linguagens Permitidas:** C ou C++ (Obrigatório).
* **Comunicação:** Via RAWSocket (necessário privilégio de `root` nas máquinas), respeitando o protocolo de comunicação definido em sala.
* **Avaliação:** Valor total de 40,0 pontos (pontuações bônus não fazem a nota final ultrapassar o limite de 40 pontos na média).

---

## Entrega e Apresentação

1. **Apresentação Física:** Entrega de um relatório impresso (1 página) descrevendo as escolhas de desenvolvimento arquiteturais adotadas pela equipe.
2. **Entrega Digital (UFPR Virtual):**
   * Código-fonte e arquivo executável compactados em um arquivo `.tgz`, nomeado com os números de GRR da dupla.
   * Relatório em formato `.pdf`.

---

## Detalhes de Implementação da Rede

* **Timeout:** Obrigatório.
* **Controle de Fluxo:** Para-e-espera (Stop-and-wait).
* **Bônus (10%):** Implementação da transmissão dos arquivos das pastilhas utilizando janela deslizante de tamanho 5.

---

## O Jogo

A partida ocorre em um labirinto carregado na memória sob o formato de uma matriz.

### Configuração do Labirinto (40x40)
* Deve ser lido a partir de um arquivo `.csv` (valores separados por `;`) durante a inicialização do Servidor.
* Caso o arquivo não seja fornecido, o programa carrega um labirinto padrão contendo paredes que formam a escrita "UFPR", sorteando aleatoriamente as posições iniciais do PacMan, dos fantasmas e das pastilhas.

### Representação dos Elementos (Mapa)

| Caractere | Elemento Correspondente | Arquivo Vinculado (Se aplicável) |
| :---: | :--- | :--- |
| **P** | PacMan | - |
| **X** | Parede | - |
| **0** | Espaço Vazio | - |
| **R** | Fantasma Vermelho | - |
| **B** | Fantasma Azul | - |
| **G** | Fantasma Verde | - |
| **Y** | Fantasma Amarelo | - |
| **1** e **2** | Pastilha Dourada (Texto) | `1.txt`, `2.txt` |
| **3** e **4** | Pastilha Dourada (Imagem) | `3.jpg`, `4.jpg` |
| **5** e **6** | Pastilha Dourada (Vídeo) | `5.mp4`, `6.mp4` |

### Mecânica e Visão "No Escuro"
* **Rodadas:** O jogo funciona por turnos. A cada rodada, o jogador e os fantasmas realizam um único movimento.
* **Objetivo:** O PacMan deve coletar as 6 pastilhas douradas para concluir a fase. A cada pastilha coletada, um arquivo correspondente é transferido, revelando o prêmio.
* **Encontros com Fantasmas:** Se o PacMan colidir com um fantasma, uma transferência de arquivo (definida pela dupla) deve ocorrer exibindo o encontro.
* **Campo de Visão:** 
  * O jogo inicia "no escuro". O PacMan enxerga apenas um raio de 1 casa ao seu redor.
  * A cada 5 movimentos (rodadas), o raio de visualização do PacMan se expande em 1 casa.

---

## Arquitetura de Software

### Cliente
Computador responsável exclusivamente pela interface com o usuário; **não** guarda o estado geral da partida.

* **Função Principal:** Coletar o movimento escolhido pelo usuário (cima, baixo, esquerda, direita).
* **Comunicação:** 
  * Cria uma mensagem de tamanho fixo contendo a ação de movimento e a envia ao Servidor.
  * Pausa sua execução e aguarda a resposta do servidor.
* **Interface:** 
  * Recebe e exibe o novo mapa atualizado em tela a cada rodada.
  * Caso encontre uma pastilha ou fantasma, o Cliente recebe o arquivo do prêmio/encontro do Servidor e o exibe ao usuário.

### Servidor
Computador responsável pela lógica central. Possui a visão global do tabuleiro de forma onisciente.

* **Inicialização:** Cria ou carrega o mapa de forma integral e conecta-se ao Cliente para enviar a visualização inicial "escura".
* **Fluxo de Rodada:**
  1. Aguarda receber a mensagem de movimentação do PacMan.
  2. Calcula a nova posição do PacMan.
  3. Calcula os movimentos de todos os fantasmas.
  4. Gera o recorte atualizado de visualização do jogador e o envia ao cliente (o envio pode exigir múltiplas mensagens, visto que o tamanho do campo de visão é variável e cresce ao decorrer do tempo).
  5. Em caso de colisão (pastilha ou fantasma), realiza a transmissão do respectivo arquivo (que pode variar de centenas de bytes a megabytes).
* **Inteligência dos Fantasmas:**
  * **Verde:** Anda em espiral no sentido horário (alternando direita/esquerda).
  * **Azul:** Anda em espiral no sentido anti-horário (regra da mão direita).
  * **Vermelho:** Segue a regra da mão esquerda.
  * **Amarelo:** Segue a regra da mão direita (aleatório).
* **Logs do Sistema:** O Servidor deve exibir e manter um log detalhado de todas as mensagens recebidas e enviadas, apresentado em uma janela de terminal separada.
