/*
Hosts an HTTP server over WIFI. WPA/WPA2 only.

Find the full UNO R4 WiFi Network documentation here:
https://docs.arduino.cc/tutorials/uno-r4-wifi/wifi-examples#simple-webserver

You can edit this in VSCode with AI and use the arduino IDE to upload it 
to the board. The IDE will live update the file.

*/
#include <OneWire.h>
#include <DallasTemperature.h>
#include <CircularBuffer.hpp>
#include "WiFiS3.h"
#include "Arduino_LED_Matrix.h"   // Include the LED_Matrix library

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

ArduinoLEDMatrix matrix;

// We use circular buffers to keep track of data moving averages.
CircularBuffer <float, 10> temperatureBufferF;             // Water Temperature
// CircularBuffer <float, 10> humidityBuffer;              // Humidity


unsigned long lastTempCheck = 0;  // Store the last time temperature was checked
const unsigned long tempCheckInterval = 10000;  // Interval to check temperature (10 seconds)
float targetTemperatureF = 93.5;


void setup() {
  Serial.begin(9600);
  sensors.begin();          // Start the DS18B20 sensor
  Serial.println("DS18B20 Temperature Sensor Ready");
  matrix.begin(); 

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
  matrix.loadFrame(LEDMATRIX_HEART_SMALL);
  unsigned long currentMillis = millis();
  
  // Check temperature every 10 seconds
  if (currentMillis - lastTempCheck >= tempCheckInterval) {
    lastTempCheck = currentMillis;
    sensors.requestTemperatures(); // Request temperature measurement
    float currentTemperatureF = sensors.getTempFByIndex(0);
    temperatureBufferF.push(currentTemperatureF);
    float myAverage = averageValue(temperatureBufferF);
    Serial.println("Temperature: " + String(myAverage) + "°F  ->  Target: " + String(targetTemperatureF) + "°F");
    Serial.println("Buffer Size: " + String(temperatureBufferF.size()));
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
            // TODO: do we need both of these two lines below?
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

    // Check for /setTemp endpoint
    // TODO: clean this up! Do we need to check for \r?
    int tempIndex = request.indexOf("GET /setTemp?value=");
    if (tempIndex >= 0) {
        int valueStart = tempIndex + 19; // Correct length of "GET /setTemp?value="
        int valueEnd = request.indexOf(' ', valueStart);
        if (valueEnd == -1) {
            valueEnd = request.indexOf('\r', valueStart); // Look for end of line if no space
        }
        if (valueEnd > valueStart) {
            String valueStr = request.substring(valueStart, valueEnd);
            Serial.println("Extracted valueStr: " + valueStr); // Debug print
            targetTemperatureF = valueStr.toFloat();
            Serial.println("New targetTemperatureF: " + String(targetTemperatureF));
        } else {
            Serial.println("Failed to find valueEnd");
        }
    } else {
        Serial.println("Failed to find /setTemp endpoint");
    }

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
        "    <p>Current->Target</p>\n"
        "    <p>" + String(averageValue(temperatureBufferF)) + "°F --> " + String(targetTemperatureF) + "°F</p>\n"
        "    <p>Last Temp Check: " + String(lastTempCheck) + "</p>\n"
        "    <form action=\"/setTemp\" method=\"get\">\n"
        "        <label for=\"value\">Set Temp Target:</label>\n"
        "        <input type=\"text\" id=\"value\" name=\"value\">\n"
        "        <input type=\"submit\" value=\"Set\">\n"
        "    </form>\n"
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


float averageValue(CircularBuffer<float, 10>& buffer) {
  if (buffer.size() == 0) return 0.0; // avoid division by zero
  float sum = 0.0;
  for (int i = 0; i < buffer.size(); i++) { sum += buffer[i]; }
  return sum / buffer.size();
}
