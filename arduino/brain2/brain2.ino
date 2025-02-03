/*
Hosts an HTTP server over WIFI. WPA/WPA2 only.

Find the full UNO R4 WiFi Network documentation here:
https://docs.arduino.cc/tutorials/uno-r4-wifi/wifi-examples#simple-webserver

You can edit this in VSCode with AI and use the arduino IDE to upload it 
to the board. The IDE will live update the file.

*/
#include <OneWire.h>
#include <DallasTemperature.h>
#include "WiFiS3.h"

#include "arduino_secrets.h"


char ssid[] = SECRET_SSID;        // network SSID
char pass[] = SECRET_PASS;        // network password

int led =  LED_BUILTIN;
int status = WL_IDLE_STATUS;
WiFiServer server(80);



// Data wire of DS18B20 Temperature Sensor (digital pin 2)
#define ONE_WIRE_BUS 2

// Create a OneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);
// Pass the reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

unsigned long lastTempCheck = 0;  // Store the last time temperature was checked
const unsigned long tempCheckInterval = 10000;  // Interval to check temperature (10 seconds)
float tempTarget = 93.5;


void setup() {
  Serial.begin(9600);
  sensors.begin();          // Start the DS18B20 sensor
  Serial.println("DS18B20 Temperature Sensor Ready");

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
  unsigned long currentMillis = millis();
  
  // Check temperature every 10 seconds
  if (currentMillis - lastTempCheck >= tempCheckInterval) {
    lastTempCheck = currentMillis;
    sensors.requestTemperatures(); // Request temperature measurement
    float temperatureF = sensors.getTempFByIndex(0);
    Serial.println("Temperature: " + String(temperatureF) + "°F");
  }

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

  handleHttpResponse(client, request);

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


void handleHttpResponse(WiFiClient& client, String& request) {
    bool ledOn = request.indexOf("GET /H") >= 0;
    bool ledOff = request.indexOf("GET /L") >= 0;

    if (ledOn) digitalWrite(LED_BUILTIN, HIGH);
    if (ledOff) digitalWrite(LED_BUILTIN, LOW);

    String htmlResponse =
        "HTTP/1.1 200 OK\r\n"
        "Content-type: text/html\r\n"
        "\r\n"
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <title>ESP Web Server</title>\n"
        "    <style>\n"
        "        body { font-size: 3vw; text-align: center; font-family: Arial, sans-serif; }\n"
        "        a { display: block; padding: 20px; margin: 10px; background: #007BFF; color: white; text-decoration: none; border-radius: 10px; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>ESP Web Server</h1>\n"
        "    <p>LED is " + String(ledOn ? "ON" : "OFF") + "</p>\n"
        "    <a href=\"/H\">Turn LED ON</a>\n"
        "    <a href=\"/L\">Turn LED OFF</a>\n"
        "</body>\n"
        "</html>\n";

    client.print(htmlResponse);
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