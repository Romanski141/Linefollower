#define MotorLB 19
#define MotorLF 18
#define MotorRF 17
#define MotorRB 16

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
 for (int i = 0; i < 255; i++)
 {
  analogWrite(MotorLF, i);
  delay(50);
 }
 analogWrite(MotorLF, 0);
  delay(1000);
  for (int i = 0; i < 255; i++)
 {
  analogWrite(MotorLB, i);
  delay(50);
 }
 analogWrite(MotorLB, 0);
 delay(1000);
 for (int i = 0; i < 255; i++)
 {
  analogWrite(MotorRF, i);
  delay(50);
 }
 analogWrite(MotorRF, 0);
 delay(1000);
 for (int i = 0; i < 255; i++)
 {
  analogWrite(MotorRB, i);
  delay(50);
 }
 analogWrite(MotorRB, 0);
delay(10000);
}
