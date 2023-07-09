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

#include "esp32_blink.hpp"

// -------------------------------------------------------------------
// Definición de Funciones
// -------------------------------------------------------------------
void gpioDefine();
boolean settingsRead();
void settingsReset();
boolean settingsSave();
void log(String type, String msg);
String platform();
String HexToStr(const unsigned long &h, const byte &l);
String UniqueID();
String DeviceID();
String ipStr(const IPAddress &ip);
String PathMqttTopic(String topic);
String longTimeStr(const time_t &t);
int getRSSIasQuality(int RSSI);
void readSensor();
float TempCPUValue();

// -------------------------------------------------------------------
// Definir Pines GPIO
// -------------------------------------------------------------------
void gpioDefine(){
    // PINES
    pinMode(WIFILED, OUTPUT);
    pinMode(MQTTLED, OUTPUT);
    pinMode(RELAY1, OUTPUT);  
    pinMode(RELAY2, OUTPUT);
    // PWM
    ledcSetup(ledChannel, freq, resolution);
    ledcAttachPin(DIMMER, ledChannel);
    // SET OFF LOS LEDS
    setOffSingle(WIFILED);          // Apagar el led
    setOffSingle(MQTTLED);          // Apagar el led
    setOffSingle(RELAY1);           // Apagar el relay
    setOffSingle(RELAY2);           // Apagar el relay   
    ledcWrite(ledChannel, 0);       // Poner el 0 el dimmer 255 
}

// -------------------------------------------------------------------
// Leer settings.json
// -------------------------------------------------------------------
boolean settingsRead(){
    DynamicJsonDocument jsonSettings(capacitySettings);
    File file = SPIFFS.open("/settings.json", "r");
    if(deserializeJson(jsonSettings, file)){
        settingsReset();      // llamar la función que formatea a valores por defecto
        DeserializationError error = deserializeJson(jsonSettings, file);
        log("ERROR","Falló la lectura de las configuraciones, tomando valores por defecto");
        if(error){
            log("ERROR", String(error.c_str()));
        }
        return false;
    }else{
        // -------------------------------------------------------------------
        // Relays settings.json
        // -------------------------------------------------------------------
        RELAY1_STATUS = jsonSettings["relay"]["RELAY1_STATUS"];
        RELAY2_STATUS = jsonSettings["relay"]["RELAY2_STATUS"];
        // -------------------------------------------------------------------
        // Dimmer settings.json
        // -------------------------------------------------------------------
        dim = jsonSettings["dimmer"]["dim_value"];
        // cerrar el archivo
        file.close();
        log("INFO", "Lectura de las configuraciones correctamente");
        return true;
    }
}
// -------------------------------------------------------------------
// Valores de Fábrica al settings.json
// -------------------------------------------------------------------
void settingsReset(){
    // -------------------------------------------------------------------
    // Relays settings.json
    // -------------------------------------------------------------------        
    RELAY1_STATUS = false;
    RELAY2_STATUS = false;
    // -------------------------------------------------------------------
    // Dimmer settings.json
    // -------------------------------------------------------------------
    dim = 0;
    log("INFO","Se reiniciaron todos los valores por defecto"); 
}
// -------------------------------------------------------------------
// Guardar settings.json
// -------------------------------------------------------------------
boolean settingsSave(){
    DynamicJsonDocument jsonSettings(capacitySettings);    
    File file = SPIFFS.open("/settings.json", "w+");
    if (file){
        // -------------------------------------------------------------------
        // Relays settings.json
        // -------------------------------------------------------------------
        JsonObject relayObj = jsonSettings.createNestedObject("relay");
        relayObj["RELAY1_STATUS"] = RELAY1_STATUS;
        relayObj["RELAY2_STATUS"] = RELAY2_STATUS;
        // -------------------------------------------------------------------
        // Dimmer settings.json
        // -------------------------------------------------------------------
        JsonObject dimmerObj = jsonSettings.createNestedObject("dimmer");
        dimmerObj["dim_value"] = dim;
        
        jsonSettings["file_version"] = "1.0.0";

        serializeJsonPretty(jsonSettings, file);
        file.close();
        log("INFO","Configuración guardada correctamente");
        serializeJsonPretty(jsonSettings, Serial);
        return true;
    }else{
        log("ERROR","Falló el guardado de la configuración");
        return false;
    }
}
// -------------------------------------------------------------------
// Genera un log personalizado en el puerto Serial
// type: INFO, WARNING, DANGER
// msg: Mensaje
// -------------------------------------------------------------------
void log(String type, String msg){ // [INFO] demo
    Serial.println("[" + type + "] " + msg);
}
// -------------------------------------------------------------------
// Definir la Plataforma
// Optiene la plataforma de hardware
// -------------------------------------------------------------------
String platform(){
#ifdef ARDUINO_ESP32_DEV
    return "ESP32";
#endif
}
// -------------------------------------------------------------------
// De HEX a String
// -------------------------------------------------------------------
String HexToStr(const unsigned long &h, const byte &l){
    String s;
    s = String(h, HEX);
    s.toUpperCase();
    s = ("00000000" + s).substring(s.length() + 8 - l);
    return s;
}
// -------------------------------------------------------------------
// Crear un ID unico desde la direccion MAC
// Retorna los ultimos 4 Bytes del MAC rotados
// -------------------------------------------------------------------
String UniqueID(){    
    char uniqueid[15]; 
    uint64_t chipid = ESP.getEfuseMac();           
    uint16_t chip = (uint16_t)(chipid >> 32);
    snprintf(uniqueid, 15, "%04X", chip);
    return uniqueid;
}
// -------------------------------------------------------------------
// Número de serie del Dispositivo único => ESP329B1C52100C3D
// -------------------------------------------------------------------
String DeviceID(){
    return String(platform())+ HexToStr(ESP.getEfuseMac(),8) + String(UniqueID());
}
// -------------------------------------------------------------------
// Retorna IPAddress en formato "n.n.n.n" de IP a String
// -------------------------------------------------------------------
String ipStr(const IPAddress &ip){    
    String sFn = "";
    for (byte bFn = 0; bFn < 3; bFn++){
        sFn += String((ip >> (8 * bFn)) & 0xFF) + ".";
    }
    sFn += String(((ip >> 8 * 3)) & 0xFF);
    return sFn;
}
// -------------------------------------------------------------------
// Crear un path para los Topicos en MQTT
// emqx/ESP329B1C52100C3D/#   +/# = > usuario/id/# => emqx/ESP329B1C52100C3D/status = true/false
// -------------------------------------------------------------------
String PathMqttTopic(String topic){
    return String(String(mqtt_user)+"/"+String(mqtt_id)+"/"+topic);
}
// -------------------------------------------------------------------
// Retorna segundos como "d:hh:mm:ss"
// -------------------------------------------------------------------
String longTimeStr(const time_t &t){        
    String s = String(t / SECS_PER_DAY) + ':';
    if (hour(t) < 10)
    {
        s += '0';
    }
    s += String(hour(t)) + ':';
    if (minute(t) < 10)
    {
        s += '0';
    }
    s += String(minute(t)) + ':';
    if (second(t) < 10)
    {
        s += '0';
    }
    s += String(second(t));
    return s;
}
// -------------------------------------------------------------------
// Retorna la calidad de señal WIFI en % => 0 - 100%
// -------------------------------------------------------------------
int getRSSIasQuality(int RSSI){
    int quality = 0;
    if(RSSI <= -100){
        quality = 0;
    }else if (RSSI >= -50){
        quality = 100;
    }else{
       quality = 2 * (RSSI + 100);
    }
    return quality;
}
// -------------------------------------------------------------------
// Leer la temperatura del sensor
// -------------------------------------------------------------------
void readSensor(){
    sensors.requestTemperatures();
    temperatureC = sensors.getTempCByIndex(0);
    temperatureF = sensors.getTempFByIndex(0);
}
// -------------------------------------------------------------------
// Sensor Temp Interno CPU
// -------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
// -------------------------------------------------------------------
// Retorna la temperatura del CPU
// -------------------------------------------------------------------
float TempCPUValue(){    
    return temp_cpu = (temprature_sens_read() - 32) / 1.8;
}




