#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize with known address 0x27 and LCD dimensions 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Data wire of DS18B20 Temperature Sensor
// is connected to digital pin 2
#define ONE_WIRE_BUS 2

// Pin 8 = SSR, other pin connected to GND
// Constrain temp to a range to reduce likeliness of always-on
// An unplugged sensor is like -127 degrees
float tempMinF = 59.0;
float tempMaxF = 93.7;
const int ssrPin = 8;

const int potPin = A0;

// Like arduino map but for floats.
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


// Create a OneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);

// Pass the oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


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
  float temperatureF = sensors.getTempFByIndex(0);

  // 0 - 1023?
  int potValue = analogRead(potPin);
  // map only works with integers.
  float mappedValue = (potValue / 1023.0) * 6.0 - 3.0;
  float targetTempF = tempMaxF + mappedValue;


  for (int i = 0; i < 20; i++) {
    // we put this in an internal loop bc the sensor bounces a lot
    // and we don't want to switch on and off the heater every second.
    // TODO: fix this (maybe use a deque and an average)
    sensors.requestTemperatures();
    temperatureF = sensors.getTempFByIndex(0);
    potValue = analogRead(potPin);  // 0 - 1023?
    mappedValue = (potValue / 1023.0) * 8.0 - 4.0; // map only works with integers
    targetTempF = tempMaxF + mappedValue;

    lcd.setCursor(0, 0);    // Set cursor to start of first row
    lcd.print("f: " + String(temperatureF) + " -> " + String(targetTempF) + "    ");
    delay(1000);
  }
  
  // Display the temperature
  Serial.println("Temperature: " + String(temperatureF) + " °F");

  int ssrStatus = LOW;


  if (tempMinF < temperatureF && temperatureF < targetTempF) {
    // digitalWrite(ssrPin, HIGH);
    ssrStatus = HIGH;
  } else {
    ssrStatus = LOW;
    // digitalWrite(ssrPin, LOW);
  }

  digitalWrite(ssrPin, ssrStatus);

  lcd.clear();            // Clear lcd (not necessary but nice)
  lcd.setCursor(0, 0);    // Set cursor to start of the second row
  lcd.print("f: " + String(temperatureF) + " -> " + String(targetTempF) + "    ");   // Print the label
  // lcd.print(temperatureC);     // Display the counter value
  // lcd.print("    ");      // Clear extra characters
  lcd.setCursor(0, 1);
  lcd.print("heater: " + String(ssrStatus) + "    ");

  delay(1000); // wait 1 second
  // delay(1000 * 60); // Wait 60 seconds before taking another reading
}
