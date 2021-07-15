// Configuraciones
#define CLOCK_TIME  40    // segundos por vuelta (450 ~ 7m 30seg)
#define STEPS       2048

// Entradas
#define SENSOR 12

// Salidas
#define OUT1 11
#define OUT2 10
#define OUT3 9
#define OUT4 6

// Variables de Estado
#define CLOCK 1
#define SET 2

// Variables Globales
uint8_t state = CLOCK;
uint32_t timer2_count = 0;
uint32_t step_del = 5;    // 1 -> 1ms
uint32_t step_del_h = 5;
boolean timer2_flag = false;
//uint32_t steps_buff = 2048;
boolean sensor_flag = false;
uint32_t step_pos = 0;
boolean init_flag = true;

void pin_init();
void timer2_init();
void motorStep();
boolean motorSet( uint32_t set_pos);

void setup() {
  pin_init();
  timer2_init();
  Serial.begin(9600);
  Serial.println("Comienzo..");
  
  // Salgo del rango del sensor
  while(digitalRead(SENSOR)) { 
    if(timer2_flag) {
      //step_pos++;
      timer2_flag = false;
      motorStep();
    }    
  }
  //
  
  // Hasta que no lo detecte de nuevo no empiezo el programa
  while (init_flag) {
    if(digitalRead(SENSOR) && !sensor_flag) {
      sensor_flag = true;
      if(step_pos > 0) {
        Serial.print("Pos ant: ");
        Serial.print(step_pos);
        Serial.print(" ");
        Serial.println("pasos");
        init_flag = false;
      }
    }
    if(!digitalRead(SENSOR) && sensor_flag) {
      sensor_flag = false;
    }
    if(timer2_flag) {
      //step_pos++;
      timer2_flag = false;
      motorStep();
    }
  }
  //
  
  Serial.println("Ready, en posicion 0");
  step_del_h = (uint32_t) ((CLOCK_TIME*1000)/STEPS);
  step_del = step_del_h;

}

void loop() {
  if(timer2_flag) {
    timer2_flag = false;
    motorStep();
  }
}

const boolean fStep1 [4] = {HIGH, HIGH, LOW,  LOW};
const boolean fStep2 [4] = {LOW,  LOW,  HIGH, HIGH};
const boolean fStep3 [4] = {HIGH, LOW,  LOW,  HIGH};
const boolean fStep4 [4] = {LOW,  HIGH, HIGH, LOW};
uint8_t fPos = 0;

void motorStep() {
  digitalWrite(OUT1, fStep1[fPos]);
  digitalWrite(OUT2, fStep2[fPos]);
  digitalWrite(OUT3, fStep3[fPos]);
  digitalWrite(OUT4, fStep4[fPos]);
  step_pos++;
  fPos++;
  fPos%=4;
}
/*
boolean motorSet(uint32_t set_pos) {
  //step_del = 5;
  if(step_pos == set_pos) return true;
  digitalWrite(OUT1, fStep1[fPos]);
  digitalWrite(OUT2, fStep2[fPos]);
  digitalWrite(OUT3, fStep3[fPos]);
  digitalWrite(OUT4, fStep4[fPos]);
  step_pos++;
  fPos++;
  fPos%=4;
  return false;
}*/

void pin_init() {
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
  pinMode(SENSOR, INPUT);
}

void timer2_init() {
  SREG = (SREG & 0b01111111);              // Deshabilitar interrupciones globales
  TCNT2 = 0;                               // Limpiar el registro que guarda la cuenta del Timer-2.
  TIMSK2 = TIMSK2 | 0b00000001;            // Habilitación de la bandera 0 del registro que habilita la interrupción por sobre flujo o desbordamiento del TIMER2.
  TCCR2B = 0b00000011;                     // ft2 = 250Khz => 1ms ( (1/250.000) * 255 )
  SREG = (SREG & 0b01111111) | 0b10000000; // Habilitar interrupciones
}


ISR(TIMER2_OVF_vect) {
  timer2_count++;
  if (timer2_count >= step_del) {
    timer2_flag = true;
    timer2_count = 0;
  }
}
