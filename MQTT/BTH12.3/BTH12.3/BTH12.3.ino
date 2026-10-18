
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
/* ===== WIFI ===== */
const char* ssid = "Hanh1982";
const char* password = "hiep1982";

/* ===== MQTT ===== */
const char* mqtt_server = "192.168.1.11";
const int mqtt_port = 1883;

const char* mqtt_user = "khoii";
const char* mqtt_pass = "12345";

const char* student_id = "10124180";

/* ===== TOPICS ===== */
String topic_cmd;
String topic_state;
String topic_note;
String topic_lwt;

/* ===== LED ONBOARD ESP32-S3 ===== */
#define LED_PIN 48

bool ledState = false;

WiFiClient espClient;
PubSubClient client(espClient);

/* ===== DIEU KHIEN LED ===== */
void applyLed(bool on) {
  ledState = on;

  if (on) {
    neopixelWrite(LED_PIN, 0, 50, 0);
    Serial.println("LED ON");
  } else {
    neopixelWrite(LED_PIN, 0, 0, 0);
    Serial.println("LED OFF");
  }
}

/* ===== PUBLISH RETAIN ===== */
void publishStateRetained() {
  bool success = client.publish(
    topic_state.c_str(),
    ledState ? "ON" : "OFF",
    true
  );

  Serial.println(
    success ? "Publish retain OK" : "Publish failed"
  );
}

/* ===== CONNECT WIFI ===== */
void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

/* ===== MQTT CALLBACK ===== */
void callback(char* topic, byte* payload,
              unsigned int length) {

  String msg = "";

  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  msg.trim();

  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  if (String(topic) == topic_cmd) {

    if (msg == "ON") {
      applyLed(true);
    }
    else if (msg == "OFF") {
      applyLed(false);
    }
    else if (msg == "TOGGLE") {
      applyLed(!ledState);
    }
    else {
      return;
    }

    publishStateRetained();
  }
}

/* ===== MQTT RECONNECT ===== */
void reconnect() {

  while (!client.connected()) {

    String clientId = "ESP32S3_Lab3_";
    clientId += student_id;

    Serial.print("Connecting MQTT... ");

    if (client.connect(
      clientId.c_str(),
      mqtt_user,
      mqtt_pass,
      topic_lwt.c_str(),
      0,
      true,
      "OFFLINE"
    )) {

      Serial.println("MQTT CONNECTED!");

      client.subscribe(topic_cmd.c_str());

      Serial.print("Subscribed: ");
      Serial.println(topic_cmd);

      // Note khong retain
      client.publish(
        topic_note.c_str(),
        "ESP BOOT / RECONNECT",
        false
      );

      // Trang thai ONLINE
      client.publish(
        topic_lwt.c_str(),
        "ONLINE",
        true
      );

      // Trang thai LED retain
      publishStateRetained();

    } else {
      Serial.print("MQTT failed rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

/* ===== SETUP ===== */
void setup() {

  Serial.begin(115200);
  delay(1000);

  applyLed(false);

  topic_cmd =
    "iot/lab3/" + String(student_id) + "/cmd";

  topic_state =
    "iot/lab3/" + String(student_id) + "/state";

  topic_note =
    "iot/lab3/" + String(student_id) + "/note";

  topic_lwt =
    "iot/lab3/" + String(student_id) + "/lwt";

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
