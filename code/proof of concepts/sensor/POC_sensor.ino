const int sensor[] = {33, 32, 35, 34, 39, 36};
#define SerialPort Serial
#define Baudrate 115200
void setup() {
  // put your setup code here, to run once:
 SerialPort.begin(Baudrate);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 6; i++)
    {
      long value = analogRead(sensor[i]);
      Serial.println(value);
    }
    
    delay(2500);
}
