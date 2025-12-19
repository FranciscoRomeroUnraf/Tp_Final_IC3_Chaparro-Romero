#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <time.h>
#include <ArduinoJson.h>
#include <WiFiManager.h> // Instalar librería "WiFiManager" por tzpu
#include <Preferences.h> // Para guardar la IP del broker

// ===== OBJETOS =====
WiFiClient espClient;
MqttClient client(espClient);
Preferences preferences;

// ===== VARIABLES DE CONFIGURACIÓN (Dinámicas) =====
char mqtt_server[40] = ""; // Se llenará con WiFiManager
const char* tenant   = "UNRaf";

// ===== PINES (Tus pines originales) =====
const int pinLow    = 13;
const int pinMid    = 12;
const int pinHigh   = 27;
const int pinBomba  = 15;
const int pinRed    = 33;
const int pinGreen  = 26;
const int pinBlue   = 14;

// ===== VARIABLES DE ESTADO =====
String mac;
bool bombaEncendida   = false;
bool alarmaEncendida  = false;
String nivel          = "DESCONOCIDO";

// Timers
unsigned long tiempoApagado        = 0;
unsigned long tiempoMaxBomba       = 0;
unsigned long tiempoDescansoBomba  = 10000;
unsigned long inicioBomba          = 0;
unsigned long bloqueoBomba         = 0;

// Variables de estado anterior
String prevNivel     = "";
bool prevBomba       = false;
bool prevAlarma      = false;
unsigned long ultimoEnvioHora = 0;
const unsigned long intervaloHora = 3600000;

// ===== LED RGB CONTROL =====
void setColor(bool r, bool g, bool b) {
  digitalWrite(pinRed,   r ? HIGH : LOW);
  digitalWrite(pinGreen, g ? HIGH : LOW);
  digitalWrite(pinBlue,  b ? HIGH : LOW);
}

// ===== CONFIGURACIÓN WIFI Y PORTAL =====
void setup_wifi_manager() {
  WiFi.mode(WIFI_STA);
  WiFiManager wm;

  // 1. Recuperar IP guardada o usar default
  preferences.begin("mqtt_config", true);
  String ip_guardada = preferences.getString("server_ip", "192.168.0.100");
  ip_guardada.toCharArray(mqtt_server, 40);
  preferences.end();

  // 2. Campo para ingresar IP del Broker
  WiFiManagerParameter custom_mqtt_server("server", "IP Broker MQTT", mqtt_server, 40);
  wm.addParameter(&custom_mqtt_server);

  // 3. Crear Portal si no conecta (Nombre de red: ESP32_CONFIG)
  if (!wm.autoConnect("ESP32_CONFIG")) {
    Serial.println("Fallo al conectar, reiniciando...");
    ESP.restart();
  }

  // 4. Guardar nueva IP si cambió
  String nueva_ip = custom_mqtt_server.getValue();
  if (nueva_ip != ip_guardada) {
    preferences.begin("mqtt_config", false);
    preferences.putString("server_ip", nueva_ip);
    preferences.end();
    nueva_ip.toCharArray(mqtt_server, 40);
  }

  Serial.println("\nWiFi Conectado. IP Broker: " + String(mqtt_server));
  
  // Configurar hora
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  mac = WiFi.macAddress();
  mac.replace(":", "");
}

// ===== UTILS =====
String getDateCompact() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "00000000T";
  char buffer[16];
  strftime(buffer, sizeof(buffer), "%Y%m%dT%H%M%S", &timeinfo);
  return String(buffer);
}

// ===== CALLBACK MQTT =====
void onMqttMessage(int messageSize) {
  String msg;
  while (client.available()) msg += (char)client.read();
  
  StaticJsonDocument<200> doc;
  deserializeJson(doc, msg);

  String command = doc["command"];
  int tiempo     = doc["tiempo"] | 0;
  String ackTopic = String(tenant) + "/" + mac + "/ack";

  // Lógica de comandos original
  if (command == "encender" && tiempo > 0) {
    bombaEncendida = true;
    inicioBomba = millis();
    tiempoMaxBomba = tiempo * 1000;
    digitalWrite(pinBomba, HIGH);
    client.beginMessage(ackTopic, false, 1); client.print("Bomba ON"); client.endMessage();
  } 
  else if (command == "apagar") {
    bombaEncendida = false;
    digitalWrite(pinBomba, LOW);
    client.beginMessage(ackTopic, false, 1); client.print("Bomba OFF"); client.endMessage();
  } 
  else if (command == "set_max") {
    tiempoMaxBomba = tiempo * 1000;
  }
  else if (command == "set_descanso") {
    tiempoDescansoBomba = tiempo * 1000;
  }
}

