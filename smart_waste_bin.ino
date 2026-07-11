#include <ESP32Servo.h>

// === Pin Definitions ===
#define IR_SENSOR_PIN 13
#define MOISTURE_PIN 34
#define SERVO_PIN 18

#define TRIG_WET 27
#define ECHO_WET 26
#define TRIG_DRY 33
#define ECHO_DRY 32

#define LED_PIN 2 // Optional: status LED for IR detection

Servo myServo;

// === Function to measure distance ===
long readUltrasonicDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH, 30000) * 0.034 / 2; // in cm
}

void setup() {
  Serial.begin(115200);

  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(MOISTURE_PIN, INPUT);

  pinMode(TRIG_WET, OUTPUT);
  pinMode(ECHO_WET, INPUT);
  pinMode(TRIG_DRY, OUTPUT);
  pinMode(ECHO_DRY, INPUT);

  pinMode(LED_PIN, OUTPUT);

  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN);
  myServo.write(90); // Neutral position
  Serial.println("System Ready");
}

void loop() {
  int irValue = digitalRead(IR_SENSOR_PIN);
  digitalWrite(LED_PIN, (irValue == LOW) ? HIGH : LOW); // LED ON when object detected

  if (irValue == LOW) {
    delay(100); // Debounce
    if (digitalRead(IR_SENSOR_PIN) == LOW) {
      Serial.println("IR Triggered: Waste Detected!");

      // === Read moisture ===
      int moisture = analogRead(MOISTURE_PIN);
      String wasteType = (moisture < 3000) ? "Wet" : "Dry"; // Calibrated threshold
      Serial.print("Moisture Value: ");
      Serial.print(moisture);
      Serial.print(" → Type: ");
      Serial.println(wasteType);

      // === Rotate servo ===
      if (wasteType == "Wet") {
        myServo.write(30); // Rotate left to WET bin
        Serial.println("Wet Waste → Routing to WET bin");
      } else {
        myServo.write(150); // Rotate right to DRY bin
        Serial.println("Dry Waste → Routing to DRY bin");
      }

      delay(1500);
      myServo.write(90); // Reset to center
      Serial.println("Servo Reset to Center\n");
    }
  }

  // === Bin fill levels ===
  long wetDistance = readUltrasonicDistance(TRIG_WET, ECHO_WET);
  long dryDistance = readUltrasonicDistance(TRIG_DRY, ECHO_DRY);

  String wetStatus = (wetDistance > 10) ? "FULL" : "Not Full";
  String dryStatus = (dryDistance > 10) ? "FULL" : "Not Full";

  Serial.print("Wet Bin Level: ");
  Serial.print(wetDistance);
  Serial.print(" cm → Status: ");
  Serial.println(wetStatus);

  Serial.print("Dry Bin Level: ");
  Serial.print(dryDistance);
  Serial.print(" cm → Status: ");
  Serial.println(dryStatus);

  Serial.println("-----------------------------------");
  delay(2000);
}
