#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
MPU6050 mpu;

const int buzzer = 8;
const int button = 7;
const int led = 13;
const int alcoholPin = A0;

bool accidentDetected = false;
bool alcoholCancelled = false;

bool lastButtonState = HIGH;

void showMessage(String line1, String line2) {

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(15, 20);
  display.println(line1);

  display.setCursor(15, 35);
  display.println(line2);

  display.display();
}

void setup() {

  Serial.begin(9600);

  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(button, INPUT_PULLUP);

  Wire.begin();
  mpu.initialize();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  showMessage("SMART HELMET", "Monitoring...");
  delay(2000);
}

void loop() {

  // ---------- BUTTON ----------
  bool currentButtonState = digitalRead(button);

  if (lastButtonState == HIGH && currentButtonState == LOW) {

    accidentDetected = false;
    alcoholCancelled = true;

    digitalWrite(buzzer, LOW);
    digitalWrite(led, LOW);

    showMessage("Alert", "Cancelled");
    delay(1500);

    showMessage("SMART HELMET", "Monitoring...");
  }

  lastButtonState = currentButtonState;

  // ---------- ALCOHOL ----------
  int alcoholValue = analogRead(alcoholPin);

  if (alcoholValue < 400) {
    alcoholCancelled = false;
  }

  if (alcoholValue > 500 && !alcoholCancelled && !accidentDetected) {

    digitalWrite(led, HIGH);

    showMessage("ALCOHOL", "DETECTED!");
  }
  else if (!accidentDetected) {

    digitalWrite(led, LOW);

    showMessage("SMART HELMET", "Monitoring...");
  }

  // ---------- MPU ----------
  int16_t gx, gy, gz;
  mpu.getRotation(&gx, &gy, &gz);

  // Detect accident
  if ((abs(gx) > 8000 || abs(gy) > 8000) && !accidentDetected) {

    accidentDetected = true;

    digitalWrite(buzzer, HIGH);
    digitalWrite(led, HIGH);

    showMessage("ACCIDENT", "DETECTED!");
  }

  // ---------- KEEP ALERT ACTIVE ----------
  if (accidentDetected) {

    digitalWrite(buzzer, HIGH);
    digitalWrite(led, HIGH);

    showMessage("ACCIDENT", "DETECTED!");

    // AUTO CANCEL when MPU returns normal
    if (abs(gx) < 1000 && abs(gy) < 1000) {

      accidentDetected = false;

      digitalWrite(buzzer, LOW);
      digitalWrite(led, LOW);

      showMessage("Alert", "Cancelled");
      delay(1500);

      showMessage("SMART HELMET", "Monitoring...");
    }
  }

  delay(100);
}