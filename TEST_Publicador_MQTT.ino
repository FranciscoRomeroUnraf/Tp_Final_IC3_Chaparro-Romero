#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <time.h>

// ==============================
// CONFIGURACIÓN WIFI
// ==============================
const char* ssid     = "FT-ROMERO-2.4GHz";
const char* password = "00417639883";

// ==============================
// CONFIGURACIÓN MQTT
// ==============================
const char* mqtt_server = "192.168.0.16";
const int   mqtt_port   = 1883;

const char* tenant = "UNRaf";
const char* device_mac = "TESTESP321234";  // MAC ficticia para test

WiFiClient espClient;
MqttClient mqtt(espClient);

// ==============================
// TIMING
// ==============================
unsigned long lastPublish = 0;
const unsigned long interval = 10000; // 10 segundos

// ==============================
// WIFI
// ==============================
void setup_wifi() {
  Serial.print("Conectando a WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  configTime(-3 * 3600, 0, "pool.ntp.org");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nHora sincronizada");
}

// ==============================
// FECHA Y HORA
// ==============================
String getDateCompact() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "00000000T000000";

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y%m%dT%H%M%S", &timeinfo);
  return String(buffer);
}

// ==============================
// MQTT CONNECT
// ==============================
void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Conectando a MQTT...");

    String statusTopic = String(tenant) + "/" + device_mac + "/status";

    mqtt.beginWill(statusTopic.c_str(), 7, true, 1);
    mqtt.print("offline");
    mqtt.endWill();

    if (mqtt.connect(mqtt_server, mqtt_port)) {
      Serial.println("OK");

      mqtt.beginMessage(statusTopic.c_str(), true, 1);
      mqtt.print("online");
      mqtt.endMessage();
    } else {
      Serial.print(" error=");
      Serial.println(mqtt.connectError());
      delay(3000);
    }
  }
}

// ==============================
// SETUP
// ==============================
void setup() {
  Serial.begin(115200);
  randomSeed(esp_random());

  setup_wifi();
}

// ==============================
// LOOP
// ==============================
void loop() {
  if (!mqtt.connected()) reconnect();
  mqtt.poll();

  if (millis() - lastPublish >= interval) {
    lastPublish = millis();

    String nivel;
    int r = random(3);
    if (r == 0) nivel = "BAJO";
    else if (r == 1) nivel = "MEDIO";
    else nivel = "ALTO";

    String bomba  = random(2) ? "ON" : "OFF";
    String alarma = (nivel == "BAJO") ? "ON" : "OFF";
    String fecha  = getDateCompact();

    String base = String(tenant) + "/" + device_mac;

    mqtt.beginMessage((base + "/nivel").c_str(), true, 1);
    mqtt.print(nivel);
    mqtt.endMessage();

    mqtt.beginMessage((base + "/bomba").c_str(), true, 1);
    mqtt.print(bomba);
    mqtt.endMessage();

    mqtt.beginMessage((base + "/alarma").c_str(), true, 1);
    mqtt.print(alarma);
    mqtt.endMessage();

    mqtt.beginMessage((base + "/datetime").c_str(), false, 1);
    mqtt.print(fecha);
    mqtt.endMessage();

    Serial.println("📤 TEST publicado:");
    Serial.println("Nivel: " + nivel);
    Serial.println("Bomba: " + bomba);
    Serial.println("Alarma: " + alarma);
    Serial.println("Fecha: " + fecha);
    Serial.println("----------------------");
  }
}

