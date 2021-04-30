// 60 Hz --> 16,67ms (17ms)
// 50 Hz --> 20ms

#define PERIODO 170  // 60Hz

#define TIEMPO_PROGRAMA 1000

#define BOTON 2
#define LED 10
#define GATE 13
#define DEBOUNCE_TIME 150
#define ledPin 13

#define ENCENDER_LUCES digitalWrite(LED, HIGH)
#define ENCENDER_SONIDO digitalWrite(LED, HIGH)

// Estados
#define REPOSO 0
#define ARRANQUE 1
#define TIEMPO 2
#define FRENO 3
#define MANUAL 4

// Variables Globales
uint8_t state = REPOSO;
uint8_t next_state = REPOSO;
boolean stateMotor = false;
boolean stateLights = false;
uint8_t stateSound = 0;

String code = "";

volatile boolean timer_flag = 0;
long startTime = 0;
volatile long pwmTime = 0;
uint16_t duty = 100;

// Funciones
void setupTimer1();
void timer1_init();
void arranque();
void tiempo();
void freno();
void custom();

void setup() {
  pinMode(GATE, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BOTON, INPUT_PULLUP);
  //attachInterrupt(0, zeroCrossingInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(BOTON), motor, FALLING);
  Serial.begin(9600);
  //setupTimer1();
  timer1_init();
}

void loop() {
  if (millis() - pwmTime > ((uint8_t) duty*PERIODO/100)) {
   digitalWrite(LED, LOW);
  }
  update_state();
}

void update_state () {
  if (timer_flag) {
    timer_flag = false;
    switch(state) {
      case REPOSO:
        break;
      case ARRANQUE:
        arranque();
        break;
      case TIEMPO:
        tiempo();      
        break;
      case FRENO:
        freno();
        break;
      case MANUAL:
        manual();
        break;
      default:
        state = REPOSO;
        break;
    }
  }
}

void update_bluetooth () {
  if (code == "mh");
  else if (code == "ml");
  else if (code == "lh");
  else if (code == "ll");
  else if (code == "on");
  else if (code == "dem");
  else if (code == "s1");
  else if (code == "s2");
  else if (code == "s3");
  else code = "";
}

void arranque () {
  static uint16_t count = 0;
  if (state != next_state) {
    state = next_state;
    duty = 0;
    count = 0;
    return;
  }
  if (duty == 255) {
    next_state = TIEMPO;
  }
  duty++;
  ENCENDER_LUCES;
  ENCENDER_SONIDO;
}

void tiempo() {
  static uint16_t count = 0;
  if (state != next_state) {
    state = next_state;
    count = 0;
    return;
  }
  if (count == TIEMPO_PROGRAMA) {
    next_state = FRENO;
  }
  count++;
}

void freno () {
  static uint16_t count = 0;
  if (state != next_state) {
    state = next_state;
    duty = 0;
    count = 0;
    return;
  }
  if (duty == 0) {
    next_state = REPOSO;
  }
  duty--;
}

void manual () {
  
}

void motor () {
  // DEBOUNCE PARA PULSADORES
  if (millis() - startTime > DEBOUNCE_TIME) {
    startTime = millis();
    pwmTime = millis();
    digitalWrite(LED, HIGH);
    delayMicroseconds(500000);
    digitalWrite(LED, LOW);
    delayMicroseconds(10);
  }
  // Entrada Digtal
  /*
   * pwmTime = millis();
   * digitalWrite(LED, HIGH);
   * 
   */
}

/*
void setupTimer1() {
  noInterrupts();
  // Clear registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // 1 Hz (16000000/((15624+1)*1024))
  OCR1A = 15624;
  // CTC
  TCCR1B |= (1 << WGM12);
  // Prescaler 1024
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // Output Compare Match A Interrupt Enable
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

ISR(TIMER1_COMPA_vect) {
  digitalWrite(ledPin, digitalRead(ledPin) ^ 1);
}*/

/////////////////
/////////////////
/////////////////

void timer1_init() {
  // set up Timer1
  OCR1A = 100;                        // initialize the comparator
  TIMSK1 = 0x03;                      // enable comparator A and overflow interrupts
  TCCR1A = 0x00;                      // timer control registers set for
  TCCR1B = 0x00;                      // normal operation, timer disabled

}

bool startflag = false;             // flag for motor start delay
int dimming = 540;                  // this should be the same as maxoutputlimit
#define PULSE 2            // number of triac trigger pulse width counts. One count is 16 microseconds

// Interrupt Service Routines
void zeroCrossingInterrupt() { // zero cross detect
  TCCR1B = 0x04;               // start timer with divide by 256 input
  TCNT1 = 0;                   // reset timer - count from zero
  OCR1A = dimming;             // set the compare register brightness desired.
}


ISR(TIMER1_COMPA_vect) {       // comparator match
  if (startflag == true) {     // flag for start up delay
    digitalWrite(GATE, HIGH);  // set TRIAC gate to high
    TCNT1 = 65536 - PULSE;     // trigger pulse width
  }
}

ISR(TIMER1_OVF_vect) {         // timer1 overflow
  digitalWrite(GATE, LOW);     // turn off TRIAC gate
  TCCR1B = 0x00;               // disable timer stops unintended triggers
}
