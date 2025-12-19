import paho.mqtt.client as mqtt
import os
import time
import sys

# Configuración dinámica
BROKER = os.getenv('MQTT_BROKER', 'localhost')
PORT = 1883
TENANT = "UNRaf"

# Suscribirse a TODOS los dispositivos
BASE_TOPIC = f"{TENANT}/#"

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"✅ PANEL DE CONTROL: Conectado a {BROKER}")
        client.subscribe(BASE_TOPIC)
    else:
        print(f"❌ Error conexión: {rc}")

def on_message(client, userdata, msg):
    try:
        topic = msg.topic
        payload = msg.payload.decode("utf-8")
        print(f"📩 [{topic}] → {payload}")
    except Exception as e:
        print(f"Error procesando mensaje: {e}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

print(f"🔌 Panel iniciando conexión a {BROKER}...")
time.sleep(5) # Espera inicial

client.connect(BROKER, PORT, 60)
client.loop_forever()