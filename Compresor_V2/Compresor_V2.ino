/*  
 *  Programa: Compresor de aire
 *  Version: 2.0
 *  
 *  Descripcion: Control de compresor con corte por funcionamiento continuo o mantenimiento.
 *  Interfaz con LCD 16x2 y uso de memoria SD.
 *  
 */

// Entradas
#define LLAVE             9
#define LLAVE_ON          digitalRead(LLAVE)
#define LLAVE_OFF         !digitalRead(LLAVE)
#define BUTTON_RESET      8
#define RESET_PRESIONADO  digitalRead(BUTTON_RESET)
#define S_MOTOR           7
#define MOTOR_ANDANDO     !digitalRead(S_MOTOR)
//#define BUTTON_SERVICE        // Activo Alto?
//#define SERVICE_PRESIONADO  digitalRead(BUTTON_SERVICE)

// Salidas
#define LED_OFF         12
#define LED_ON          11
#define CONTACTOR       10

// Tiempos
#define STOP_TIMEOUT    60000    // (10 min) (1 - 10ms) Tiempo maximo en funcionamiento continuo
#define SERVICE_TIMEOUT 1080000  // (30 hs) (1 - 10ms) Tiempo maximo de funcionamiento

// Variables de estado
#define INICIO          0
#define REPOSO          1
#define STOP            2
#define OFF             3
#define SERVICE         4

// Variables Globales
uint8_t estado = REPOSO;
boolean timer_flag = false;
unsigned long previousMillis = 0;
const long interval = 10;

void setup() {
  pinMode(LLAVE, INPUT);
  pinMode(BUTTON_RESET, INPUT);
  pinMode(S_MOTOR, INPUT);
  pinMode(LED_OFF, OUTPUT);
  pinMode(LED_ON, OUTPUT);
  pinMode(CONTACTOR, OUTPUT);
}


void loop() {
  softTimer();
  update_estado();
}


void softTimer() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    timer_flag = true;
  }
}


uint32_t motorServiceCount = 0;

void update_estado() {
  static uint16_t ledCount = 0;
  static uint32_t motorCount = 0;
  if (!timer_flag) return;
  timer_flag = true;
  
  switch(estado) {

    case INICIO:
      motorCount = 0;
      estado = REPOSO;
      break;

    case REPOSO:
      if (LLAVE_OFF) estado = OFF;
      else if (motorServiceCount >= SERVICE_TIMEOUT) estado = SERVICE;
      else if (motorCount >= STOP_TIMEOUT) estado = STOP;
      else {
        if (MOTOR_ANDANDO) {
          motorCount++;
          motorServiceCount++;
        }
        else motorCount = 0;
        digitalWrite(CONTACTOR, HIGH);
        digitalWrite(LED_OFF, LOW);
        digitalWrite(LED_ON, HIGH);
      }
      break;

   case STOP:
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(LED_OFF, LOW);
      ledCount++;
      if(ledCount == 50) {
        ledCount = 0;
        digitalWrite(LED_ON, !digitalRead(LED_ON));
      }
      if (LLAVE_OFF) estado = OFF;
      else if (RESET_PRESIONADO) {
        motorCount = 0;
        estado = REPOSO;
      }
      break;

   case SERVICE:
      digitalWrite(CONTACTOR, LOW);
      ledCount++;
      if(ledCount == 50) {
        ledCount = 0;
        digitalWrite(LED_OFF, !digitalRead(LED_OFF));
        digitalWrite(LED_ON, !digitalRead(LED_ON));
      }
      if (LLAVE_OFF) estado = OFF;
      // falta cambiar de estado
      break;

   case OFF:
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(LED_OFF, HIGH);
      digitalWrite(LED_ON, LOW);
      if(LLAVE_ON) {
        motorCount = 0;
        estado = REPOSO;
      }
      break;

   default:
      estado = INICIO;
      break;
  }
}