// -------------------------------------------------------------------
// Función para operar los Relay de forma Global -> API, MQTT, WS
// ejemplo: {"protocol": "MQTT", "output": "RELAY1", "value": true }
// ejemplo: {"protocol": "API", "output": "RELAY1", "value": true }
// -------------------------------------------------------------------
boolean apiPostOnOffRelays(String command){

    DynamicJsonDocument JsonCommand(320);
    deserializeJson(JsonCommand, command); 

    log("INFO", "Comando enviado desde: "+JsonCommand["protocol"].as<String>()+" <=> "+JsonCommand["output"].as<String>()+" <=> "+JsonCommand["value"].as<String>());

    if(JsonCommand["value"] == true){
        digitalWrite(JsonCommand["output"] == "RELAY1" ? RELAY1 : RELAY2, HIGH);
        JsonCommand["output"] == "RELAY1" ? RELAY1_STATUS = HIGH : RELAY2_STATUS = HIGH ;
        return true;
    }else if(JsonCommand["value"] == false ){
        digitalWrite(JsonCommand["output"] == "RELAY1" ? RELAY1 : RELAY2, LOW);
        JsonCommand["output"] == "RELAY1" ? RELAY1_STATUS = LOW : RELAY2_STATUS = LOW ;
        return false;
    }else{
        log("WARNING", "Comando NO permitido");
        return false;
    }
}

// -------------------------------------------------------------------
// Función para el dimmer dispositivo Global -> API, MQTT, WS
// ejemplo: {"protocol": "API", "output": "Dimmer", "value": 0-100 }
// -------------------------------------------------------------------
void apiPostDimmer(String dimmer){

    DynamicJsonDocument JsonDimmer(320);

    deserializeJson(JsonDimmer, dimmer);  

    log("INFO", "Comando enviado desde: "+JsonDimmer["protocol"].as<String>()+" => "+JsonDimmer["output"].as<String>()+" => "+JsonDimmer["value"].as<String>()+" %");

    dim = JsonDimmer["value"].as<int>();

    if( dim > 100 )  dim = 100;
    if( dim < 0   )  dim = 0;

    if(settingsSave())
        ledcWrite(ledChannel, dim * 2.55); 
        // multiplicamos por 2.55*100 para llegar a 255 que seria el maximo a 8bit = 3.3V
}