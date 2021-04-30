// PARA VERSION FINAL -> Cambiar tiempo de dia de 36 a 180 (3 minutos), y borrar o comentar los serial.print()

// Librerias
#include <Adafruit_NeoPixel.h>

// Estados
#define DIA  0
#define TRANS_TARDE  1
#define TARDE  2
#define TRANS_NOCHE1  3
#define NOCHE1  4
#define TRANS_NOCHE2  5
#define NOCHE2  6
#define TRANS_NOCHE3  7
#define AMANECER  8
#define NUM_ESTADOS 9
#define DIA_ESTATICO 10
#define TARDE_ESTATICO 11
#define NOCHE_ESTATICO 12

/////////////////////
// Configuraciones //
/////////////////////

// Entradas
#define BOTON1 5
#define BOTON2 4
#define BOTON3 3
#define BOTON4 2

// Colores
#define BRIGHTNESS 255          // 0 a 255
#define DIA_R 244
#define DIA_G 164
#define DIA_B 50
#define TARDE_R 255
#define TARDE_G 100
#define TARDE_B 0
#define NOCHE1_R 50
#define NOCHE1_G 50
#define NOCHE1_B 70
#define NOCHE2_R 5
#define NOCHE2_G 5
#define NOCHE2_B 7

// Tira Horizontal
#define NUM_LEDS 496
#define PIN_LEDS 6

// Tira Vertical
#define NUM_LEDS2 113
#define PIN_LEDS2 7
#define NUM_VERTICAL_1  45
#define NUM_VERTICAL_2  68
#define LED_VERTICAL_1  1
#define LED_VERTICAL_2  135

// Tira Vertical 2
#define NUM_LEDS3 152
#define PIN_LEDS3 1
#define NUM_VERTICAL_3  69
#define NUM_VERTICAL_4  83
#define LED_VERTICAL_3  177
#define LED_VERTICAL_4  258

// Tiempos
#define VEL_TRANS 1             // Velocidad transicion (solo numeros enteros)
#define INTERVALO 100           // Expresado en milisegundos
#define TIEMPO_DIA 36           // Expresados en segundos
#define TIEMPO_TRANS_TARDE 30
#define TIEMPO_TARDE 60
#define TIEMPO_TRANS_NOCHE1 30
#define TIEMPO_NOCHE1 60
#define TIEMPO_TRANS_NOCHE2 30
#define TIEMPO_NOCHE2 60
#define TIEMPO_TRANS_NOCHE3 30
#define TIEMPO_AMANECER 60

/////////////////////
//   Fin config    //
/////////////////////

// Variable de estado
uint8_t estado = DIA;

// Variables globales
unsigned long previousMillis = 0;
const long interval = INTERVALO;
uint8_t r [NUM_LEDS];
uint8_t g [NUM_LEDS];
uint8_t b [NUM_LEDS];
uint8_t auxR, auxG, auxB;
int8_t dR, dG, dB;
uint16_t i = 0;
uint32_t count = 0;

// Funciones
void softTimer();
void irqTimer();
void mostrar();
void setColor(uint8_t red, uint8_t green, uint8_t blue, uint16_t tiempo);
void transition(uint8_t initR, uint8_t initG, uint8_t initB, uint8_t finalR, uint8_t finalG, uint8_t finalB, uint16_t tiempo);
void update_color(uint8_t initR, uint8_t initG, uint8_t initB, uint8_t finalR, uint8_t finalG, uint8_t finalB, uint16_t tiempo);
void wave();
void selectColor(uint8_t red, uint8_t green, uint8_t blue);
void tiraVertical();
void tiraVertical2();
void update_entradas();

Adafruit_NeoPixel tira = Adafruit_NeoPixel(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel tira2 = Adafruit_NeoPixel(NUM_LEDS2, PIN_LEDS2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel tira3 = Adafruit_NeoPixel(NUM_LEDS3, PIN_LEDS3, NEO_GRB + NEO_KHZ800);

void setup() {
  //Serial.begin(9600);
  tira.begin();
  tira.clear();
  tira.setBrightness(BRIGHTNESS);
  tira.show();
  tira2.begin();
  tira2.clear();
  tira2.setBrightness(BRIGHTNESS);
  tira2.show();
  tira3.begin();
  tira3.clear();
  tira3.setBrightness(BRIGHTNESS);
  tira3.show();
  delay(100);
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);
  pinMode(BOTON3, INPUT_PULLUP);
  pinMode(BOTON4, INPUT_PULLUP);
  selectColor(DIA_R, DIA_G, DIA_B);
}


void loop() {
  softTimer();
}


void softTimer () {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    irqTimer();
    tiraVertical();
    tiraVertical2();
    wave();
    update_entradas();
  }
}


