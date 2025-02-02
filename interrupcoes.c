#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include <stdint.h>

#include "ws2812.pio.h"  

// Definições de pinos e constantes
#define LED_COUNT      25      // Número total de LEDs na matriz 5x5 WS2812
#define WS2812_PIN     7       // Pino conectado à matriz WS2812
#define BOTAO_A        5       // Pino conectado ao Botão A
#define BOTAO_B        6       // Pino conectado ao Botão B
#define LED_R          13      // Pino para o componente vermelho do LED RGB
#define BLINK_INTERVAL 100     // Intervalo para piscar o LED vermelho: 100ms (5 Hz)
#define DEBOUNCE_TIME  200     // Tempo de debouncing para os botões (em ms)
#define INTENSIDADE 0.5  // 50% de intensidade

// Variáveis globais para temporização do LED RGB
uint32_t lastBlinkTime = 0;
bool ledState = false;

// Variáveis de debouncing para os botões (usadas na ISR)
volatile uint32_t lastPressTimeA = 0;
volatile uint32_t lastPressTimeB = 0;

// Flags para sinalizar eventos de botão (para serem processados no loop principal)
volatile bool evento_botao_a = false;
volatile bool evento_botao_b = false;

// Variável global para armazenar o número atual exibido (0 a 9)
volatile int numero_atual = 0;

// Estrutura para os LEDs WS2812 (formato RGB)
typedef struct {
    uint8_t R, G, B;
} npLED_t;

npLED_t leds[LED_COUNT];

PIO np_pio;  // PIO utilizado para os WS2812
uint sm;     // State machine utilizada

// ======================= Funções para WS2812 ==============================

// Inicializa o PIO e a state machine para controlar os WS2812
void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2812_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, true);
    ws2812_program_init(np_pio, sm, offset, pin, 800000.f, false);

    // Inicializa os LEDs apagados
    for (uint i = 0; i < LED_COUNT; ++i) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Define a cor de um LED específico da matriz WS2812
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

// Apaga todos os LEDs da matriz
void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        npSetLED(i, 0, 0, 0);
    }
}

// Envia os dados de cor para os LEDs WS2812
void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        // O formato usado pelo WS2812 é GRB
        pio_sm_put_blocking(np_pio, sm, (leds[i].G << 16) | (leds[i].R << 8) | leds[i].B);
    }
    sleep_us(100);  // Pequeno delay para garantir a atualização
}

// =================== Definição dos padrões de exibição ======================

// Cores associadas a cada número
uint32_t cores_numeros[10] = {
    0xFF0000, // 0: Vermelho
    0x00FF00, // 1: Verde
    0x40E0D0, // 2: Azul turquesa
    0xFFFF00, // 3: Amarelo
    0xFF00FF, // 4: Magenta
    0x00FFFF, // 5: Ciano
    0xFF8000, // 6: Laranja
    0x800080, // 7: Roxo
    0x008080, // 8: Teal
    0x808000  // 9: Oliva
};

// Padrões fixos para exibição dos números (matriz 5x5; 1 liga o LED, 0 desliga)
uint8_t numeros[10][LED_COUNT] = {
    {0, 1, 1, 1, 0,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     0, 1, 1, 1, 0}, // 0
    
    {0, 0, 1, 0, 0,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 1, 1, 1, 0}, // 1
    
    {0, 1, 1, 1, 0,
     1, 0, 0, 0, 1,
     0, 0, 1, 1, 0,
     0, 1, 0, 0, 0,
     1, 1, 1, 1, 1}, // 2
    
    {1, 1, 1, 1, 0,
     0, 0, 0, 0, 1,
     0, 0, 1, 1, 0,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 0}, // 3
    
    {0, 0, 1, 1, 0,
     0, 1, 0, 1, 0,
     1, 0, 0, 1, 0,
     1, 1, 1, 1, 1,
     0, 0, 0, 1, 0}, // 4
    
    {1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 0,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 0}, // 5
    
    {0, 1, 1, 1, 0,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 0,
     1, 0, 0, 0, 1,
     0, 1, 1, 1, 0}, // 6
    
    {1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     0, 0, 0, 1, 0,
     0, 0, 1, 0, 0,
     0, 1, 0, 0, 0}, // 7
    
    {0, 1, 1, 1, 0,
     1, 0, 0, 0, 1,
     0, 1, 1, 1, 0,
     1, 0, 0, 0, 1,
     0, 1, 1, 1, 0}, // 8
    
    {0, 1, 1, 1, 0,
     1, 0, 0, 0, 1,
     0, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     0, 1, 1, 1, 0}  // 9
};

