#include <SoftwareSerial.h>

SoftwareSerial espSerial(9, 10); // RX, TX

const int LED_PIN = 13;

void setup() {
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);
  espSerial.begin(9600);
}

void loop() {
  if (espSerial.available()) {
    char data = espSerial.read();

    if (data == '0') {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED TAT");
    }

    if (data == '1') {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED BAT");
    }
  }
}