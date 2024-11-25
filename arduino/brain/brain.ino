#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize with known address 0x27 and LCD dimensions 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Data wire of DS18B20 Temperature Sensor
// is connected to digital pin 2
#define ONE_WIRE_BUS 2

// Define the pin connected to the SSR
// Other side of the SSR is gonnected to GND
// Constrain temp to a range to reduce likeliness of always-on
// An unplugged sensor is like -127 degrees
float tempMin = 15.0;
float tempMax = 34.28;
const int ssrPin = 8;

// Create a OneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);

// Pass the oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


/*
Fahrenheit | Celsius
-----------|---------
 60.0.     | 15.55 
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
 97.0      | 36.11
 98.6.     | 37.00

Note: Celsius = (Fahrenheit - 32) * 5 / 9
*/



void setup() {
  Serial.begin(9600);
  sensors.begin(); // Start the DS18B20 sensor
  Serial.println("DS18B20 Temperature Sensor Ready");
  pinMode(ssrPin, OUTPUT);
  // LCD init
  lcd.init();           // Initialize the LCD
  lcd.backlight();      // Ensure the backlight is on
  lcd.setCursor(0, 0);  // Set cursor to the top-left corner
  lcd.print("Hello, World!");
  lcd.setCursor(0, 1);  // Move to the second row
  lcd.print("I2C LCD Display");
  delay(1000);
}

void loop() {

  sensors.requestTemperatures(); // Request temperature measurement
  
  // Read temperature in Celsius
  float temperatureC = sensors.getTempCByIndex(0);
  
  // Display the temperature
  Serial.println("Temperature: " + String(temperatureC) + " °C");

  int ssrStatus = LOW;


  if (tempMin < temperatureC && temperatureC < tempMax) {
    // digitalWrite(ssrPin, HIGH);
    ssrStatus = HIGH;
  } else {
    ssrStatus = LOW;
    // digitalWrite(ssrPin, LOW);
  }

  digitalWrite(ssrPin, ssrStatus);

  lcd.clear();            // Clear lcd (not necessary but nice)
  lcd.setCursor(0, 0);    // Set cursor to start of the second row
  lcd.print("temp_c: " + String(temperatureC) + "    ");   // Print the label
  // lcd.print(temperatureC);     // Display the counter value
  // lcd.print("    ");      // Clear extra characters
  lcd.setCursor(0, 1);
  lcd.print("heater: " + String(ssrStatus) + "    ");

  delay(1000 * 20); // wait 20 second
  // delay(1000 * 60); // Wait 60 seconds before taking another reading
}
