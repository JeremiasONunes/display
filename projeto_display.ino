#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

// Definição dos pinos
#define X_JOYSTICK A0     // Pino analógico para o eixo X do joystick
#define BOTAO 2           // Pino digital para o botão de seleção
#define LED_PIN 9         // Pino digital para o LED indicador

// Configuração do display (U8G2 com protocolo SPI)
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R2, 13, 11, 10, U8X8_PIN_NONE);

// Variáveis de navegação e seleção
int opcaoSelecionada = 0;        // Índice da opção selecionada no menu
bool opcaoConfirmada = false;    // Indica se uma opção foi confirmada

// Definição das opções e seus tempos correspondentes em segundos
const char* opcoes[] = {" 5 min ", " 10 min", " 20 min"}; // Opções do menu
unsigned long tempos[] = {5 * 60, 10 * 60, 20 * 60};      // Tempos em segundos (300s, 600s, 1200s)

void setup() {
    pinMode(X_JOYSTICK, INPUT);      // Configura o pino do joystick como entrada
    pinMode(BOTAO, INPUT_PULLUP);   // Configura o pino do botão como entrada com resistor pull-up
    pinMode(LED_PIN, OUTPUT);       // Configura o pino do LED como saída
    Serial.begin(115200);           // Inicializa a comunicação serial para depuração
    u8g2.begin();                   // Inicializa o display
    u8g2.setFont(u8g2_font_6x10_tr); // Configura a fonte do display
}

// Função para exibir uma mensagem de confirmação no display
void exibirMensagemConfirmacao() {
    u8g2.clearBuffer(); // Limpa o buffer do display
    char mensagem[30];
    snprintf(mensagem, sizeof(mensagem), "%s selecionada", opcoes[opcaoSelecionada]);
    int16_t x = (128 - u8g2.getStrWidth(mensagem)) / 2; // Calcula a posição centralizada no eixo X
    int16_t y = 32; // Posição no eixo Y
    u8g2.drawStr(x, y, mensagem); // Exibe a mensagem no display
    u8g2.sendBuffer(); // Envia o buffer para o display
    delay(2000); // Aguarda 2 segundos para que a mensagem seja visível
}

// Função para iniciar a contagem regressiva e exibir no display
void iniciarContagemRegressiva(unsigned long tempoRestante) {
    digitalWrite(LED_PIN, HIGH); // Liga o LED indicando início da contagem
    u8g2.setFont(u8g2_font_fub30_tr); // Configura uma fonte maior para o display

    while (tempoRestante > 0) {
        int minutos = tempoRestante / 60; // Converte segundos restantes para minutos
        int segundos = tempoRestante % 60; // Obtém os segundos restantes

        u8g2.clearBuffer(); // Limpa o buffer do display
        char contador[16];
        snprintf(contador, sizeof(contador), "%02d:%02d", minutos, segundos); // Formata o tempo em "MM:SS"
        int16_t x = (128 - u8g2.getStrWidth(contador)) / 2; // Centraliza o texto no eixo X
        int16_t y = 40; // Define a posição Y
        u8g2.drawStr(x, y, contador); // Exibe o contador no display
        u8g2.sendBuffer(); // Atualiza o display

        delay(1000); // Aguarda 1 segundo
        tempoRestante--; // Reduz o tempo restante
    }

    digitalWrite(LED_PIN, LOW); // Desliga o LED ao término da contagem
    u8g2.setFont(u8g2_font_6x10_tr); // Retorna para a fonte menor
    opcaoConfirmada = false; // Reseta o estado de confirmação
}

// Função para exibir o menu com as opções
void exibirMenu() {
    u8g2.clearBuffer(); // Limpa o buffer do display
    for (int i = 0; i < 3; i++) { // Itera pelas opções do menu
        if (i == opcaoSelecionada) { // Destaca a opção atualmente selecionada
            u8g2.drawStr(0, 20 + i * 20, "[x]"); // Marcação para a opção selecionada
        } else {
            u8g2.drawStr(0, 20 + i * 20, "[ ]"); // Opção não selecionada
        }
        u8g2.drawStr(16, 20 + i * 20, opcoes[i]); // Exibe o texto da opção
    }
    u8g2.sendBuffer(); // Atualiza o display
}

// Função para navegar pelas opções utilizando o joystick
void navegarOpcoes() {
    int leituraJoystick = analogRead(X_JOYSTICK); // Lê o valor do eixo X do joystick

    if (leituraJoystick < 400) { // Movimento para cima
        if (opcaoSelecionada > 0) { // Verifica limites superiores
            opcaoSelecionada--; // Move para a opção anterior
            delay(200); // Delay para debounce
        }
    } else if (leituraJoystick > 600) { // Movimento para baixo
        if (opcaoSelecionada < 2) { // Verifica limites inferiores
            opcaoSelecionada++; // Move para a próxima opção
            delay(200); // Delay para debounce
        }
    }

    exibirMenu(); // Atualiza o menu no display
}

void loop() {
    if (digitalRead(BOTAO) == LOW) { // Verifica se o botão foi pressionado
        opcaoConfirmada = true; // Marca a opção como confirmada
        delay(200); // Delay para debounce
    }

    if (opcaoConfirmada) {
        exibirMensagemConfirmacao(); // Exibe a mensagem de confirmação
        iniciarContagemRegressiva(tempos[opcaoSelecionada]); // Inicia a contagem regressiva
    } else {
        navegarOpcoes(); // Permite navegação no menu
    }

    delay(100); // Delay para evitar leitura excessiva
}
