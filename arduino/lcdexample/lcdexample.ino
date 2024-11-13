#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize with known address 0x27 and LCD dimensions 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

int counter = 0;

void setup() {
    lcd.init();           // Initialize the LCD
    lcd.backlight();      // Ensure the backlight is on
    lcd.setCursor(0, 0);  // Set cursor to the top-left corner
    lcd.print("Hello, World!");
    lcd.setCursor(0, 1);  // Move to the second row
    lcd.print("I2C LCD Display");
    delay(1000);
}

void loop() {
    lcd.clear();            // Clear lcd (not necessary but nice)
    lcd.setCursor(0, 1);    // Set cursor to start of the second row
    lcd.print("Count: ");   // Print the label
    lcd.print(counter);     // Display the counter value
    lcd.print("    ");      // Clear extra characters

    counter++;              // Increment the counter
    delay(100);            // Wait for ms
}

// Optimal LCD update: every 50-100 ms (~10-20 fps)
// Fastest safe LCD update: every 10-20 ms

