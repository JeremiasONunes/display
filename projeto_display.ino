#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

// Definição dos pinos
#define X_JOYSTICK A0     // Pino analógico para joystick
#define BOTAO 2           // Pino do botão para selecionar
#define LED_PIN 9         // Pino do LED

// Configuração do display
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R2, 13, 11, 10, U8X8_PIN_NONE);

// Variáveis de navegação e seleção
int opcaoSelecionada = 0;
bool joystickMovido = false;
bool opcaoConfirmada = false;

// Opções de tempo (5 min, 10 min, 20 min) e seus tempos em ms
const char* opcoes[] = {" 5 min ", " 10 min", " 20 min"};
unsigned long tempos[] = {5 * 60 * 1000, 10 * 60 * 1000, 20 * 60 * 1000};

void setup() {
  pinMode(X_JOYSTICK, INPUT);      // Pino do joystick
  pinMode(BOTAO, INPUT_PULLUP);    // Pino do botão com pull-up
  pinMode(LED_PIN, OUTPUT);        // Pino do LED
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tr);
}

void loop() {
  // Verifica se o botão foi pressionado para confirmar a opção
  if (digitalRead(BOTAO) == LOW) {
    opcaoConfirmada = true;
    delay(200); // Debounce do botão
  }

  if (opcaoConfirmada) {
    // Exibe confirmação no display
    u8g2.clearBuffer();
    char mensagem[30];
    snprintf(mensagem, sizeof(mensagem), "%s selecionada", opcoes[opcaoSelecionada]);
    int16_t x = (128 - u8g2.getStrWidth(mensagem)) / 2;
    int16_t y = 32;
    u8g2.drawStr(x, y, mensagem);
    u8g2.sendBuffer();
    delay(2000);

    // Acende o LED pelo tempo da opção selecionada
    digitalWrite(LED_PIN, HIGH);           // Acende o LED
    delay(tempos[opcaoSelecionada]);       // Mantém o LED aceso pelo tempo
    digitalWrite(LED_PIN, LOW);            // Apaga o LED após o tempo

    opcaoConfirmada = false; // Reseta confirmação
  } else {
    // Lógica de navegação com o joystick (como antes)
    int leituraJoystick = analogRead(X_JOYSTICK);
    if (leituraJoystick < 300 && !joystickMovido) {
      opcaoSelecionada = (opcaoSelecionada + 2) % 3;
      joystickMovido = true;
      delay(200);
    } else if (leituraJoystick > 700 && !joystickMovido) {
      opcaoSelecionada = (opcaoSelecionada + 1) % 3;
      joystickMovido = true;
      delay(200);
    }

    if (leituraJoystick >= 300 && leituraJoystick <= 700) {
      joystickMovido = false;
    }

    // Exibe as opções no display
    u8g2.clearBuffer();
    for (int i = 0; i < 3; i++) {
      if (i == opcaoSelecionada) {
        u8g2.drawStr(0, 15 + i * 20, "[x]");
      } else {
        u8g2.drawStr(0, 15 + i * 20, "[ ]");
      }
      u8g2.drawStr(16, 15 + i * 20, opcoes[i]);
    }
    u8g2.sendBuffer();
  }
  delay(100);
}
