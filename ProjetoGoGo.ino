//Apresentar velocidade média e o recorde no display
//Registrar as velocidades medidas no cartão microSD.
//Recuperar os registros e o recorde armazenados após a reinicialização.
//Acender o LED vermelho quando forem detectadas falhas.
//Manter o LED verde aceso enquanto o sistema estiver funcionando corretamente.
//Detectar falhas nos sensores e no cartão microSD.


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
    velocidade = (distancia / (intervalo / 1000.0))*3.6;
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


void setup() {
  pinMode(receptor1, INPUT);
  pinMode(receptor2, INPUT);
}

void loop() {
  if (!primeiraBarreiraDetectada && detectarPrimeiraBarreira()) {
    tempoInicial = millis();
    primeiraBarreiraDetectada = true;
  }
  
  if (primeiraBarreiraDetectada && detectarSegundaBarreira()) {
    tempoFinal = millis();
    medirIntervalo();
    calcularVelocidade();
    if (atualizarRecorde()){
      sinalizarRecorde();
      primeiraBarreiraDetectada = false;
    }
  }
}
  
    