// Função para exibir o número atual na matriz WS2812
void mostrarNumero(int numero) {
    npClear();
    uint32_t cor = cores_numeros[numero];  // Seleciona a cor definida para o número
    uint8_t r = (cor >> 16) & 0xFF;
    uint8_t g = (cor >> 8) & 0xFF;
    uint8_t b = cor & 0xFF;

    // Aplica a intensidade aos valores RGB
    r = (uint8_t)(r * INTENSIDADE);
    g = (uint8_t)(g * INTENSIDADE);
    b = (uint8_t)(b * INTENSIDADE);

for (int y = 0; y < 5; y++) { // Loop para as linhas (y)
    for (int x = 0; x < 5; x++) { // Loop para as colunas (x)
        // Calcula o índice do LED na matriz (inverte as linhas e alterna a direção das colunas)
        int index = (4 - y) * 5 + ((4 - y) % 2 == 0 ? (4 - x) : x);
        
        // Se o LED deve ser aceso (de acordo com a matriz de números), define a cor
        if (numeros[numero][y * 5 + x]) {
            npSetLED(index, r, g, b); // Define a cor com a intensidade especificada
        }
    }
}
    npWrite();
}

// ===================== Rotina de Interrupção (ISR) ===========================

// A ISR apenas sinaliza que um botão foi pressionado (após debouncing)
// para que o processamento seja feito no loop principal.
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());

    // Verifica se o debounce foi superado
    if (gpio == BOTAO_A && (currentTime - lastPressTimeA > DEBOUNCE_TIME)) {
        lastPressTimeA = currentTime;
        evento_botao_a = true;  // Sinaliza que o botão A foi pressionado
    }
    if (gpio == BOTAO_B && (currentTime - lastPressTimeB > DEBOUNCE_TIME)) {
        lastPressTimeB = currentTime;
        evento_botao_b = true;  // Sinaliza que o botão B foi pressionado
    }
}

// ===================== Função para piscar o LED RGB ===========================

// Pisca o LED vermelho (GPIO LED_R) de forma não bloqueante
void piscarLED() {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    if (currentTime - lastBlinkTime >= BLINK_INTERVAL) {
        lastBlinkTime = currentTime;
        ledState = !ledState;
        gpio_put(LED_R, ledState);
    }
}

// =============================== main ========================================

int main() {
    stdio_init_all();

    // Configuração do LED RGB (apenas o componente vermelho)
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);

    // Configuração dos botões com resistor pull-up interno e interrupções
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    // Registra a ISR para os botões (a mesma função trata ambos)
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Inicializa os WS2812 e exibe inicialmente o número 0
    npInit(WS2812_PIN);
    npClear();
    mostrarNumero(numero_atual);

    while (true) {
        // Pisca o LED RGB (5 vezes por segundo)
        piscarLED();

        // Processa o evento do Botão A: incrementa o número
        if (evento_botao_a) {
            evento_botao_a = false;  // Reseta a flag
            if (numero_atual < 9) {
                numero_atual++;
            } else {
                numero_atual = 0;
            }
            mostrarNumero(numero_atual);
        }

        // Processa o evento do Botão B: decrementa o número
        if (evento_botao_b) {
            evento_botao_b = false;  // Reseta a flag
            if (numero_atual > 0) {
                numero_atual--;
            } else {
                numero_atual = 9;
            }
            mostrarNumero(numero_atual);
        }
    }

    return 0;
}
