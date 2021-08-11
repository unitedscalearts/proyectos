/*  
 *  Programa: Compresor de aire
 *  Version: 2.1
 *  
 *  Descripcion: Control de compresor con corte por funcionamiento continuo o mantenimiento.
 *  Interfaz con LCD 16x2 y uso de memoria SD.
 *  
 */

#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

File myFile;
uint32_t data = 0;

 /* Memoria SD
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 */

const int rs = 10, en = 9, d4 = 8, d5 = 7, d6 = 6, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Entradas
#define BUTTON_SERVICE      A3
#define SERVICE_PRESIONADO  digitalRead(BUTTON_SERVICE)
#define LLAVE               A1
#define LLAVE_ON            digitalRead(LLAVE)
#define LLAVE_OFF           !digitalRead(LLAVE)
#define BUTTON_RESET        A2
#define RESET_PRESIONADO    digitalRead(BUTTON_RESET)
#define S_MOTOR             A0
#define MOTOR_ANDANDO       !digitalRead(S_MOTOR)

// Salidas
#define BACKLIGHT       3
#define CONTACTOR       2
#define LED_OFF         1
#define LED_ON          0

// Tiempos
#define STOP_TIMEOUT    6000    //60000 (10 min) (1 - 10ms) Tiempo maximo en funcionamiento continuo
#define SERVICE_TIMEOUT 10800  //1080000 (30 hs) (1 - 10ms) Tiempo maximo de funcionamiento

// Variables de estado
#define INICIO          0
#define REPOSO          1
#define STOP            2
#define OFF             3
#define SERVICE         4

// Variables Globales
uint8_t estado = INICIO;
volatile boolean timer_flag = false;
uint32_t timer_count = 0;
boolean timer_display_flag = true;
uint32_t timer_display_count = 0;
unsigned long previousMillis = 0;
const long interval = 10;
#define DISPLAY_REFRESH 1000/interval
boolean service_flag = false;
uint32_t service_delay = 0;

void timer2_init();

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Iniciando... ");
  pinMode(BUTTON_SERVICE, INPUT);
  pinMode(LLAVE, INPUT);
  pinMode(BUTTON_RESET, INPUT);
  pinMode(S_MOTOR, INPUT);
  pinMode(LED_OFF, OUTPUT);
  pinMode(LED_ON, OUTPUT);
  pinMode(CONTACTOR, OUTPUT);
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);
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
    timer_display_count++;
    if (timer_display_count == DISPLAY_REFRESH) {
      timer_display_count = 0;
      timer_display_flag = true;
    }
    if(service_flag) {
      if(service_delay) service_delay--;
    }
  }
}


void update_estado() {
  static uint16_t ledCount = 0;
  static uint32_t motorCount = 0;
  static uint32_t motorServiceCount = 0;
  if (!timer_flag) return;
  timer_flag = false;
  
  switch(estado) {

    case INICIO:
      motorCount = 0;
      estado = REPOSO;
      lcd.clear();
      break;

    // Compresor andando normalmente, contando el tiempo de service y controlando que no supere el tiempo maximo de funcionamiento continuo
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
      if(!timer_display_flag) break;
      timer_display_flag = false;
      lcd.setCursor(0, 0);
      lcd.print("Motor ");
      if(!motorCount) lcd.print("OFF");
      else {
        lcd.print("ON ");
        lcd.print((uint32_t)(motorCount/6000));
        lcd.print("m ");
        lcd.print((uint32_t)((motorCount%6000)/100));
        lcd.print("s");
      }
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("Serv ");
      lcd.print((uint32_t)((SERVICE_TIMEOUT-motorServiceCount)/360000));
      lcd.print("h ");
      lcd.print((uint32_t)(((SERVICE_TIMEOUT-motorServiceCount)%360000)/6000));
      lcd.print("m ");
      lcd.print((uint32_t)(((SERVICE_TIMEOUT-motorServiceCount)%6000)/100));
      lcd.print("s");
      lcd.print("                ");
      break;

   // Si el compresor supero el tiempo maximo de funcionamiento continuo
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
      lcd.setCursor(0, 0);
      lcd.print("STOP, precaucion");
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("Esperando reset!");
      lcd.print("                ");
      break;

   // Si el compresor supero el tiempo maximo de funcionamiento
   case SERVICE:
      static boolean text_flash = false;
      digitalWrite(CONTACTOR, LOW);
      ledCount++;
      if(ledCount == 50) {
        ledCount = 0;
        digitalWrite(LED_OFF, !digitalRead(LED_OFF));
        digitalWrite(LED_ON, !digitalRead(LED_ON));
        text_flash = !text_flash;
      }
      if (LLAVE_OFF) estado = OFF;
      if(text_flash) {
        digitalWrite(BACKLIGHT, LOW);
        lcd.clear();
        break;
      }
      digitalWrite(BACKLIGHT, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("NECESITA SERVICE");
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("Call Mechatronic");
      lcd.print("                ");
      break;

   // Si la llave esta apagada
   case OFF:
      //digitalWrite(BACKLIGHT, LOW);
      analogWrite(BACKLIGHT,10);
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(LED_OFF, HIGH);
      digitalWrite(LED_ON, LOW);
      if(LLAVE_ON) {
        motorCount = 0;
        digitalWrite(BACKLIGHT, HIGH);
        estado = REPOSO;
      }
      lcd.setCursor(0, 0);
      lcd.print("APAGADO");
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      break;

   default:
      estado = INICIO;
      break;
  }

  // Debounce con delay de boton de servicio
  if(SERVICE_PRESIONADO && !service_flag) {
    service_flag = true;
    service_delay = 2000/interval;
  }
  else if (SERVICE_PRESIONADO && service_flag && !service_delay) {
    estado = INICIO;
    motorCount = 0;
    motorServiceCount = 0;
    digitalWrite(BACKLIGHT, HIGH);
    digitalWrite(CONTACTOR, LOW);
    digitalWrite(LED_OFF, HIGH);
    digitalWrite(LED_ON, HIGH);
  }
  if (!SERVICE_PRESIONADO && service_flag) {
    service_flag = false;
  }
}
