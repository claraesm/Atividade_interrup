
// Inclusão das bibliotecas necessárias
#include <stdio.h> // Biblioteca padrão da linguagem C
#include "pico/stdlib.h" // Biblioteca padrão para Raspberry Pi Pico
#include "hardware/pio.h"
#include "ws2818b.pio.h" // Biblioteca para controle da matriz de LEDs

// Definição dos pinos dos componentes
#define PINO_LED_MATRIZ 7  // Pino de controle da matriz de LEDs
#define TOTAL_LEDS 25       // Quantidade total de LEDs na matriz
#define PINO_BOTAO_1 5      // Pino do primeiro botão
#define PINO_BOTAO_2 6      // Pino do segundo botão
#define LED_AZUL 11        // Pino do LED azul
#define LED_VERDE 12       // Pino do LED verde
#define LED_VERMELHO 13    // Pino do LED vermelho

// Variável para definir o brilho dos LEDs (em porcentagem)
uint8_t brilho_atual = (255 * 5) / 1000;

int eventos_totais = 10; // Número total de eventos de alternância entre os botões
int estado_sistema = 0;  // Estado atual do sistema, modificado por interação dos botões

// Tempo mínimo entre interrupções para evitar repetição indesejada (debounce)
#define TEMPO_DEBOUNCE 100 // Milissegundos

// Variáveis para controlar o debounce dos botões
volatile uint32_t ultima_interrupcao_botao1 = 0;
volatile uint32_t ultima_interrupcao_botao2 = 0;

/* Função para converter coordenadas (x, y) da matriz
   em um índice correspondente no vetor de LEDs */
int calcularIndice(int x, int y) {
    if (x % 2 == 0) {
        return 24 - (x * 5 + y);
    } else {
        return 24 - (x * 5 + (4 - y));
    }
}

// Função para inicializar os pinos do LED RGB
void configurar_led_rgb() {
    gpio_init(LED_VERMELHO);
    gpio_init(LED_VERDE);
    gpio_init(LED_AZUL);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
}

// Função para acionar os LEDs RGB por um tempo determinado
void acionar_led(bool vermelho, bool verde, bool azul, int tempo_ms) {
    configurar_led_rgb();
    gpio_put(LED_VERMELHO, vermelho);
    gpio_put(LED_VERDE, verde);
    gpio_put(LED_AZUL, azul);
    sleep_ms(tempo_ms);
}

// Estrutura para armazenar as cores de cada LED da matriz
struct cor_led_t {
    uint8_t vermelho, verde, azul;
};
typedef struct cor_led_t matrizLED_t;
matrizLED_t matriz_leds[TOTAL_LEDS];

// Definição do PIO e state machine para controle dos LEDs
PIO pio_leds;
uint state_machine;

// Função para atualizar os LEDs da matriz
void atualizar_matriz() {
    for (uint i = 0; i < TOTAL_LEDS; ++i) {
        pio_sm_put_blocking(pio_leds, state_machine, matriz_leds[i].vermelho);
        pio_sm_put_blocking(pio_leds, state_machine, matriz_leds[i].verde);
        pio_sm_put_blocking(pio_leds, state_machine, matriz_leds[i].azul);
    }
    sleep_us(100);
}

// Função para inicializar a matriz de LEDs

void iniciar_matriz_leds(uint pino) {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    pio_leds = pio0;
    state_machine = pio_claim_unused_sm(pio_leds, true);
    ws2818b_program_init(pio_leds, state_machine, offset, pino, 800000.f);

    for (uint i = 0; i < TOTAL_LEDS; ++i) {
        matriz_leds[i].vermelho = matriz_leds[i].verde = matriz_leds[i].azul = 0;
    }
    atualizar_matriz();
}

// Função para definir a cor de um LED específico na matriz
void definir_cor_led(const uint indice, const uint8_t r, const uint8_t g, const uint8_t b) {
    matriz_leds[indice].vermelho = r;
    matriz_leds[indice].verde = g;
    matriz_leds[indice].azul = b;
}

// Funções para desenhar os números na matriz de LEDs
void desenhar_numero_0() {
    int matriz[5][5][3] = {
        {{0, 0, 0}, {brilho_atual, 0, 0}, {brilho_atual, 0, 0}, {brilho_atual, 0, 0},{0, 0, 0}},
        {{0, 0, 0}, {brilho_atual, 0, 0}, {0, 0, 0}, {brilho_atual, 0, 0},  {0, 0, 0}},
        {{0, 0, 0}, {brilho_atual, 0, 0}, {0, 0, 0}, {brilho_atual, 0, 0},  { 0, 0, 0}},
        {{0, 0, 0}, {brilho_atual, 0, 0}, {0, 0, 0}, {brilho_atual, 0, 0},  {0, 0, 0}},
        {{0, 0, 0}, {brilho_atual, 0, 0}, {brilho_atual, 0, 0}, {brilho_atual, 0, 0},{0, 0, 0}}
    };
    for (int linha = 0; linha < 5; linha++) {
        for (int coluna = 0; coluna < 5; coluna++) {
            int posicao = calcularIndice(linha, coluna);
            definir_cor_led(posicao, matriz[linha][coluna][0], matriz[linha][coluna][1], matriz[linha][coluna][2]);
        }
    }
    atualizar_matriz();
}

