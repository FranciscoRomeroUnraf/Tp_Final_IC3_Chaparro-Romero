import paho.mqtt.client as mqtt
import time
import random
import os
from datetime import datetime

# Configuración: Si está en Docker usa el nombre del servicio, si no localhost
BROKER = os.getenv('MQTT_BROKER', 'localhost')
PORT = 1883
TENANT = "UNRaf"
# Usamos una MAC virtual para diferenciarlo
DEVICE_MAC = "TEST_VIRTUAL_DOCKER" 
TOPIC_BASE = f"{TENANT}/{DEVICE_MAC}"

client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"✅ SIMULADOR: Conectado al broker {BROKER}")
        # Publicar estado online (LWT simulado)
        client.publish(f"{TOPIC_BASE}/status", "online", retain=True)
    else:
        print(f"❌ Fallo conexión código: {rc}")

client.on_connect = on_connect

print("⏳ Iniciando Simulador (Esperando al broker)...")
time.sleep(5) # Espera de seguridad para que Mosquitto arranque

try:
    client.connect(BROKER, PORT, 60)
    client.loop_start()
except Exception as e:
    print(f"Error fatal: {e}")
    exit(1)

# Bucle principal (Equivalente al void loop del Arduino)
while True:
    # --- Lógica copiada de TEST_Publicador_MQTT.ino ---
    niveles = ["BAJO", "MEDIO", "ALTO"]
    nivel = random.choice(niveles)
    
    # Lógica de bomba y alarma basada en tu código original
    bomba = "ON" if random.choice([True, False]) else "OFF"
    alarma = "ON" if nivel == "BAJO" else "OFF"
    
    # Fecha compacta
    fecha = datetime.now().strftime("%Y%m%dT%H%M%S")

    # Publicar en los tópicos
    client.publish(f"{TOPIC_BASE}/nivel", nivel)
    client.publish(f"{TOPIC_BASE}/bomba", bomba)
    client.publish(f"{TOPIC_BASE}/alarma", alarma)
    client.publish(f"{TOPIC_BASE}/datetime", fecha)

    print(f"📤 [SIMULADOR] Enviado: Nivel={nivel}, Bomba={bomba}, Alarma={alarma}")
    
    # Esperar 10 segundos (Intervalo definido en tu código Arduino)
    time.sleep(10)