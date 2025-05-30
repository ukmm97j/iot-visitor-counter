#include <WiFiEsp.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ThingSpeak.h>
#include <SoftwareSerial.h>

// Software serial connection with ESP8266 via D2 and D3
SoftwareSerial espSerial(2, 3); // RX, TX

// WiFi credentials
char ssid[] = "M97";
char pass[] = "10101010";

// ThingSpeak credentials
unsigned long channelID = 2966182;
const char* writeAPIKey = "KQI3CL1K1IPSCGYO";

// Objects
WiFiEspClient client;
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD 16x2 with I2C address 0x27

// Pin definitions
int irPin = 7; // ✅ IR sensor now connected to D7
int motionCount = 0;
bool objectDetected = false;
unsigned long lastSendTime = 0;
unsigned long sendInterval = 15000; // Send every 15 seconds

void setup() {
  pinMode(irPin, INPUT);  // Configure IR pin

  lcd.init();             // Initialize LCD
  lcd.backlight();        // Turn on backlight
  lcd.setCursor(0, 0);
  lcd.print("Booting...");

  Serial.begin(9600);
  espSerial.begin(9600);    // Start ESP communication
  WiFi.init(&espSerial);    // Initialize WiFi library

  if (WiFi.status() == WL_NO_SHIELD) {
    lcd.clear();
    lcd.print("No WiFi shield");
    while (true);
  }

  while (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.print("Connecting WiFi");
    WiFi.begin(ssid, pass);
    delay(5000);
  }

  lcd.clear();
  lcd.print("WiFi Connected!");
  delay(1000);

  ThingSpeak.begin(client);

  // ✅ Start count from 0
  motionCount = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready");
  lcd.setCursor(0, 1);
  lcd.print("Count: 0");
}

void loop() {
  int motion = digitalRead(irPin);

  if (motion == LOW && !objectDetected) {
    objectDetected = true;
    motionCount++;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Object Detected");
    lcd.setCursor(0, 1);
    lcd.print("Count: ");
    lcd.print(motionCount);

    Serial.print("Detected! Count = ");
    Serial.println(motionCount);

    if (millis() - lastSendTime > sendInterval) {
      int statusCode = ThingSpeak.writeField(channelID, 1, motionCount, writeAPIKey);
      if (statusCode == 200) {
        Serial.println("Sent to ThingSpeak");
      } else {
        Serial.print("Failed. Status: ");
        Serial.println(statusCode);
      }
      lastSendTime = millis();
    }

    delay(1000);
  }

  if (motion == HIGH && objectDetected) {
    objectDetected = false;
  }
}