void desenhar_numero_1() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, 0, 0}, {0, brilho_atual, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, brilho_atual, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, brilho_atual, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, brilho_atual, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, brilho_atual, 0}, {0, 0, 0}, {0, 0, 0}}
    };

    // Exibir a primeira matriz
    for (int linha = 0; linha < 5; linha++) {
        for (int cols = 0; cols < 5; cols++) {
            int posicao = calcularIndice(linha, cols);
            definir_cor_led(posicao, mat1[linha][cols][0], mat1[linha][cols][1], mat1[linha][cols][2]);
        }
    }
    atualizar_matriz();
}

void desenhar_numero_2() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, 0}}
    };

    // Exibir a primeira matriz
    for (int linha = 0; linha < 5; linha++) {
        for (int cols = 0; cols < 5; cols++) {
            int posicao = calcularIndice(linha, cols);
            definir_cor_led(posicao, mat1[linha][cols][0], mat1[linha][cols][1], mat1[linha][cols][2]);
        }
    }
    atualizar_matriz();
}

void desenhar_numero_3() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {100, brilho_atual, 0}, {100, brilho_atual, 0}, {100, brilho_atual, 0}, {0, 0, 0  }},
        {{0, 0, 0}, {0, 0, 0},     {0, 0,     0}, {100, brilho_atual, 0}, {0, 0,   0}},
        {{0, 0, 0}, {100, brilho_atual, 0}, {100, brilho_atual, 0}, {100, brilho_atual, 0}, {0, 0,   0}},
        {{0, 0, 0}, {0, 0, 0},     {0, 0,     0}, {100, brilho_atual, 0}, {0, 0,   0}},
        {{0, 0, 0}, {100, brilho_atual, 0}, {100, brilho_atual, 0}, {100, brilho_atual, 0}, {0, 0,   0}}
    };

    // Exibir a primeira matriz
    for (int linha = 0; linha < 5; linha++) {
        for (int cols = 0; cols < 5; cols++) {
            int posicao = calcularIndice(linha, cols);
            definir_cor_led(posicao, mat1[linha][cols][0], mat1[linha][cols][1], mat1[linha][cols][2]);
        }
    }
    atualizar_matriz();
}

void desenhar_numero_4() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, 0}, {0, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, 0}, {0, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, brilho_atual}, {0, 0, 0}}
    };

    // Exibir a primeira matriz
    for (int linha = 0; linha < 5; linha++) {
        for (int cols = 0; cols < 5; cols++) {
            int posicao = calcularIndice(linha, cols);
            definir_cor_led(posicao, mat1[linha][cols][0], mat1[linha][cols][1], mat1[linha][cols][2]);
        }
    }
    atualizar_matriz();
}

void desenhar_numero_5() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, brilho_atual, 0}, {0, brilho_atual, 0}, {0, brilho_atual, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, brilho_atual, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, brilho_atual, 0}, {0, brilho_atual, 0}, {0, brilho_atual, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, brilho_atual, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, brilho_atual, 0}, {0, brilho_atual, 0}, {0, brilho_atual, 0}, {0, 0, 0}}
    
    };

    // Exibir a matriz
    for (int linha = 0; linha < 5; linha++) {
        for (int cols = 0; cols < 5; cols++) {
            int posicao = calcularIndice(linha, cols);
            definir_cor_led(posicao, mat1[linha][cols][0], mat1[linha][cols][1], mat1[linha][cols][2]);
        }
    }
    atualizar_matriz();
}

void desenhar_numero_6() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, 0}, {0, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, brilho_atual}, {0, 0, 0}}
    };

    // Exibir a matriz
    for (int linha = 0; linha < 5; linha++) {
        for (int cols = 0; cols < 5; cols++) {
            int posicao = calcularIndice(linha, cols);
            definir_cor_led(posicao, mat1[linha][cols][0], mat1[linha][cols][1], mat1[linha][cols][2]);
        }
    }
    atualizar_matriz();
}

void desenhar_numero_7() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, brilho_atual, brilho_atual}, {0, brilho_atual, brilho_atual}, {0, brilho_atual, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, brilho_atual, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, brilho_atual, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, brilho_atual, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, brilho_atual, brilho_atual}, {0, 0, 0}}
    };

    // Exibir a matriz
    for (int linha = 0; linha < 5; linha++) {
        for (int cols = 0; cols < 5; cols++) {
            int posicao = calcularIndice(linha, cols);
            definir_cor_led(posicao, mat1[linha][cols][0], mat1[linha][cols][1], mat1[linha][cols][2]);
        }
    }
    atualizar_matriz();
}

