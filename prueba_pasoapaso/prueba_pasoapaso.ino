
#define OUT1 11
#define OUT2 10
#define OUT3 9
#define OUT4 6

void setup() {
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);

}

void loop() {
  motorStep();
  delay(1000);
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
