# Projeto de Controle de LEDs com Interrupções - RP2040 (BitDogLab)

## Objetivo

Este projeto visa consolidar a compreensão dos conceitos relacionados ao uso de interrupções no microcontrolador RP2040 e explorar as funcionalidades da placa de desenvolvimento BitDogLab. Os principais objetivos são:

- Compreender o funcionamento e a aplicação de interrupções em microcontroladores.
- Identificar e corrigir o fenômeno do bouncing em botões por meio de debouncing via software.
- Manipular e controlar LEDs comuns e LEDs endereçáveis WS2812.
- Fixar o estudo do uso de resistores de pull-up internos em botões de acionamento.
- Desenvolver um projeto funcional que combine hardware e software.

## Descrição do Projeto

Neste projeto, utilizamos os seguintes componentes conectados à placa **BitDogLab**:

- **Matriz 5x5 de LEDs (endereçáveis) WS2812**, conectada à **GPIO 7**.
- **LED RGB**, com os pinos conectados às **GPIOs (11, 12 e 13)**.
- **Botão A**, conectado à **GPIO 5**.
- **Botão B**, conectado à **GPIO 6**.

## Funcionalidades

O sistema apresenta as seguintes funcionalidades:

1. O **LED vermelho** do LED RGB pisca continuamente **5 vezes por segundo**.
2. O **botão A** incrementa o número exibido na **matriz de LEDs** cada vez que for pressionado.
3. O **botão B** decrementa o número exibido na **matriz de LEDs** cada vez que for pressionado.
4. A **matriz de LEDs WS2812** é utilizada para criar efeitos visuais representando números de **0 a 9**.
   - Cada número é exibido na matriz em um formato fixo, como caracteres em estilo digital, com segmentos iluminados formando o número.

## Requisitos

Para o desenvolvimento deste projeto, foram seguidos os seguintes requisitos:

1. **Uso de interrupções**: Todas as funcionalidades relacionadas aos botões foram implementadas utilizando **rotinas de interrupção (IRQ)**.
2. **Debouncing**: O tratamento do bouncing dos botões foi implementado via **software**.
3. **Controle de LEDs**: O projeto inclui o uso de **LEDs comuns** e **LEDs WS2812**, demonstrando o domínio de diferentes tipos de controle.
4. **Organização do código**: O código está bem estruturado e comentado para facilitar o entendimento.

## Como Executar

### Pré-requisitos

- **Raspberry Pi Pico** com o **RP2040**.
- **Placa BitDogLab**.
- **VS Code** com o **Pico SDK** instalado.
- **Biblioteca WS2812** (para controlar os LEDs endereçáveis).

  ### Passos

1. Clone o repositório em sua máquina local:
   ```bash
   git clone https://github.com/Luizgvv/interrupcoes_atividade.git
2. Conecte a placa BitDogLab ao computador.
3. Abra o código no VS Code e compile o projeto.
4. Carregue o código na placa RP2040.
5. Observe as funcionalidades descritas no projeto em execução.
   
### Vídeo de Demonstração

[![Demonstração na Placa](https://img.youtube.com/vi/5SkjIg2ZeIg/0.jpg)](https://youtu.be/5SkjIg2ZeIg) .

Um vídeo de demonstração com a explicação e exibição do funcionamento do projeto foi produzido. Clique na imagem acima ou no link abaixo para assistir:

🔗 [Assistir no YouTube](https://youtu.be/5SkjIg2ZeIg)  .

