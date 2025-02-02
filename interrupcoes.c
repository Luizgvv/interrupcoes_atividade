#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include <stdint.h>

#include "ws2812.pio.h"  

// Definições de pinos e constantes
#define LED_COUNT      25      // Número total de LEDs na matriz 5x5 WS2812
#define NUM_PIXELS     LED_COUNT // Para compatibilidade com o código abaixo
#define WS2812_PIN     7       // Pino conectado à matriz WS2812
#define BOTAO_A        5       // Pino conectado ao Botão A
#define BOTAO_B        6       // Pino conectado ao Botão B
#define LED_R          13      // Pino para o componente vermelho do LED RGB
#define BLINK_INTERVAL 100     // Intervalo para piscar o LED vermelho: 100ms (5 Hz)
#define DEBOUNCE_TIME  200     // Tempo de debouncing para os botões (em ms)
#define INTENSIDADE    0.5     // 50% de intensidade
#define IS_RGBW        false  
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
uint sm;     // Máquina de estado utilizada

// ======================= Funções para WS2812 ==============================

// Função auxiliar para enviar um pixel (o WS2812 espera dados no formato GRB)
static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

// Converte os valores RGB para um valor de 24 bits no formato GRB
// (Note que, neste código, usamos a ordem: R é deslocado 8, G é deslocado 16 e B fica no byte menos significativo)
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Inicializa o PIO e a máquina de estado para controlar os WS2812
void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2812_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, true);
    ws2812_program_init(np_pio, sm, offset, pin, 800000, IS_RGBW);

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

// Envia os dados de cor para os LEDs WS2812 (formato GRB)
void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        pio_sm_put_blocking(np_pio, sm, (leds[i].G << 16) | (leds[i].R << 8) | leds[i].B);
    }
    sleep_us(100);  // Pequeno delay para garantir a atualização
}

// =================== Definição dos padrões de exibição ======================

// Cores associadas a cada número (0xRRGGBB)
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

// Padrões fixos para exibição dos números (matriz 5x5; cada número tem 25 pixels)
// Usamos um array unidimensional de 25 elementos para cada número.
uint8_t numeros[10][NUM_PIXELS] = {
 // 0
    { 0,1,1,1,0,
      1,0,0,0,1,
      1,0,0,0,1,
      1,0,0,0,1,
      0,1,1,1,0 },
    // 1
    { 0,1,1,1,0,
      0,0,1,0,0,
      0,0,1,0,0,
      0,1,1,0,0,
      0,0,1,0,0 },
    // 2
    { 1,1,1,1,1,
      0,0,1,0,0,
      0,1,0,0,0,
      1,0,0,0,1,
      0,1,1,1,0 },
    // 3
    { 0,1,1,1,1,
      0,0,0,0,1,
      0,1,1,0,0,
      0,0,0,0,1,
      0,1,1,1,1 },
    // 4
    { 0,1,0,0,0,
      1,1,1,1,1,
      0,1,0,0,1,
      0,1,0,1,0,
      0,1,1,0,0 },
    // 5
    { 0,1,1,1,1,
      0,0,0,0,1,
      0,1,1,1,1,
      1,0,0,0,0,
      1,1,1,1,1 },
    // 6
    { 0,1,1,1,0,
      1,0,0,0,1,
      0,1,1,1,1,
      1,0,0,0,0,
      0,1,1,1,0 },
    // 7
    { 0,0,0,1,0,
      0,0,1,0,0,
      0,1,0,0,0,
      0,0,0,0,1,
      1,1,1,1,1 },
    // 8
    { 0,1,1,1,0,
      1,0,0,0,1,
      0,1,1,1,0,
      1,0,0,0,1,
      0,1,1,1,0 },
    // 9
    { 0,1,1,1,0,
      0,0,0,0,1,
      1,1,1,1,0,
      1,0,0,0,1,
      0,1,1,1,0 }

};

// Função para exibir um número na matriz WS2812
// Aqui, usamos o array "numeros" para definir quais LEDs acendem
void display_number(PIO pio, uint sm, uint num) {
    // Seleciona a cor associada ao número e aplica a intensidade
    uint32_t cor = cores_numeros[num];
    uint8_t r = (cor >> 16) & 0xFF;
    uint8_t g = (cor >> 8)  & 0xFF;
    uint8_t b = cor & 0xFF;
    r = (uint8_t)(r * INTENSIDADE);
    g = (uint8_t)(g * INTENSIDADE);
    b = (uint8_t)(b * INTENSIDADE);
    
    
    // Para cada LED na matriz (25 LEDs), se o padrão for "1", envia a cor; caso contrário, apaga
    for (uint i = 0; i < NUM_PIXELS; ++i) {
        if (numeros[num][i])
            put_pixel(pio, sm, urgb_u32(r, g, b));
        else
            put_pixel(pio, sm, urgb_u32(0, 0, 0));
    }
}

// ===================== Rotina de Interrupção (ISR) ===========================

// A ISR apenas sinaliza que um botão foi pressionado (após debouncing)
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    if (gpio == BOTAO_A && (currentTime - lastPressTimeA > DEBOUNCE_TIME)) {
        lastPressTimeA = currentTime;
        evento_botao_a = true;
    }
    if (gpio == BOTAO_B && (currentTime - lastPressTimeB > DEBOUNCE_TIME)) {
        lastPressTimeB = currentTime;
        evento_botao_b = true;
    }
}

// ===================== Função para piscar o LED RGB ===========================

// Pisca o LED vermelho (indicador) de forma não bloqueante
void piscarLED_rgb() {
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
    printf("WS2812 Number Display Test\n");
    
    // Configuração do LED RGB (apenas para indicação)
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    
    // Configuração dos botões com resistor pull-up interno e interrupções
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    
    // Registra a ISR para os botões
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    
    // Inicializa os WS2812
    PIO pio;
    uint sm_local;
    uint offset;
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm_local, &offset, WS2812_PIN, 1, true);
    hard_assert(success);
    
    ws2812_program_init(pio, sm_local, offset, WS2812_PIN, 800000, IS_RGBW);
    
    // Exibe inicialmente o número 0
    uint num = 0;
    display_number(pio, sm_local, num);
    
    while (true) {
        // Pisca o LED RGB (indicador)
        piscarLED_rgb();
        
        // Se o botão A for pressionado, incrementa o número
        if (evento_botao_a) {
            evento_botao_a = false;
            if (num < 9)
                num++;
            else
                num = 0;
            display_number(pio, sm_local, num);
            sleep_ms(300);
        }
        
        // Se o botão B for pressionado, decrementa o número
        if (evento_botao_b) {
            evento_botao_b = false;
            if (num > 0)
                num--;
            else
                num = 9;
            display_number(pio, sm_local, num);
            sleep_ms(300);
        }
    }
    
    return 0;
}
