#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

// Definição dos pinos de conexão entre os componentes e a placa
#define X_JOYSTICK A0  // Pino analógico para navegação com o joystick
#define BOTAO 2        // Pino do botão para selecionar a opção

// Criação do objeto para o display LCD
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R2, /* clock=*/13, /* data=*/11, /* cs=*/10, /* reset=*/U8X8_PIN_NONE);

// Variáveis para controle de navegação e seleção
int opcaoSelecionada = 0; // Armazena a opção atualmente selecionada (0 a 2)
bool joystickMovido = false; // Flag para evitar múltiplas leituras rápidas
bool opcaoConfirmada = false; // Flag para indicar se a opção foi confirmada

// Lista de opções (adicione ou altere o texto aqui conforme necessário)
const char* opcoes[] = {" 5 min ", " 10 min", " 20 min"};

void setup() {
  pinMode(X_JOYSTICK, INPUT);       // Configura o pino do joystick como entrada
  pinMode(BOTAO, INPUT_PULLUP);     // Configura o pino do botão com resistor pull-up
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tr); // Fonte menor para evitar corte de texto
}

void loop() {
  // Verifica se o botão foi pressionado
  if (digitalRead(BOTAO) == LOW) {
    opcaoConfirmada = true; // Define que a opção foi confirmada
    delay(200); // Debounce para evitar leituras repetidas do botão
  }

  // Estrutura para verificar se uma opção foi confirmada
  if (opcaoConfirmada) {
    // Exibe a mensagem de confirmação centralizada no display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr); // Fonte para a mensagem

    // Prepara a mensagem "Opção X selecionada"
    char mensagem[30];
    snprintf(mensagem, sizeof(mensagem), "%s selecionada", opcoes[opcaoSelecionada]);

    // Centraliza a mensagem no display
    int16_t x = (128 - u8g2.getStrWidth(mensagem)) / 2; // Calcula posição X central
    int16_t y = 32; // Posição Y central na altura do display
    u8g2.drawStr(x, y, mensagem);

    u8g2.sendBuffer(); // Atualiza o display com a mensagem
    if (opcoes[opcaoSelecionada] == " 5 min "){
      
    }
    delay(2000); // Exibe a mensagem por 2 segundos

    opcaoConfirmada = false; // Reseta a confirmação para voltar ao menu

  } else {
    // Lógica de navegação entre as opções com o joystick
    int leituraJoystick = analogRead(X_JOYSTICK);

    if (leituraJoystick < 300 && !joystickMovido) { // Movimento para a esquerda
      opcaoSelecionada = (opcaoSelecionada + 2) % 3; // Alterna entre as opções
      joystickMovido = true; // Ativa flag para evitar múltiplas leituras rápidas
      delay(200); // Pausa para dar tempo de reação ao joystick
    } 
    else if (leituraJoystick > 700 && !joystickMovido) { // Movimento para a direita
      opcaoSelecionada = (opcaoSelecionada + 1) % 3;
      joystickMovido = true;
      delay(200);
    }

    // Reset da flag quando o joystick volta para o centro
    if (leituraJoystick >= 300 && leituraJoystick <= 700) {
      joystickMovido = false;
    }

    // Exibe as opções no display com destaque na opção selecionada
    u8g2.clearBuffer(); // Limpa o buffer do display

    // Exibe cada opção com uma caixa de seleção, destacando a opção selecionada
    for (int i = 0; i < 3; i++) {
      if (i == opcaoSelecionada) {
        u8g2.drawStr(0, 15 + i * 20, "[x]"); // Caixa marcada para a opção selecionada
      } else {
        u8g2.drawStr(0, 15 + i * 20, "[ ]"); // Caixa vazia para as outras opções
      }
      u8g2.drawStr(16, 15 + i * 20, opcoes[i]); // Desenha o texto da opção
    }

    u8g2.sendBuffer(); // Envia os dados para o display
  }

  delay(100); // Intervalo para evitar atualizações muito rápidas
}
