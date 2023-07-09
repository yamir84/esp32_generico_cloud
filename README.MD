# IOTHost ESP32 GENERIC DEVICE - CLOUD PROJECT
```tsx
  /* -------------------------------------------------------------------
  ;* AdminESP - iothost.org 2023
  ;* Sitio WEB: https://iothost.org
  ;* Correo: admin@electroniciot.com
  ;* Cel_WSP: +591 71243395
  ;* Plataforma: ESP32
  ;* Framework:  Arduino - Platformio - VSC
  ;* Proyecto: Dispositivo IOT para el Cloud IOT con EMQX
  ;* Autor: Ing. Yamir Hidalgo Peña
  ;* -------------------------------------------------------------------
  */
```
*********
## CONFIGURA EL PUERTO DEL DISPOSITIVO
*********

* 1 - Abrir el archivo: platformio.ini | editar el puerto
```tsx
  upload_port = COM11
  monitor_port = COM11
```
*********
## CONFIGURA VARIABLES DE CONEXIÓN
*********
* 2 - Abrir el archivo: /include/esp32_definitions.hpp | editar los siguientes parametros
```tsx
// -------------------------------------------------------------------
// Zona configuración WIFI modo Cliente
// -------------------------------------------------------------------
char            wifi_ssid[63]       = "nombre_de_tu_red_wifi";      // Nombre de la red WiFi a conectar                
char            wifi_password[63]   = "password_red_wifi";          // Contraseña de la Red WiFi a conectar
```
```tsx
// -------------------------------------------------------------------
// Zona configuración MQTT 
// ------------------------------------------------------------------- 
char            mqtt_server[39]     = "iot01.iothost.org";  // Servidor del MQTT Broker
int             mqtt_port           = 1883;                 // Puerto servidor MQTT Broker 1883
char            mqtt_id[30]         = "tu_serialnumber";    // Cliente ID MQTT Broker       
char            mqtt_user[30]       = "tu_usuario";         // Usuario MQTT Broker 
char            mqtt_password[39]   = "tu_contraseña";      // Contraseña del MQTT Broker
int             mqtt_time_interval  = 30000;                // Tiempo de envio por MQTT en segundos 30 Segundos
```
*********
## ACCEDER A LA PLATAFORMA IOT DE INTEGRACIÓN
*********
* 3 - https://iot02.iothost.org
* 4 - Crearte un usuario en el formulario de registro | https://iot02.iothost.org/register
* 5 - Ir a dispositivos | registrar nuevo | llenar los parámetros y el número de serie, verificar que no exista, asignado en ``(mqtt_id)``
* 6 - Disfrutar del DEMO
*********
## ACCEDER AL CURSO COMPLETO EN UDEMY
*********
https://cutt.ly/mwibaNsU
*********
## ACCEDER A DESCUENTOS DEL CURSO EN EL SIGUIENTE LINK
*********
https://cursomania.org/home/course/mqtt-emqx-iot