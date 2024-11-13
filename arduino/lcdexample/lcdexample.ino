#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize with known address 0x27 and LCD dimensions 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    lcd.init();           // Initialize the LCD
    lcd.backlight();      // Ensure the backlight is on
    lcd.setCursor(0, 0);  // Set cursor to the top-left corner
    lcd.print("Hello, World!");
    lcd.setCursor(0, 1);  // Move to the second row
    lcd.print("I2C LCD Display");
}

void loop() {
    // No additional code needed here
}
