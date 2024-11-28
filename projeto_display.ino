#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

// Definição dos pinos
#define X_JOYSTICK A0     // Pino analógico para joystick
#define BOTAO 2           // Pino do botão para selecionar/pausar
#define LED_PIN 9         // Pino do LED

// Configuração do display
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R2, 13, 11, 10, U8X8_PIN_NONE);

// Variáveis de navegação e seleção
int opcaoSelecionada = 0;
bool opcaoConfirmada = false;

// Opções de tempo (5 min, 10 min, 20 min) e seus tempos em segundos
const char* opcoes[] = {" 10 min ", " 15 min", " 20 min"};
unsigned long tempos[] = {10 * 60, 15 * 60, 20 * 60}; // Tempo em segundos

// Debounce do botão
bool debounceBotao(int pinoBotao) {
    static unsigned long ultimoTempo = 0;
    static bool estadoAnterior = HIGH;
    bool estadoAtual = digitalRead(pinoBotao);

    if (estadoAtual == LOW && estadoAnterior == HIGH && millis() - ultimoTempo > 50) {
        ultimoTempo = millis();
        estadoAnterior = estadoAtual;
        return true;
    }
    estadoAnterior = estadoAtual;
    return false;
}

// Variável global para acumular o tempo total cronometrado
unsigned long tempoTotalCronometrado = 0; // Tempo total acumulado em segundos

void exibirMensagemConfirmacao() {
    u8g2.clearBuffer();
    char mensagem[30];
    snprintf(mensagem, sizeof(mensagem), "%s selecionado", opcoes[opcaoSelecionada]);
    int16_t x = (128 - u8g2.getStrWidth(mensagem)) / 2;
    int16_t y = 32;
    u8g2.drawStr(x, y, mensagem);
    u8g2.sendBuffer();
    delay(2000);
}

int exibirMenuPausa() {
    int opcaoPausa = 0;
    bool escolhaConfirmada = false;
    digitalWrite(LED_PIN, LOW); // Desliga o LED ao término da contagem
    delay(400);
    while (!escolhaConfirmada) {
        u8g2.setFont(u8g2_font_6x10_tr); // Define fonte pequena como padrão
        u8g2.clearBuffer();

        // Exibe as opções no menu de pausa
        for (int i = 0; i < 2; i++) {
            if (i == opcaoPausa) {
                u8g2.drawStr(0, 20 + i * 20, "[x]");
            } else {
                u8g2.drawStr(0, 20 + i * 20, "[ ]");
            }
            const char* opcoesPausa[] = {" Continuar", " Parar"};
            u8g2.drawStr(16, 20 + i * 20, opcoesPausa[i]);
        }

        u8g2.sendBuffer();

        // Leitura do joystick para navegação
        int leituraJoystick = analogRead(X_JOYSTICK);

        if (leituraJoystick < 400) { // Movimento para cima
            if (opcaoPausa > 0) {
                opcaoPausa--;
                delay(200); // Debounce
            }
        } else if (leituraJoystick > 600) { // Movimento para baixo
            if (opcaoPausa < 1) {
                opcaoPausa++;
                delay(200); // Debounce
            }
        }

        // Confirmação da escolha
        if (debounceBotao(BOTAO)) {
            escolhaConfirmada = true;
        }
    }

    return opcaoPausa;
}

