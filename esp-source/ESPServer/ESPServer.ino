#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

// ############################################################################
#define FIREBASE_DB       "oulu-iot-hw3.firebaseio.com"
#define FIREBASE_KEY      "paste-secret-key-here-removed-for-security"

#define WIFI_ROUTER       "AccessPoint"
#define WIFI_PASSWORD     "Password"

#define MOTION_PIN1        D1
#define MOTION_PIN2        D2
// ############################################################################
bool manual = true;
bool detected1 = false;
bool detected2 = false;
int n = 0;
int t = 0;

// Interrupt function to read from motion sensors
ICACHE_RAM_ATTR void motion1() {
  detected1 = true;
}

ICACHE_RAM_ATTR void motion2() {
  detected2 = true;
}

void setup() {
  // Initiate serial monitor
  Serial.begin(9600);

  // Define pin modes
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOTION_PIN1, INPUT_PULLUP);
  pinMode(MOTION_PIN2, INPUT_PULLUP);

  // Initiate interrupts
  if (!manual) {
    attachInterrupt(digitalPinToInterrupt(MOTION_PIN1), motion1, RISING);
    attachInterrupt(digitalPinToInterrupt(MOTION_PIN2), motion2, RISING);
  }

  // Connect to network
  digitalWrite(LED_BUILTIN, LOW);
  WiFi.begin(WIFI_ROUTER, WIFI_PASSWORD);
  Serial.print("Connecting to Home WIFI Router");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WiFi.localIP());

  // Initiate Firebase connection
  Firebase.begin(FIREBASE_DB, FIREBASE_KEY);
}

void manualMode(int d) {
  delay(d);
  digitalWrite(LED_BUILTIN, HIGH);
  Firebase.setString("counter", String(n));
  if (Firebase.failed()) {
    Serial.print("Error encountered");
    Serial.println(Firebase.error());
    return;
  }

  Serial.println("Sensor reading sent ...");
  n++;  t++;
  if (t == 3) {
    t = 0; n = n - 2;
    Serial.println("Resetting counter ...");
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void autoMode() {
  if (detected1) {
    if (detected2) {
      detected1 = false; detected2 = false; n++;
      digitalWrite(LED_BUILTIN, HIGH);
      Firebase.setString("counter", String(n));
      if (Firebase.failed()) {
        Serial.print("Error encountered");
        Serial.println(Firebase.error());
        return;
      }
      Serial.println("Sensor reading sent ...");
    } else if (detected2) {
      if (detected1) {
        detected1 = false; detected2 = false; n--;
        digitalWrite(LED_BUILTIN, HIGH);
        Firebase.setString("counter", String(n));
        if (Firebase.failed()) {
          Serial.print("Error encountered");
          Serial.println(Firebase.error());
          return;
        }
        Serial.println("Sensor reading sent ...");
      }
    } else {
      detected1 = false; detected2 = false;
    }
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void loop() {
  if (manual) {
    manualMode(2000);
  } else {
    autoMode();
  }
}
