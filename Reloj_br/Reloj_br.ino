/*
 * Notas: 1h reloj -> 4,096s tiempo real (Velocidad MAXIMA)
 * 
 * min -> 2048 pasos/vuelta
 * h -> 1024 pasos/vuelta
 * 2ms/paso -> 1h reloj = 4,096 seg tiempo real
 * 24h reloj = 98,304 seg = 1m 38,304seg (aprox) (Velocidad MAXIMA)
 */

// Configuraciones
#define CLOCK_DEL   8     // Delay del reloj 1 -> 98,3 seg/dia, 2 -> 196,6 seg/dia (aprox)

// Entradas
#define SENSOR_HOUR 12
#define SENSOR_MIN 7

// Salidas
// Horas
#define OUT1 11   // IN1
#define OUT2 10   // IN2
#define OUT3 9    // IN3
#define OUT4 8    // IN4
// Minutos
#define OUT5 6   // IN1
#define OUT6 5   // IN2
#define OUT7 4   // IN3
#define OUT8 3   // IN4

// Control paso a paso
#define CLOCK 0
#define COUNTER_CLOCK 1
#define HOUR 0
#define MINUTE 1

// Variables Globales
uint32_t timer2_count = 0;
uint32_t step_del = 2;    // 1 -> 1ms
boolean timer2_flag = false;

void pin_init();
void timer2_init();
void motorStep(boolean motorSel, boolean motorDir, uint32_t clock_del);

void setup() {
  pin_init();
  timer2_init();
  Serial.begin(9600);
  Serial.println("Comienzo..");

/*
  // Buscando el punto 0 v2.0 (eliminando la histeresis del sensor)

  // 1 - Salgo del rango del sensor
  while(digitalRead(SENSOR_MIN) || digitalRead(SENSOR_HOUR)) { 
    if(timer2_flag) {
      if(digitalRead(SENSOR_MIN)) {
        motorStep(MIN,COUNTER_CLOCK,1);
      }
      if(digitalRead(SENSOR_HOUR)) {
        motorStep(HOUR,COUNTER_CLOCK,1);
      }
      timer2_flag = false;
    }
  }

  // 2 - Busco nuevamente el sensor
  while(!digitalRead(SENSOR_MIN) || !digitalRead(SENSOR_HOUR)) {
    if(timer2_flag) {
      if(!digitalRead(SENSOR_MIN)) {
        motorStep(MIN,CLOCK,1);
      }
      if(!digitalRead(SENSOR_HOUR)) {
        motorStep(HOUR,CLOCK,1);
      }
      timer2_flag = false;
    }
  }

  // 3 - Ready, informo
  Serial.println("Ready, en posicion 0");
*/
}

void loop() {
  if(timer2_flag) {
    timer2_flag = false;
    motorStep(HOUR,CLOCK,CLOCK_DEL*24);
    motorStep(MINUTE,CLOCK,CLOCK_DEL);
  }
}

const boolean fStep1 [4] = {HIGH, HIGH, LOW,  LOW};
const boolean fStep2 [4] = {HIGH, LOW,  LOW,  HIGH};
const boolean fStep3 [4] = {LOW,  LOW,  HIGH, HIGH};
const boolean fStep4 [4] = {LOW,  HIGH, HIGH, LOW};
uint8_t pos_hour = 0;
uint8_t pos_min = 0;

void motorStep(boolean motorSel, boolean dirSel, uint32_t clock_del) {
  static uint32_t clock_del_hour_count = 0;
  static uint32_t clock_del_min_count = 0;  
  if(motorSel == HOUR) {
    clock_del_hour_count++;
    if(clock_del_hour_count != clock_del) return;
    clock_del_hour_count = 0;
    if (dirSel == CLOCK) {
      pos_hour--;
      if(pos_hour > 4) pos_hour = 3;
    }
    else if (dirSel == COUNTER_CLOCK) {
      pos_hour++;
      pos_hour%=4;
    }
    digitalWrite(OUT1, fStep1[pos_hour]);
    digitalWrite(OUT2, fStep2[pos_hour]);
    digitalWrite(OUT3, fStep3[pos_hour]);
    digitalWrite(OUT4, fStep4[pos_hour]);
  }
  else if (motorSel == MINUTE) {
    clock_del_min_count++;
    if(clock_del_min_count != clock_del) return;
    clock_del_min_count = 0;
    if (dirSel == CLOCK) {
      pos_min++;
      pos_min%=4;
    }
    else if (dirSel == COUNTER_CLOCK) {
      pos_min--;
      if(pos_min > 4) pos_min = 3;
    }
    digitalWrite(OUT5, fStep1[pos_min]);
    digitalWrite(OUT6, fStep2[pos_min]);
    digitalWrite(OUT7, fStep3[pos_min]);
    digitalWrite(OUT8, fStep4[pos_min]);
  }
}


void pin_init() {
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
  pinMode(OUT5, OUTPUT);
  pinMode(OUT6, OUTPUT);
  pinMode(OUT7, OUTPUT);
  pinMode(OUT8, OUTPUT);
  pinMode(SENSOR_HOUR, INPUT);
  pinMode(SENSOR_MIN, INPUT);
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
