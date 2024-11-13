// Define the pin connected to the SSR
const int ssrPin = 8;

void setup() {
  // Set the SSR pin as an output
  pinMode(ssrPin, OUTPUT);
}

void loop() {
  // Turn the SSR on (activates the connected device)
  digitalWrite(ssrPin, HIGH);
  delay(5000); // Keep it on for 5 seconds

  // Turn the SSR off (deactivates the connected device)
  digitalWrite(ssrPin, LOW);
  delay(5000); // Keep it off for 5 seconds
}