#include <WiFi.h>
#include <WiFiClient.h>
#include "StreamCommand.h"
#include "TelnetCommand.h"

#define SerialPort Serial
StreamCommand sCmd(SerialPort);
TelnetCommand tCmd(SerialPort);

const char *ssid = "HGDevice";
const char *password = "KillAllHuman$";

const byte ledPin = 23;

WiFiServer telnetServer(23);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Start Telnet server
  telnetServer.begin();
  Serial.println("Telnet server started on port 23");
  Serial.println(WiFi.localIP());

  tCmd.addCommand("on", LEDon);
  tCmd.addCommand("off", LEDoff);
  tCmd.setDefaultHandler(onUnknownCommand);

  pinMode(ledPin, OUTPUT);
}

void loop() {
  // Check for Telnet connection
  WiFiClient telnetClient = telnetServer.available();
  if (telnetClient) {
    Serial.println("Telnet client connected");

    while (telnetClient.connected()) {
      // Check if there is data available from Telnet
      if (telnetClient.available()) {
        tCmd.readStream(telnetClient);
        char data = telnetClient.read();
        
        // Forward the data to Arduino via Serial
        Serial.write(data);
      }

      // Check if there is data available from Arduino via Serial
      if (Serial.available()) {
        char data = Serial.read();
        // Forward the data to Telnet client
        telnetClient.write(data);
        telnetClient.write("/n");
      }
    }

    // Telnet client disconnected
    Serial.println("Telnet client disconnected");
    telnetClient.stop();

    
  }
}

void LEDon()
{
  digitalWrite(ledPin, HIGH);
  Serial.println("led is aan");
}
void LEDoff()
{
  digitalWrite(ledPin, LOW);
  Serial.println("Led is uit");
}


void onUnknownCommand(char *command)
{
  SerialPort.print("unknown command: \"");
  SerialPort.print(command);
  SerialPort.println("\"");
}