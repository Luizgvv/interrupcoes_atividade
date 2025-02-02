Projeto de Controle de LEDs com Interrupções - RP2040 (BitDogLab)
Objetivo
Este projeto visa consolidar a compreensão dos conceitos relacionados ao uso de interrupções no microcontrolador RP2040 e explorar as funcionalidades da placa de desenvolvimento BitDogLab. Os principais objetivos são:

Compreender o funcionamento e a aplicação de interrupções em microcontroladores.
Identificar e corrigir o fenômeno do bouncing em botões por meio de debouncing via software.
Manipular e controlar LEDs comuns e LEDs endereçáveis WS2812.
Fixar o estudo do uso de resistores de pull-up internos em botões de acionamento.
Desenvolver um projeto funcional que combine hardware e software.
Descrição do Projeto
Neste projeto, utilizamos os seguintes componentes conectados à placa BitDogLab:

Matriz 5x5 de LEDs (endereçáveis) WS2812, conectada à GPIO 7.
LED RGB, com os pinos conectados às GPIOs (11, 12 e 13).
Botão A, conectado à GPIO 5.
Botão B, conectado à GPIO 6.
Funcionalidades
O sistema apresenta as seguintes funcionalidades:

O LED vermelho do LED RGB pisca continuamente 5 vezes por segundo.
O botão A incrementa o número exibido na matriz de LEDs cada vez que for pressionado.
O botão B decrementa o número exibido na matriz de LEDs cada vez que for pressionado.
A matriz de LEDs WS2812 é utilizada para criar efeitos visuais representando números de 0 a 9.
Cada número é exibido na matriz em um formato fixo, como caracteres em estilo digital, com segmentos iluminados formando o número.
Requisitos
Para o desenvolvimento deste projeto, foram seguidos os seguintes requisitos:

Uso de interrupções: Todas as funcionalidades relacionadas aos botões foram implementadas utilizando rotinas de interrupção (IRQ).
Debouncing: O tratamento do bouncing dos botões foi implementado via software.
Controle de LEDs: O projeto inclui o uso de LEDs comuns e LEDs WS2812, demonstrando o domínio de diferentes tipos de controle.
Organização do código: O código está bem estruturado e comentado para facilitar o entendimento.
Como Executar
Pré-requisitos
Raspberry Pi Pico com o RP2040.
Placa BitDogLab.
VS Code com o Pico SDK instalado.
Biblioteca WS2812 (para controlar os LEDs endereçáveis).
