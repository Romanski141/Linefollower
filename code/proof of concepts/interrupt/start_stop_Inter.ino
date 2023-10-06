const byte ledPin = 13;
const byte interruptPin = 2;
volatile byte state = LOW;
int counter = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, FALLING);
  Serial.begin(9600);
}

void loop() {
  digitalWrite(ledPin, state);
}

void blink() {
  state = 1;
  Serial.println(counter);
  delay (100);
  counter = counter + 1;

  if (counter == 2)
    {
      counter = 0;
      state = 0;
    }
}