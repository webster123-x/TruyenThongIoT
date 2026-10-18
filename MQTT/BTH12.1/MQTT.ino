#include <WiFi.h>
#include <PubSubClient.h>

// ===== WIFI =====
const char* ssid = "Hanh1982";
const char* password = "hiep1982";

// ===== MQTT BROKER =====
const char* mqtt_server = "192.168.1.11";
const int mqtt_port = 1883;
const char* mqtt_user = "khoii";
const char* mqtt_pass = "12345";

const char* mqtt_topic = "iot/lab1/10120056/sensor";

WiFiClient espClient;
PubSubClient mqtt(espClient);

// ===== BIEN THOI GIAN =====
unsigned long lastRead = 0;
unsigned long lastMqttAttempt = 0;
unsigned long lastWifiAttempt = 0;

bool wifiWasConnected = false;

// ===== KET NOI WIFI =====
void connectWiFi() {
  Serial.print("\nDang ket noi Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  wifiWasConnected = (WiFi.status() == WL_CONNECTED);

  if (wifiWasConnected) {
    Serial.println("\nDa ket noi Wi-Fi!");
    Serial.print("IP ESP32: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nKet noi Wi-Fi that bai.");
  }

  lastWifiAttempt = millis();
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  mqtt.setServer(mqtt_server, mqtt_port);
  mqtt.setSocketTimeout(5);

  connectWiFi();
}

// ===== LOOP =====
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiWasConnected) {
      Serial.println("Mat ket noi Wi-Fi!");
      wifiWasConnected = false;
      mqtt.disconnect();
    }

    if (millis() - lastWifiAttempt >= 10000) {
      connectWiFi();
    }

    delay(10);
    return;
  }

  if (!wifiWasConnected) {
    wifiWasConnected = true;
    Serial.println("Wi-Fi da ket noi lai!");
    Serial.print("IP ESP32: ");
    Serial.println(WiFi.localIP());
  }

  // Thu ket noi MQTT moi 5 giay
  if (!mqtt.connected() &&
      millis() - lastMqttAttempt >= 5000) {
    lastMqttAttempt = millis();

    Serial.print("Dang ket noi MQTT: ");
    Serial.println(mqtt_server);

    String clientId = "ESP32-Virtual-";
    clientId += WiFi.macAddress();

    if (mqtt.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Da ket noi MQTT!");
    } else {
      Serial.print("Ket noi MQTT that bai. Ma loi: ");
      Serial.println(mqtt.state());
    }
  }

  if (mqtt.connected()) {
    mqtt.loop();
  }

  // Tao va gui du lieu ao moi 5 giay
  if (millis() - lastRead >= 5000) {
    lastRead = millis();

    // Nhiet do: 25.0 den 35.0 do C
    // Do am: 40.0 den 90.0 %
    float temp = random(250, 351) / 10.0f;
    float hum = random(400, 901) / 10.0f;

    Serial.print("[DU LIEU ] Nhiet do: ");
    Serial.print(temp, 1);
    Serial.print(" C | Do am: ");
    Serial.print(hum, 1);
    Serial.println(" %");

    if (mqtt.connected()) {
      char payload[80];

      snprintf(
        payload,
        sizeof(payload),
        "{\"temp\":%.1f,\"hum\":%.1f}",
        temp,
        hum
      );

      if (mqtt.publish(mqtt_topic, payload, true)) {
        Serial.print("Da gui MQTT: ");
        Serial.println(payload);
      } else {
        Serial.println("Gui MQTT that bai!");
      }
    } else {
      Serial.println("MQTT chua ket noi, chua gui du lieu.");
    }
  }
}