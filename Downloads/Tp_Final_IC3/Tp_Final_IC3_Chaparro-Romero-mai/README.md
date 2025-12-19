Este modo levanta el Broker MQTT, un Simulador de ESP32 y el Panel de Control en contenedores aislados. No requiere instalar Python ni Arduino IDE en la máquina anfitriona.

Requisitos: Tener instalado Docker Desktop.

Ejecución: Abra una terminal en la carpeta principal del proyecto y ejecute:

docker-compose up --build

Resultado:

Se iniciará el broker Mosquitto.

Se iniciará el Simulador enviando datos aleatorios (Nivel, Bomba, Alarma).

Se iniciará el Panel de Control mostrando los datos recibidos en tiempo real.

Para detener: Presione Ctrl + C.