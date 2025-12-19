# Sistema MQTT con ESP32, Docker y Python

## 📌 Descripción general

Este proyecto implementa un **sistema de comunicación IoT basado en MQTT**, donde un **ESP32** actúa como publicador de datos, un **broker Mosquitto** corre dentro de un contenedor **Docker**, y un **cliente Python** se encarga de suscribirse a los tópicos para monitoreo y control.

El sistema fue diseñado con fines **académicos**, permitiendo demostrar el uso del protocolo MQTT tanto con **hardware real** como mediante un **publicador de prueba (TEST)** que simula datos.

---

## 🧩 Arquitectura del sistema

* **ESP32 (Publicador)**
  Publica datos periódicos (reales o simulados) en distintos tópicos MQTT.

* **Broker MQTT (Mosquitto en Docker)**
  Recibe los mensajes publicados y los distribuye a los clientes suscriptos.

* **Cliente Python (Suscriptor)**
  Se suscribe a los tópicos, muestra los datos recibidos y puede enviar comandos.

---

## 📁 Estructura del repositorio

```
📦 Proyecto_MQTT
├── Publicador_MQTT_IC3.ino
│   ESP32 real: publica datos del sistema
│
├── TEST_Publicador_MQTT.ino
│   ESP32 de prueba: genera datos aleatorios cada 10 s
│
├── Suscriptor_MQTT_IC3.py
│   Cliente Python suscriptor/publicador
│
├── docker-compose.yml
│   Configuración del broker Mosquitto
│
├── mosquitto.conf
│   Configuración del broker MQTT
│
├── data/
│   Persistencia de mensajes del broker
│
└── log/
    Logs del broker Mosquitto
```

---

## 🐳 Configuración del broker MQTT (Docker)

### Requisitos

* Docker
* Docker Compose

### Pasos

1. Ubicarse en el directorio del proyecto
2. Levantar el broker:

```bash
docker-compose up -d
```

3. Verificar que el broker esté activo:

```bash
docker ps
```

El broker quedará escuchando en el puerto **1883**.

---

## 🤖 Publicador ESP32

### Opción 1: Publicador real

Archivo: `Publicador_MQTT_IC3.ino`

* Se programa desde **Arduino IDE**
* Publica datos reales del sistema
* Se conecta al broker mediante WiFi

**Configuración obligatoria:**
Antes de compilar, el usuario **DEBE configurar la IP del broker MQTT** dentro del código. Esta IP debe coincidir con la IP de la computadora o máquina virtual donde corre Mosquitto en Docker.

Ejemplo:

```cpp
const char* mqtt_server = "192.168.0.16"; // IP del broker Mosquitto
```

---

### Opción 2: Publicador TEST (simulación)

Archivo: `TEST_Publicador_MQTT.ino`

* Genera **datos aleatorios**
* Publica mensajes cada **10 segundos**
* Permite probar el sistema **sin sensores ni hardware adicional**

**Configuración obligatoria:**
Al igual que el publicador real, **es obligatorio configurar la IP del broker MQTT** dentro del código para que la comunicación funcione correctamente.

Ejemplo:

```cpp
const char* mqtt_server = "192.168.0.16";
```

Esta opción fue incluida para facilitar la validación del sistema en entornos académicos.

---

el## 🐍 Cliente Python (Suscriptor)

Archivo: `Suscriptor_MQTT_IC3.py`

Funciones principales:

* Suscribirse a los tópicos MQTT
* Mostrar los datos recibidos
* Publicar comandos hacia el ESP32

**Configuración obligatoria:**
Antes de ejecutar el script, el usuario **DEBE configurar la IP del broker MQTT**, y esta **debe ser la misma** utilizada en los programas del ESP32.

Ejemplo:

```python
BROKER = "192.168.0.16"
```

### Ejecución

````bash
python Suscriptor_MQTT_IC3.py
``Mostrar los datos recibidos
- Publicar comandos hacia el ESP32

### Ejecución

```bash
python Suscriptor_MQTT_IC3.py
````

---

## ▶️ Orden de ejecución recomendado

1. Levantar el broker Mosquitto con  el cliente Python
2. Encender el ESP32 (real o TEST)

---

## ✅ Validación del sistema

El sistema fue probado exitosamente utilizando:

* Un **ESP32 real** con datos reales
* Un **ESP32 TEST** con datos simulados

Esto permite verificar el correcto funcionamiento del protocolo MQTT, la comunicación bidireccional y la interoperabilidad entre distintos clientes.

---

## 🎓 Observaciones finales

Este proyecto demuestra la correcta implementación de un sistema IoT basado en MQTT, utilizando herramientas actuales como Docker y Python, y contemplando escenarios reales y de simulación, lo que lo hace adecuado para su evaluación académica.

