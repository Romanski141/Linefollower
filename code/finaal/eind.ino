#include "SerialCommand.h"
#include <EEPROM.h>
#include <WiFi.h>
#include "StreamCommand.h"
#include "TelnetCommand.h"


#define SerialPort Serial
#define Baudrate 115200
#define PARAM_ADDRESS 0
#define MotorLB 19
#define MotorLF 18
#define MotorRF 17
#define MotorRB 16

long normalised[6];
float debugpos;
float position;
bool run;

StreamCommand sCmd(SerialPort);
TelnetCommand tCmd(SerialPort);
WiFiServer telnetServer(23);
WiFiClient telnetClient;

const char *ssid = "MSI 4139";
const char *password = "9=Y60g18";

bool debug;
unsigned long previous, calculationTime;
const int sensor[] = {33, 32, 35, 34, 39, 36};

struct Parameters{
  unsigned long cycleTime;
  int black[6];
  int white[6];
  int power;
  float diff;
  float kp;
  
  /* andere parameters die in het eeprom geheugen moeten opgeslagen worden voeg je hier toe ... */
} params;
Parameters storedParams;
 
  /* andere parameters die in het eeprom geheugen moeten opgeslagen worden voeg je hier toe ... */


void setup()
{
  SerialPort.begin(Baudrate);

  EEPROM.begin(sizeof(Parameters));
  
  EEPROM.get(PARAM_ADDRESS, storedParams);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Start Telnet server
  telnetServer.begin();
  Serial.println("Telnet server started on port 23");

  sCmd.addCommand("set", onSet);
  sCmd.addCommand("debug", onDebug);
  sCmd.addCommand("calibrate", onCalibrate);
  sCmd.addCommand("run", onRun);
  sCmd.addCommand("stop", onStop);
  sCmd.setDefaultHandler(onUnknownCommand);

  tCmd.addCommand("set", onSettelnet);
  tCmd.addCommand("debugtelnet", ondebugtelnet);
  tCmd.addCommand("calibrate", onCalibratetelnet);
  tCmd.addCommand("run", onRun);
  tCmd.addCommand("stop", onStop);
  tCmd.setDefaultHandler(onUnknownCommand);

  SerialPort.println("ready");

}

void loop()
{
  
  draadloos();
  sCmd.readStream();
 
  unsigned long current = micros();
  if (current - previous >= storedParams.cycleTime)
  {
    previous = current;

    for (int i = 0; i < 6; i++)
    {
      long value = analogRead(sensor[i]);
      normalised[i] = map(value, storedParams.black[i], storedParams.white[i], 0, 4096);
    }
    /* code die cyclisch moet uitgevoerd worden programmeer je hier ... */

    /* normaliseren en interpoleren sensor */
    int index = 0;
    for (int i = 1; i < 6; i++) if (normalised[i] < normalised[index]) index = i;

    if (normalised[index] > 3000) run = false;

    if (index == 0)
    {
      index = 1;
    } 
    else if (index == 5) 
    {
      index = 6;
    }
    else 
    {
      long sZero = normalised[index];
    long sMinusOne = normalised[index-1];
    long sPlusOne = normalised[index+1];

    float b = ((float) (sPlusOne - sMinusOne)) / 2;
    float a = sPlusOne - b - sZero;

    position = -b / (2 * a);  
    position += index - 2.5;   
    position *= 10;  //sensor distance in mm
    debugpos = position;
    }

    float error = -position; //error = setpoint - input
    float output = error * storedParams.kp;
    /* pid regeling */

    output = constrain(output, -400, 400);
    int powerleft = 0;
    int powerright = 0;

    if(run) if (output >= 0)
    {
      powerleft = constrain(storedParams.power + storedParams.diff * output, -200, 200);
      powerright = constrain(powerleft - output, -200, 200);
      powerleft = powerright + output;
    }
    else
    {
      powerright = constrain(storedParams.power - storedParams.diff * output, -200, 200);
      powerleft = constrain(powerright + output, -200, 200);
      powerright = powerleft - output;
    }
 
    analogWrite(MotorLF, powerleft > 0 ? powerleft : 0);
    analogWrite(MotorLB, powerleft < 0 ? -powerleft : 0);
    analogWrite(MotorRF, powerright > 0 ? powerright : 0);
    analogWrite(MotorRB, powerright < 0 ? -powerright : 0);
    
    /* aansturen motoren */


    
  }

  unsigned long difference = micros() - current;
  if (difference > calculationTime) calculationTime = difference;
}

void onUnknownCommand(char *command)
{
  SerialPort.print("unknown command: \"");
  SerialPort.print(command);
  SerialPort.println("\"");
}

void onSet()
{
  char* param = sCmd.next();
  char* value = sCmd.next();  
  
  if (strcmp(param, "cycle") == 0) storedParams.cycleTime = atol(value);
  else if (strcmp(param, "power") == 0) storedParams.power = atol(value);
  else if (strcmp(param, "diff") == 0) storedParams.diff = atof(value);
  else if (strcmp(param, "kp") == 0) storedParams.kp = atof(value);
  /* parameters een nieuwe waarde geven via het set commando doe je hier ... */
  

  EEPROM.put(PARAM_ADDRESS, storedParams);
  EEPROM.commit();
}

