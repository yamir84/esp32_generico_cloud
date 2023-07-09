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

// -------------------------------------------------------------------
// Definiciones
// -------------------------------------------------------------------
#define RELAY1  32                  // GPIO32 para salida de Relay 1
#define RELAY2  33                  // GPIO33 para salida de Relay 2
#define WIFILED 26                  // GPIO26 LED INDICADOR WIFI 
#define MQTTLED 27                  // GPIO27 LED INDICADOR MQTT
#define DIMMER  25                  // GPIO25 LED INDICADOR DIMMER
// -------------------------------------------------------------------
// CALCULAR LA CAPACIDAD DEL JSON
// Asistente ArduinoJson: https://arduinojson.org/v6/assistant/
// Documentación: https://arduinojson.org/v6/api/json/deserializejson/
// https://arduinojson.org/v6/doc/
// -------------------------------------------------------------------
const size_t capacitySettings = 1024*5;                 // 5KB
// -------------------------------------------------------------------
// Versión de Firmware desde las variables de entorno platformio.ini
// -------------------------------------------------------------------
#define TEXTIFY(A) #A
#define ESCAPEQUOTE(A) TEXTIFY(A)
String device_fw_version = ESCAPEQUOTE(BUILD_TAG);
// -------------------------------------------------------------------
// Version de Hardware y Fabricante
// -------------------------------------------------------------------
#define device_hw_version   "ESP32 v1 00000000"         // Versión del hardware
#define device_manufacturer "IOTHOST"                   // Fabricante
// -------------------------------------------------------------------
// Zona configuración Dispositivo
// ------------------------------------------------------------------- 
int             device_restart;                         // Número de reinicios 
// -------------------------------------------------------------------
// Zona configuración WIFI modo Cliente
// -------------------------------------------------------------------
char            wifi_ssid[63]       = "iotmaster";      // Nombre de la red WiFi a conectar                
char            wifi_password[63]   = "iotmaster84";    // Contraseña de la Red WiFi a conectar
// -------------------------------------------------------------------
// Zona configuración MQTT 
// ------------------------------------------------------------------- 
char            mqtt_server[39]     = "164.152.53.131"; // Servidor del MQTT Broker
int             mqtt_port           = 1883;             // Puerto servidor MQTT Broker 1883
char            mqtt_id[30]         = "000000001";      // Cliente ID MQTT Broker       
char            mqtt_user[30]       = "emqx";           // Usuario MQTT Broker 
char            mqtt_password[39]   = "public";         // Contraseña del MQTT Broker
int             mqtt_time_interval  = 30000;            // Tiempo de envio por MQTT en segundos 30 Segundos
// -------------------------------------------------------------------
// Zona EEPROM para contador de reinicios
// -------------------------------------------------------------------
#define Start_Address 0                                 // Posición de inicio en la EEPROM
#define Restart_Address Start_Address + sizeof(int)     // Dirección del dato en la EEPROM
// -------------------------------------------------------------------
// Zona Relay
// ------------------------------------------------------------------- 
bool            RELAY1_STATUS;                         // GPIO32 Estado del pin
bool            RELAY2_STATUS;                         // GPIO33 Estado del pin
// -------------------------------------------------------------------
// Zona PWM
// ------------------------------------------------------------------- 
const int       freq = 1000;                           // frecuencia de trabajo - 1kHz
const int       ledChannel = 0;                        // definicion del canal - 0
const int       resolution = 8;                        // 8 bits -> 255
int             dim;                                   // valor del dimmer a enviar ( 0 - 100 )
// -------------------------------------------------------------------
// Zona Otras 
// -------------------------------------------------------------------
float           temp_cpu;                              // Temperatura del CPU en °C
float           temperatureC, temperatureF;            // Temperatura en °C °F
