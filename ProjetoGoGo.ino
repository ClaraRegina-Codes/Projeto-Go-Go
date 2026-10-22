#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define ledGreen 26
#define ledRed 27
#define receptor1 35
#define receptor2 34
#define buzzer 25
#define displaySDA 21
#define displaySCL 22
#define cartao 5
#define distancia 5.0
unsigned long tempoInicial = 0;
unsigned long tempoFinal = 0;
unsigned long intervalo = 0;
float velocidade = 0.0;
float recorde = 297.0;
bool primeiraBarreiraDetectada = false;
bool sistemaFuncionando = true;
bool cartaoFuncionando = false;
bool displayFuncionando = false;
const char* arquivo = "/velocidades.txt";
void detectarFalha();

bool detectarPrimeiraBarreira() {
  return digitalRead(receptor1) == LOW;
}

bool detectarSegundaBarreira() {
  return digitalRead(receptor2) == LOW;
}

void medirIntervalo() {
  intervalo = tempoFinal - tempoInicial;
}

void calcularVelocidade() {
  if (intervalo > 0) {
    velocidade = (distancia / (intervalo / 1000.0)) * 3.6;
  }
}

bool atualizarRecorde() {
  if (velocidade > recorde) {
    recorde = velocidade;
    return true;
  }
  return false;
}

void sinalizarRecorde() {
  tone(buzzer, 2000, 200);
}

void atualizarDisplay() {
  if (!displayFuncionando) {
    return;
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Velocidade: ");
  display.print(velocidade, 1);
  display.println(" km/h");
  display.setCursor(0, 20);
  display.println("RECORDE");
  display.print(recorde, 1);
  display.println(" km/h");
  display.display();
}

void lerCartao() {
  if (!cartaoFuncionando) {
    return;
  }

  File arquivoLeitura = SD.open(arquivo, FILE_READ);
  if (!arquivoLeitura) {
    cartaoFuncionando = false;
    detectarFalha();
    return;
  }
  while (arquivoLeitura.available()) {
    String linha = arquivoLeitura.readStringUntil('\n');
    if (linha.startsWith("Recorde: ")) {
      float recordeSalvo = linha.substring(9).toFloat();

      if (recordeSalvo > recorde) {
        recorde = recordeSalvo;
      }
    }
    Serial.println(linha);
  }
  arquivoLeitura.close();
}

void gravarDados(float velocidade, float recorde) {
  if (!cartaoFuncionando) {
    return;
  }

  File arquivoDados = SD.open(arquivo, FILE_APPEND);
  if (arquivoDados) {
    arquivoDados.print("Velocidade: ");
    arquivoDados.print(velocidade, 1);
    arquivoDados.println(" km/h");
    arquivoDados.print("Recorde: ");
    arquivoDados.print(recorde, 1);
    arquivoDados.println(" km/h");
    arquivoDados.close();
  } else {
    cartaoFuncionando = false;
    detectarFalha();
  }
}

void detectarFalha() {
  sistemaFuncionando = false;
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, LOW);
}

void setup() {
  Serial.begin(9600);
  pinMode(receptor1, INPUT);
  pinMode(receptor2, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, LOW);
  Wire.begin(displaySDA, displaySCL);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    displayFuncionando = false;
    detectarFalha();
  } else {
    displayFuncionando = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Sistema iniciado");
    display.display();
  }
  if (!SD.begin(cartao)) {
    cartaoFuncionando = false;
    detectarFalha();
  } else {
    cartaoFuncionando = true;
    if (!SD.exists(arquivo)) {
      File arquivoNovo = SD.open(arquivo, FILE_WRITE);

      if (arquivoNovo) {
        arquivoNovo.close();
      } else {
        cartaoFuncionando = false;
        detectarFalha();
      }
    }
  }
  
  lerCartao();
  if (displayFuncionando && cartaoFuncionando) {
    sistemaFuncionando = true;
    digitalWrite(ledGreen, HIGH);
    digitalWrite(ledRed, LOW);
    atualizarDisplay();
  } else {
    detectarFalha();
  }
}

void loop() {
  if (!sistemaFuncionando) {
    return;
  }
  if (!primeiraBarreiraDetectada && detectarPrimeiraBarreira()) {
    tempoInicial = millis();
    primeiraBarreiraDetectada = true;
  }
  if (primeiraBarreiraDetectada && detectarSegundaBarreira()) {
    tempoFinal = millis();
    medirIntervalo();
    calcularVelocidade();

    if (intervalo > 0) {
      if (atualizarRecorde()) {
        sinalizarRecorde();
      }
      atualizarDisplay();
      gravarDados(velocidade, recorde);
    }
    primeiraBarreiraDetectada = false;
  }
}