void irqTimer() {
  switch (estado) {

    case DIA:
      setColor(DIA_R, DIA_G, DIA_B, TIEMPO_DIA);
      break;

    case TRANS_TARDE:
      transition(DIA_R, DIA_G, DIA_B, TARDE_R, TARDE_G, TARDE_B, TIEMPO_TRANS_TARDE);
      break;

    case TARDE:
      setColor(TARDE_R, TARDE_G, TARDE_B, TIEMPO_TARDE);
      break;

    case TRANS_NOCHE1:
      transition(TARDE_R, TARDE_G, TARDE_B, NOCHE1_R, NOCHE1_G, NOCHE1_B, TIEMPO_TRANS_NOCHE1);
      break;

    case NOCHE1:
      setColor(NOCHE1_R, NOCHE1_G, NOCHE1_B, TIEMPO_NOCHE1);
      break;

    case TRANS_NOCHE2:
      transition(NOCHE1_R, NOCHE1_G, NOCHE1_B, NOCHE2_R, NOCHE2_G, NOCHE2_B, TIEMPO_TRANS_NOCHE2);
      break;

    case NOCHE2:
      setColor(NOCHE2_R, NOCHE2_G, NOCHE2_B, TIEMPO_NOCHE2);
      break;

    case TRANS_NOCHE3:
      transition(NOCHE2_R, NOCHE2_G, NOCHE2_B, NOCHE1_R, NOCHE1_G, NOCHE1_B, TIEMPO_TRANS_NOCHE3);
      break;

    case AMANECER:
      transition(NOCHE1_R, NOCHE1_G, NOCHE1_B, DIA_R, DIA_G, DIA_B, TIEMPO_AMANECER);
      break;

    case DIA_ESTATICO:
      selectColor(DIA_R, DIA_G, DIA_B);
      break;

    case TARDE_ESTATICO:
      selectColor(TARDE_R, TARDE_G, TARDE_B);
      break;

    case NOCHE_ESTATICO:
      selectColor(NOCHE1_R/2, NOCHE1_G/2, NOCHE1_B/2);
      break;

    default:
      estado = DIA;
      break;
  }
  mostrar();
  //Serial.print(" Estado: ");
  //Serial.println(estado);
}


void mostrar() {
  uint16_t i = 0;
  while (i < NUM_LEDS) {
    tira.setPixelColor(i, r[i], g[i], b[i]);
    i++;
  }
  i = 0;
  tira.show();
}


void setColor(uint8_t sR, uint8_t sG, uint8_t sB, uint16_t tiempo) {
  count++;
  //Serial.print("CONTADOR: ");
  //Serial.print(count);
  if (count == tiempo * 10) {
    count = 0;
    estado++;
    estado %= NUM_ESTADOS;
    auxR = 0;
    auxG = 0;
    auxB = 0;
  }
  r[0] = sR;
  g[0] = sG;
  b[0] = sB;
  /*i = 0;
    while(i < NUM_LEDS) {
    r[i] = sR;
    g[i] = sG;
    b[i] = sB;
    i++;
    }*/
}


void transition(uint8_t iR, uint8_t iG, uint8_t iB, uint8_t fR, uint8_t fG, uint8_t fB, uint16_t tiempo) {
  count++;
  //Serial.print(" CONTADOR: ");
  //Serial.print(count);
  update_color(iR, iG, iB, fR, fG, fB, tiempo);
  if (count == tiempo * 10) {
    count = 0;
    estado++;
    estado %= NUM_ESTADOS;
  }/*
  i = NUM_LEDS-1;
  while(i) {
    r[i] = r[i-1];
    g[i] = g[i-1];
    b[i] = b[i-1];
    i--;
  }*/
}

uint8_t z = 0;
void wave() {
  z = 0;
  while (z < VEL_TRANS) {
    i = NUM_LEDS - 1;
    while (i) {
      r[i] = r[i - 1];
      g[i] = g[i - 1];
      b[i] = b[i - 1];
      i--;
    }
    z++;
  }/*
  Serial.print(" AUXR: ");
  Serial.print(auxR);
  Serial.print(" AUXG: ");
  Serial.print(auxG);
  Serial.print(" AUXB: ");
  Serial.print(auxB);
  Serial.print(" dR: ");
  Serial.print(dR);
  Serial.print(" dG: ");
  Serial.print(dG);
  Serial.print(" dB: ");
  Serial.print(dB);
  Serial.print(" r0: ");
  Serial.print(r[0]);
  Serial.print(" g0: ");
  Serial.print(g[0]);
  Serial.print(" b0: ");
  Serial.print(b[0]);
  Serial.print(" rN: ");
  Serial.print(r[NUM_LEDS - 1]);
  Serial.print(" gN: ");
  Serial.print(g[NUM_LEDS - 1]);
  Serial.print(" bN: ");
  Serial.print(b[NUM_LEDS - 1]);*/
}