void onDebug()
{
  SerialPort.print("cycle time: ");
  SerialPort.println(storedParams.cycleTime);
  
  /* parameters weergeven met behulp van het debug commando doe je hier ... */
  
  SerialPort.print("calculation time: ");
  SerialPort.println(calculationTime);
  calculationTime = 0;

  SerialPort.println("black:");
  for (int i = 0; i < 6; i++)
  {
    SerialPort.println(storedParams.black[i]);
  }

  SerialPort.println("white:");
  for (int i = 0; i < 6; i++)
  {
    SerialPort.println(storedParams.white[i]);
  }

  SerialPort.println("genormaliseerd:");
  for (int i = 0; i < 6; i++)
  {
    SerialPort.println(normalised[i]);
  }

  SerialPort.println("positie:");
  SerialPort.println(debugpos);

  SerialPort.print("power: ");
  SerialPort.println(storedParams.power);

  SerialPort.print("diff: ");
  SerialPort.println(storedParams.diff);

  SerialPort.print("kp: ");
  SerialPort.println(storedParams.kp);


}
void onCalibrate()
{
  char* param = sCmd.next();
   

  if (strcmp(param, "black") == 0)
  {
    SerialPort.print("start calibrating black... ");
    for (int i = 0; i < 6; i++) 
    {
      storedParams.black[i]=analogRead(sensor[i]);
      SerialPort.println(analogRead(sensor[i]));
    }
    
    SerialPort.println("done");
  }
  else if (strcmp(param, "white") == 0)
  {
    SerialPort.print("start calibrating white... ");    
    for (int i = 0; i < 6; i++) storedParams.white[i]=analogRead(sensor[i]);  
    SerialPort.println("done");
        
  }
  EEPROM.put(PARAM_ADDRESS, storedParams);
  EEPROM.commit();  
  
}
void onRun()
{
  run = true;
}
void onStop()
{
  run = false;
}
void ondebugtelnet()
{
  telnetClient.print("cycle time: ");
  telnetClient.println(storedParams.cycleTime);
  
  /* parameters weergeven met behulp van het debug commando doe je hier ... */
  
  telnetClient.print("calculation time: ");
  telnetClient.println(calculationTime);
  calculationTime = 0;

  telnetClient.println("black:");
  for (int i = 0; i < 6; i++)
  {
    telnetClient.println(storedParams.black[i]);
  }

  telnetClient.println("white:");
  for (int i = 0; i < 6; i++)
  {
    telnetClient.println(storedParams.white[i]);
  }

  telnetClient.println("genormaliseerd:");
  for (int i = 0; i < 6; i++)
  {
    telnetClient.println(normalised[i]);
  }

  telnetClient.println("positie:");
  telnetClient.println(debugpos);

  telnetClient.print("power: ");
  telnetClient.println(storedParams.power);

  telnetClient.print("diff: ");
  telnetClient.println(storedParams.diff);

  telnetClient.print("kp: ");
  telnetClient.println(storedParams.kp);
  SerialPort.println("telnetcmd run debug done");



}
void draadloos() 
{
  if (telnetServer.hasClient()) 
  {
    if (!telnetClient || !telnetClient.connected()) 
    {
      if (telnetClient) {telnetClient.stop();}
      telnetClient = telnetServer.available();
      if (telnetClient) 
      {
        Serial.println("New Telnet client connected");
        telnetClient.println("T34-76 ready for action");
      }
    }
  }

  if (telnetClient && telnetClient.connected() && telnetClient.available()) 
  {
    tCmd.readStream(telnetClient);
  }
  if (telnetClient && !telnetClient.connected()) 
  {
    Serial.println("Telnet client disconnected");
    telnetClient.stop();
  }
}
void onCalibratetelnet()
{
  char* param = tCmd.next();
   

  if (strcmp(param, "black") == 0)
  {
    telnetClient.print("start calibrating black... ");
    for (int i = 0; i < 6; i++) 
    {
      storedParams.black[i]=analogRead(sensor[i]);
      telnetClient.println(analogRead(sensor[i]));
    }
    
    telnetClient.println("done");
  }
  else if (strcmp(param, "white") == 0)
  {
    telnetClient.print("start calibrating white... ");    
    for (int i = 0; i < 6; i++) storedParams.white[i]=analogRead(sensor[i]);  
    telnetClient.println("done");
        
  }
  EEPROM.put(PARAM_ADDRESS, storedParams);
  EEPROM.commit();  
  
}
void onSettelnet()
{
  char* param = tCmd.next();
  char* value = tCmd.next();  
  
  if (strcmp(param, "cycle") == 0) storedParams.cycleTime = atol(value);
  else if (strcmp(param, "power") == 0) storedParams.power = atol(value);
  else if (strcmp(param, "diff") == 0) storedParams.diff = atof(value);
  else if (strcmp(param, "kp") == 0) storedParams.kp = atof(value);
  /* parameters een nieuwe waarde geven via het set commando doe je hier ... */
  

  EEPROM.put(PARAM_ADDRESS, storedParams);
  EEPROM.commit();
}

