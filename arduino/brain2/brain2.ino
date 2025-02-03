/*
Hosts an HTTP server over WIFI. WPA/WPA2 only.

Find the full UNO R4 WiFi Network documentation here:
https://docs.arduino.cc/tutorials/uno-r4-wifi/wifi-examples#simple-webserver

You can edit this in VSCode with AI and use the arduino IDE to upload it 
to the board. The IDE will live update the file.

*/

#include "WiFiS3.h"

#include "arduino_secrets.h"


char ssid[] = SECRET_SSID;        // network SSID
char pass[] = SECRET_PASS;        // network password

int led =  LED_BUILTIN;
int status = WL_IDLE_STATUS;
WiFiServer server(80);



// Data wire of DS18B20 Temperature Sensor (digital pin 2)
// #define ONE_WIRE_BUS 2

// Create a OneWire instance to communicate with any OneWire device
// OneWire oneWire(ONE_WIRE_BUS);

// Pass the oneWire reference to Dallas Temperature.
// DallasTemperature sensors(&oneWire);



void setup() {
  Serial.begin(9600);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network.
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  printWifiStatus();

}


void loop() {

  // Allow a client to connect
  WiFiClient client = server.available();
  if (!client) return;

  Serial.println("new client");
  
  // Get the client request and read into memory
  String request = readHttpRequest(client);
  if (request.length() == 0) {
    client.stop();
    Serial.println("Client disconnected (empty request).");
    return;
  }

  Serial.println("Client Request:\n" + request);


  handleRequest(client, request);

  client.stop();
  Serial.println("Client disconnected.");
}


// Reads the full HTTP request into a string
String readHttpRequest(WiFiClient& client) {
    String request = "";
    unsigned long timeout = millis() + 1000;  // Timeout after 1 second

    while (client.connected() && millis() < timeout) {
        while (client.available()) {
            char c = client.read();
            request += c;
            timeout = millis() + 1000;  // Reset timeout on new data
            if (request.endsWith("\r\n\r\n")) break;  // End of HTTP headers
        }
    }
    return request;
}


// Processes HTTP requests
void handleRequest(WiFiClient& client, String& request) {
    String response = "HTTP/1.1 200 OK\r\nContent-type:text/html\r\n\r\n";

    if (request.indexOf("GET /H") >= 0) {
        digitalWrite(LED_BUILTIN, HIGH);
        response += "<p style=\"font-size:7vw;\">LED is <b>ON</b></p>";
    } else if (request.indexOf("GET /L") >= 0) {
        digitalWrite(LED_BUILTIN, LOW);
        response += "<p style=\"font-size:7vw;\">LED is <b>OFF</b></p>";
    } else {
        response += "<p style=\"font-size:7vw;\">Invalid Request</p>";
    }

    response += "<p style=\"font-size:7vw;\"><a href=\"/H\">Turn LED ON</a></p>";
    response += "<p style=\"font-size:7vw;\"><a href=\"/L\">Turn LED OFF</a></p>";

    client.print(response);
}


// Prints WiFi SSID, IP address, and signal strength
void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}