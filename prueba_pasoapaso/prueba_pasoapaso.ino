
#define OUT1 11
#define OUT2 10
#define OUT3 9
#define OUT4 6
#define SENSOR 12

void setup() {
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
  pinMode(SENSOR, INPUT);
  Serial.begin(9600);
}

boolean sensorFlag = false;
uint32_t count = 0;

void loop() {
  if(digitalRead(SENSOR) && !sensorFlag) {
    sensorFlag = true;
    if(count > 0) {
      Serial.print(" Pasos: ");
      Serial.println(count);
    }
    Serial.println("comienza cuenta");
    count = 0;
  }
  if(sensorFlag && !digitalRead(SENSOR)) {
    sensorFlag = false;
    Serial.println("No hay sensor");
  }
  count++;
  motorStep();
  delay(5);
}

boolean fStep1 [4] = {HIGH, HIGH, LOW, LOW};
boolean fStep2 [4] = {LOW, LOW, HIGH, HIGH};
boolean fStep3 [4] = {HIGH, LOW, LOW, HIGH};
boolean fStep4 [4] = {LOW, HIGH, HIGH, LOW};
uint8_t fPos = 0;
boolean hStep1 [8] = {HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, LOW};
boolean hStep2 [8] = {LOW, LOW, LOW, HIGH, HIGH, HIGH, LOW, HIGH};
boolean hStep3 [8] = {LOW, HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW};
boolean hStep4 [8] = {LOW, LOW, LOW, LOW, LOW, HIGH, HIGH, HIGH};

void motorStep() {
  digitalWrite(OUT1, fStep1[fPos]);
  digitalWrite(OUT2, fStep2[fPos]);
  digitalWrite(OUT3, fStep3[fPos]);
  digitalWrite(OUT4, fStep4[fPos]);
  fPos++;
  fPos%=4;
}