void desenhar_numero_8() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {brilho_atual, 0, brilho_atual}, {brilho_atual, 0, brilho_atual}, {brilho_atual, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {brilho_atual, 0, brilho_atual}, {0, 0, 0}, {brilho_atual, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {brilho_atual, 0, brilho_atual}, {brilho_atual, 0, brilho_atual}, {brilho_atual, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {brilho_atual, 0, brilho_atual}, {0, 0, 0}, {brilho_atual, 0, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {brilho_atual, 0, brilho_atual}, {brilho_atual, 0, brilho_atual}, {brilho_atual, 0, brilho_atual}, {0, 0, 0}}
    };

    // Exibir a matriz
    for (int linha = 0; linha < 5; linha++) {
        for (int cols = 0; cols < 5; cols++) {
            int posicao = calcularIndice(linha, cols);
            definir_cor_led(posicao, mat1[linha][cols][0], mat1[linha][cols][1], mat1[linha][cols][2]);
        }
    }
    atualizar_matriz();
}

void desenhar_numero_9() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {brilho_atual, brilho_atual, brilho_atual}, {brilho_atual, brilho_atual, brilho_atual}, {brilho_atual, brilho_atual, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {brilho_atual, brilho_atual, brilho_atual}, {0, 0, 0}, {brilho_atual, brilho_atual, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {brilho_atual, brilho_atual, brilho_atual}, {brilho_atual, brilho_atual, brilho_atual}, {brilho_atual, brilho_atual, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {brilho_atual, brilho_atual, brilho_atual}, {0, 0, 0}},
        {{0, 0, 0}, {brilho_atual, brilho_atual, brilho_atual}, {brilho_atual, brilho_atual, brilho_atual}, {brilho_atual, brilho_atual, brilho_atual}, {0, 0, 0}}

    };

    // Exibir a matriz
    for (int linha = 0; linha < 5; linha++) {
        for (int cols = 0; cols < 5; cols++) {
            int posicao = calcularIndice(linha, cols);
            definir_cor_led(posicao, mat1[linha][cols][0], mat1[linha][cols][1], mat1[linha][cols][2]);
        }
    }
    atualizar_matriz();
}

// Alterna o estado do sistema para exibir o número correspondente na matriz de LEDs
void atualizar_estado() {
    switch (estado_sistema) {
        case 0:
            desenhar_numero_0();
            printf("0\n");
            break;
        case 1:
            desenhar_numero_1();
            printf("1\n");
            break;
        case 2:
            desenhar_numero_2();
            printf("2\n");
            break;
        case 3:
            desenhar_numero_3();
            printf("3\n");
            break;
        case 4:
            desenhar_numero_4();
            printf("4\n");
            break;
        case 5:
            desenhar_numero_5();
            printf("5\n");
            break;
        case 6:
            desenhar_numero_6();
            printf("6\n");
            break;
        case 7:
            desenhar_numero_7();
            printf("7\n");
            break;
        case 8:
            desenhar_numero_8();
            printf("8\n");
            break;
        case 9:
            desenhar_numero_9();
            printf("9\n");
            break;
    }
}

// Função para tratar interrupções e realizar o controle de debounce dos botões
/* Essa função verifica o tempo decorrido desde a última ativação do botão 
   e, caso tenha passado o tempo mínimo necessário para evitar múltiplas leituras 
   indevidas (debounce), atualiza o estado do sistema. */
void debounce_botao(uint pino, volatile uint32_t *ultimo_tempo_irq, int direcao) {
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
    if (tempo_atual - *ultimo_tempo_irq > TEMPO_DEBOUNCE) {
        *ultimo_tempo_irq = tempo_atual;
        estado_sistema += direcao;
        if (estado_sistema >= eventos_totais) estado_sistema = 0;
        if (estado_sistema < 0) estado_sistema = eventos_totais - 1;
        atualizar_estado();
    }
}

// Configura os pinos dos botões como entradas com pull-up interno ativado
void iniciar_botoes() {
    gpio_init(PINO_BOTAO_1);
    gpio_init(PINO_BOTAO_2);
    gpio_set_dir(PINO_BOTAO_1, GPIO_IN);
    gpio_set_dir(PINO_BOTAO_2, GPIO_IN);
    gpio_pull_up(PINO_BOTAO_1);
    gpio_pull_up(PINO_BOTAO_2);
}

// Função principal do sistema
int main() {
    stdio_init_all();
    iniciar_matriz_leds(PINO_LED_MATRIZ); // Configura a matriz de LEDs
    iniciar_botoes(); // Inicializa os botões para interação do usuário
    printf("0\n");
    desenhar_numero_0(); // Define a matriz de LEDs para exibir o número inicial (0)

    while (1) {
        acionar_led(1, 0, 0, 180); // Aciona o LED RGB vermelho por 180 ms
        acionar_led(0, 0, 0, 20);  // Desliga todos os LEDs por 20 ms

        // Verifica se o botão B foi pressionado e decrementa o estado
        if (!gpio_get(PINO_BOTAO_2)) debounce_botao(PINO_BOTAO_2, &ultima_interrupcao_botao2, -1);
        // Verifica se o botão A foi pressionado e incrementa o estado
        if (!gpio_get(PINO_BOTAO_1)) debounce_botao(PINO_BOTAO_1, &ultima_interrupcao_botao1, +1);
        sleep_ms(10); // Pequena pausa para evitar leituras excessivas dos botões

    }
    return (0); 

}