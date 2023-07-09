;/* -------------------------------------------------------------------
 ;* AdminESP - iothost.org 2023
 ;* Sitio WEB: https://iothost.org
 ;* Correo: admin@electroniciot.com
 ;* Cel_WSP: +591 71243395
 ;* Plataforma: ESP32
 ;* Framework:  Arduino - Platformio - VSC
 ;* Proyecto: Dispositivo IOT para el Cloud IOT con EMQX
 ;* Autor: Ing. Yamir Hidalgo Peña
 ;* -------------------------------------------------------------------
;*/

#include <Arduino.h>
#include <EEPROM.h>
#include <SPIFFS.h>
#include <WIFI.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// -------------------------------------------------------------------
// Configuración del sensor de temperatura DS18B20
// -------------------------------------------------------------------
#define ONE_WIRE_BUS 13
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// -------------------------------------------------------------------
// Archivos *.hpp - Fragmentar el Código
// -------------------------------------------------------------------
#include "esp32_definitions.hpp"
#include "esp32_functions.hpp"
#include "esp32_wifi.hpp"
#include "esp32_mqtt.hpp"
#include "esp32_tasks.hpp"

// -------------------------------------------------------------------
// SETUP
// -------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  log("INFO", "Iniciando el Setup");
  EEPROM.begin(256);                                          // Memoria EEPROM init 256 bytes
  EEPROM.get(Restart_Address, device_restart);                // Leer el valor de la memoria
  device_restart++;                                           // Aumentar el contador
  EEPROM.put(Restart_Address, device_restart);                // Guardar el valor a la memoria
  EEPROM.commit();
  EEPROM.end();
  log("INFO", "Cantidad de reinicios :" + String(device_restart));
  // inicializar el spiffs
  if(!SPIFFS.begin(true)){
    log("ERROR", "Falló la inicialización del SPIFFS");
    while(true);
  }
  // traer las configuraciones desde json
  if(!settingsRead()){
    settingsSave();
  }
  // definir los pines y setearlos
  gpioDefine();
  // Paso estados a los pines de los Relays
  setOnOffSingle(RELAY1, RELAY1_STATUS);
  setOnOffSingle(RELAY2, RELAY2_STATUS);
  // Pasar el dato del dimmer
  ledcWrite(ledChannel, dim * 2.55); // dim => 0 - 100 
  // iniciar el wifi
  wifi_setup();


  // Crear Tarea Reconexión WIFI
  xTaskCreate(TaskWifiReconnect, "TaskWifiReconnect", 1024*6, NULL, 2, NULL);
  // Crear Tarea Reconexión MQTT
  xTaskCreate(TaskMqttReconnect, "TaskMqttReconnect", 1024*6, NULL, 2, NULL);
  // LED MQTT Task
  xTaskCreate( TaskMQTTLed, "TaskMQTTLed", 2048, NULL, 1, NULL);

}

// -------------------------------------------------------------------
// LOOP
// -------------------------------------------------------------------
void loop() {
  // ....
}