// ===== RECONNECT =====
void reconnect() {
  if (client.connected()) return;
  
  Serial.print("Conectando a MQTT: "); Serial.println(mqtt_server);
  
  String statusTopic = String(tenant) + "/" + mac + "/status";
  client.beginWill(statusTopic.c_str(), 7, true, 1);
  client.print("offline");
  client.endWill();

  if (client.connect(mqtt_server, 1883)) {
    Serial.println("Conectado");
    client.subscribe((String(tenant) + "/" + mac + "/cmd").c_str());
    client.beginMessage(statusTopic.c_str(), true, 1); client.print("online"); client.endMessage();
  } else {
    Serial.println("Error MQTT. Reintento en 5s");
    delay(5000);
  }
}

// ===== SETUP & LOOP =====
void setup() {
  Serial.begin(115200);
  pinMode(pinLow, INPUT_PULLUP);
  pinMode(pinMid, INPUT_PULLUP);
  pinMode(pinHigh, INPUT_PULLUP);
  pinMode(pinBomba, OUTPUT);
  digitalWrite(pinBomba, LOW);
  pinMode(pinRed, OUTPUT); pinMode(pinGreen, OUTPUT); pinMode(pinBlue, OUTPUT);
  setColor(false, false, false);

  setup_wifi_manager(); // Nueva función de conexión
  client.onMessage(onMqttMessage);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) ESP.restart();
  if (!client.connected()) reconnect();
  client.poll();

  unsigned long ahora = millis();
  
  // --- Lógica de Sensores Original ---
  bool low = !digitalRead(pinLow);
  bool mid = !digitalRead(pinMid);
  bool high = !digitalRead(pinHigh);

  if (low && !high) { nivel = "BAJO"; alarmaEncendida = true; } 
  else if (mid && !high) { nivel = "MEDIO"; alarmaEncendida = false; } 
  else if (high) { nivel = "ALTO"; alarmaEncendida = false; } 
  else { nivel = "DESCONOCIDO"; alarmaEncendida = false; }

  // --- Lógica de Bomba ---
  if (bombaEncendida && tiempoMaxBomba > 0 && (ahora - inicioBomba >= tiempoMaxBomba)) {
    bombaEncendida = false;
    digitalWrite(pinBomba, LOW);
    bloqueoBomba = ahora + tiempoDescansoBomba;
    client.beginMessage((String(tenant) + "/" + mac + "/ack").c_str(), false, 1);
    client.print("Apagado por tiempo max"); client.endMessage();
  }

  if (nivel == "MEDIO" && ahora > bloqueoBomba) {
    if (!bombaEncendida) {
      bombaEncendida = true;
      inicioBomba = ahora;
      digitalWrite(pinBomba, HIGH);
    }
  } else if (nivel != "MEDIO" && bombaEncendida) {
    bombaEncendida = false;
    digitalWrite(pinBomba, LOW);
  }

  // --- LEDs ---
  if (alarmaEncendida) setColor(true, false, false);
  else if (bombaEncendida) setColor(false, true, false);
  else if (nivel == "ALTO") setColor(false, false, true);
  else setColor(false, false, false);

  // --- Publicación ---
  bool publicar = false;
  if (nivel != prevNivel || bombaEncendida != prevBomba || alarmaEncendida != prevAlarma) publicar = true;
  if (millis() - ultimoEnvioHora >= intervaloHora) { publicar = true; ultimoEnvioHora = millis(); }

  if (publicar) {
    String topicBase = String(tenant) + "/" + mac;
    client.beginMessage((topicBase + "/nivel").c_str(), true, 1); client.print(nivel); client.endMessage();
    client.beginMessage((topicBase + "/bomba").c_str(), true, 1); client.print(bombaEncendida ? "ON" : "OFF"); client.endMessage();
    client.beginMessage((topicBase + "/alarma").c_str(), true, 1); client.print(alarmaEncendida ? "ON" : "OFF"); client.endMessage();
    
    prevNivel = nivel; prevBomba = bombaEncendida; prevAlarma = alarmaEncendida;
  }
  delay(100);
}