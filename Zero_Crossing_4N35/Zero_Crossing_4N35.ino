
void zeroCrossingInterrupt();

void setup() {  
  //IRQ0 is pin 2. Call zeroCrossingInterrupt 
  attachInterrupt(0,zeroCrossingInterrupt, FALLING);    
  
  Serial.begin(9600);
}


void loop() {
  
}

void zeroCrossingInterrupt () {
  Serial.println("ZERO");
}
