
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

/* ===== THONG TIN WIFI ===== */
const char* ssid = "Hanh1982";
const char* password = "hiep1982";

/* ===== MQTT ===== */
const char* mqtt_server = "192.168.1.11";
const int mqtt_port = 1883;
const char* mqtt_user = "khoii";
const char* mqtt_pass = "12345";

const char* student_id = "10124180  ";

/* ===== MQTT TOPICS ===== */
const char* topic_cmd = "iot/lab2/10124180/cmd";
const char* topic_state = "iot/lab2/10124180/state";

/* ===== LED ONBOARD ===== */
#define LED_PIN 48

bool ledState = false;

WiFiClient espClient;
PubSubClient client(espClient);

/* ===== DIEU KHIEN LED ===== */
void controlLED(bool state) {
  ledState = state;

  if (ledState) {
    neopixelWrite(LED_PIN, 0, 50, 0);
    Serial.println("LED ON");
  } else {
    neopixelWrite(LED_PIN, 0, 0, 0);
    Serial.println("LED OFF");
  }
}

/* ===== KET NOI WIFI ===== */
void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

/* ===== NHAN LENH MQTT ===== */
void callback(char* topic, byte* payload,
              unsigned int length) {

  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  message.trim();

  Serial.print("Received MQTT: ");
  Serial.println(message);

  if (String(topic) == topic_cmd) {

    if (message == "ON") {
      controlLED(true);
    }
    else if (message == "OFF") {
      controlLED(false);
    }
    else {
      return;
    }

    // Gui trang thai LED len MQTT
    client.publish(
      topic_state,
      ledState ? "ON" : "OFF"
    );
  }
}

/* ===== KET NOI MQTT ===== */
void reconnect() {
  while (!client.connected()) {

    String clientId = "ESP32S3_Lab2_";
    clientId += student_id;

    Serial.print("Connecting MQTT...");

    if (client.connect(
      clientId.c_str(),
      mqtt_user,
      mqtt_pass
    )) {

      Serial.println("MQTT CONNECTED!");

      // Dang ky nhan lenh
      client.subscribe(topic_cmd);

      Serial.print("Subscribed: ");
      Serial.println(topic_cmd);

      // Dong bo trang thai ban dau
      client.publish(
        topic_state,
        ledState ? "ON" : "OFF"
      );

    } else {
      Serial.print("Failed rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);
  delay(1000);

  controlLED(false);

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setKeepAlive(60);
}

/* ===== LOOP ===== */
void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}