void iniciarContagemRegressiva(unsigned long tempoRestante) {
    unsigned long tempoInicial = tempoRestante; // Salva o tempo inicial
    digitalWrite(LED_PIN, HIGH); // Liga o LED indicando início da contagem
    u8g2.setFont(u8g2_font_fub30_tr); // Configura uma fonte maior para o display

    bool pausado = false; // Variável para controlar a pausa

    while (tempoRestante > 0) {
        if (digitalRead(BOTAO) == LOW) { // Verifica se o botão foi pressionado
            pausado = true;              // Entra no estado de pausa
        }

        if (pausado) {
            int escolha = exibirMenuPausa(); // Mostra o menu de pausa
            
            if (escolha == 0) { // Continuar
                pausado = false;
                u8g2.setFont(u8g2_font_fub30_tr); // Configura uma fonte maior para o display
                digitalWrite(LED_PIN, HIGH); // Liga o LED indicando início da contagem
            } else if (escolha == 1) { // Parar
                digitalWrite(LED_PIN, LOW); // Desliga o LED ao término da contagem
                u8g2.setFont(u8g2_font_6x10_tr); // Retorna para a fonte menor
                opcaoConfirmada = false; // Reseta o estado de confirmação
                break; // Sai do loop de contagem
            }
        }

        // Atualiza o display e decrementa o tempo se não estiver pausado
        if (!pausado) {
            
            int minutos = tempoRestante / 60;
            int segundos = tempoRestante % 60;

            u8g2.clearBuffer();
            
            // Configura a fonte para o cronômetro (fonte grande)
            u8g2.setFont(u8g2_font_fub30_tr); // Configura uma fonte maior para o display
            char contador[16];
            snprintf(contador, sizeof(contador), "%02d:%02d", minutos, segundos);
            int16_t x = (128 - u8g2.getStrWidth(contador)) / 2;
            int16_t y = 40;
            u8g2.drawStr(x, y, contador);
            
            // Configura a fonte para o texto pequeno
            u8g2.setFont(u8g2_font_6x10_tr);
            const char* textoPequeno = "press 1 sec pause";
            int16_t xTexto = (128 - u8g2.getStrWidth(textoPequeno)) / 2;
            int16_t yTexto = y + 20; // Posição abaixo do cronômetro
            u8g2.drawStr(xTexto, yTexto, textoPequeno);
        
            u8g2.sendBuffer();

            delay(1000); // Aguarda 1 segundo
            tempoRestante--;
        }
    }

    // Atualiza o tempo total cronometrado
    tempoTotalCronometrado += (tempoInicial - tempoRestante);

    digitalWrite(LED_PIN, LOW); // Desliga o LED ao término da contagem
    u8g2.setFont(u8g2_font_6x10_tr); // Retorna para a fonte menor
    opcaoConfirmada = false; // Reseta o estado de confirmação
}


void navegarOpcoes() {
    int leituraJoystick = analogRead(X_JOYSTICK);

    if (leituraJoystick < 400) { // Movimento para cima
        if (opcaoSelecionada > 0) {
            opcaoSelecionada--;
            delay(200); // Debounce
        }
    } else if (leituraJoystick > 600) { // Movimento para baixo
        if (opcaoSelecionada < 2) {
            opcaoSelecionada++;
            delay(200); // Debounce
        }
    }

    exibirMenu();
}

void exibirMenu() {
    u8g2.clearBuffer();

    // Exibe as opções no menu
    for (int i = 0; i < 3; i++) {
        if (i == opcaoSelecionada) {
            u8g2.drawStr(0, 20 + i * 20, "[x]");
        } else {
            u8g2.drawStr(0, 20 + i * 20, "[ ]");
        }
        u8g2.drawStr(16, 20 + i * 20, opcoes[i]);
    }

    // Calcula e exibe o tempo total no canto superior direito
    int horasTotais = tempoTotalCronometrado / 3600;           // Calcula as horas
    int minutosTotais = (tempoTotalCronometrado % 3600) / 60;  // Calcula os minutos restantes
    int segundosTotais = tempoTotalCronometrado % 60;          // Calcula os segundos restantes

    char tempoTotal[16];
    snprintf(tempoTotal, sizeof(tempoTotal), "tempo %02d:%02d:%02d", horasTotais, minutosTotais, segundosTotais);

    int16_t x = 128 - u8g2.getStrWidth(tempoTotal) - 2; // Posiciona no canto superior direito
    int16_t y = 10; // Posição no topo
    u8g2.drawStr(x, y, tempoTotal);

    u8g2.sendBuffer();
}

void setup() {
    pinMode(X_JOYSTICK, INPUT);
    pinMode(BOTAO, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tr); // Define fonte menor como padrão
}

void loop() {
    if (opcaoConfirmada) {
        iniciarContagemRegressiva(tempos[opcaoSelecionada]);
    } else {
        if (debounceBotao(BOTAO)) {
            opcaoConfirmada = true;
            exibirMensagemConfirmacao();
        } else {
            navegarOpcoes();
        }
    }
}
