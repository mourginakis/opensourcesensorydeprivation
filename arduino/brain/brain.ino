#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire of DS18B20 Temperature Sensor
// is connected to digital pin 2
#define ONE_WIRE_BUS 2

// Define the pin connected to the SSR
// Constrain temp to a range to reduce likeliness of always-on
float tempMin = 28.0;
float tempMax = 30.0;
const int ssrPin = 8;

// Create a OneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);

// Pass the oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


/*
Fahrenheit | Celsius
-----------|---------
 93.0      | 33.89
 93.1      | 33.94
 93.2      | 33.99
 93.3      | 34.06
 93.4      | 34.11
 93.5      | 34.17
 93.6      | 34.22
 93.7      | 34.28
 93.8      | 34.33
 93.9      | 34.39
 94.0      | 34.44

Note: Celsius = (Fahrenheit - 32) * 5 / 9
*/



void setup() {
  Serial.begin(9600);
  sensors.begin(); // Start the DS18B20 sensor
  Serial.println("DS18B20 Temperature Sensor Ready");
  pinMode(ssrPin, OUTPUT);
}

void loop() {
  sensors.requestTemperatures(); // Request temperature measurement
  
  // Read temperature in Celsius
  float temperatureC = sensors.getTempCByIndex(0);
  
  // Display the temperature
  Serial.println("Temperature: " + String(temperatureC) + " °C");

  if (tempMin < temperatureC && temperatureC < tempMax) {
    digitalWrite(ssrPin, HIGH);
  } else {
    digitalWrite(ssrPin, LOW);
  }
  
  delay(1000); // Wait 1 second before taking another reading
}