void update_color(uint8_t iR, uint8_t iG, uint8_t iB, uint8_t fR, uint8_t fG, uint8_t fB, uint16_t tiempo) {

  if (fR - iR > 0) dR = (int8_t)((tiempo * 10) / (fR - iR));
  else dR = (int8_t)((tiempo * 10) / (iR - fR));

  if (fG - iG > 0) dG = (int8_t)((tiempo * 10) / (fG - iG));
  else dG = (int8_t)((tiempo * 10) / (iG - fG));

  if (fB - iB > 0) dB = (int8_t)((tiempo * 10) / (fB - iB));
  else dB = (int8_t)((tiempo * 10) / (iB - fB));

  if (auxR == dR - 1) {
    auxR = 0;
    if (r[0] > fR) r[0]--;
    else if (r[0] < fR) r[0]++;
  }
  else {
    auxR++;
  }

  if (auxG == dG - 1) {
    auxG = 0;
    if (g[0] > fG) g[0]--;
    else if (g[0] < fG) g[0]++;
  }
  else {
    auxG++;
  }

  if (auxB == dB - 1) {
    auxB = 0;
    if (b[0] > fB) b[0]--;
    else if (b[0] < fB) b[0]++;
  }
  else {
    auxB++;
  }
}

void selectColor(uint8_t red, uint8_t green, uint8_t blue) {
  i = 0;
  while (i < NUM_LEDS) {
    r[i] = red;
    g[i] = green;
    b[i] = blue;
    i++;
  }
}


// Tira Vertical
uint16_t k = 0;
void tiraVertical() {
  k = 0;
  while (k < NUM_VERTICAL_1) {
    tira2.setPixelColor(k, r[LED_VERTICAL_1], g[LED_VERTICAL_1], b[LED_VERTICAL_1]);
    k++;
  }
  while (k < (NUM_VERTICAL_1 + NUM_VERTICAL_2)) {
    tira2.setPixelColor(k, r[LED_VERTICAL_2], g[LED_VERTICAL_2], b[LED_VERTICAL_2]);
    k++;
  }
  tira2.show();
}

// Tira Vertical 2
void tiraVertical2() {
  k = 0;
  while (k < (NUM_VERTICAL_3)) {
    tira3.setPixelColor(k, r[LED_VERTICAL_3], g[LED_VERTICAL_3], b[LED_VERTICAL_3]);
    k++;
  }
  while (k < (NUM_VERTICAL_3 + NUM_VERTICAL_4)) {
    tira3.setPixelColor(k, r[LED_VERTICAL_4], g[LED_VERTICAL_4], b[LED_VERTICAL_4]);
    k++;
  }
  tira3.show();
}


uint8_t antBot1 = 0;
uint8_t antBot2 = 0;
uint8_t antBot3 = 0;
uint8_t antBot4 = 0;
void update_entradas() {
  if (!digitalRead(BOTON1) & !antBot1) {
    antBot1=1;
    estado=DIA;
    count=0;
    auxR=0;
    auxG=0;
    auxB=0;
    selectColor(DIA_R, DIA_G, DIA_B);
    delay(20);
  }
  else if (digitalRead(BOTON1) & antBot1)  antBot1=0;

  if (!digitalRead(BOTON2) & !antBot2) {
    antBot2=1;
    estado=DIA_ESTATICO;
    count=0;
    delay(20);
  }
  else if (digitalRead(BOTON2) & antBot2) antBot2=0;

  if (!digitalRead(BOTON3) & !antBot3) {
    antBot3=1;
    estado=TARDE_ESTATICO;
    count=0;
    delay(20);
  }
  else if (digitalRead(BOTON3) & antBot3) antBot3=0;

  if (!digitalRead(BOTON4) & !antBot4) {
    antBot4=1;
    estado=NOCHE_ESTATICO;
    count=0;
    delay(20);
  }
  else if (digitalRead(BOTON4) & antBot4) antBot4=0;
/*
  Serial.print(" B1: ");
  Serial.print(digitalRead(BOTON1));
  Serial.print(" B2: ");
  Serial.print(digitalRead(BOTON2));
  Serial.print(" B3: ");
  Serial.print(digitalRead(BOTON3));
  Serial.print(" B4: ");
  Serial.print(digitalRead(BOTON4));*/
}
