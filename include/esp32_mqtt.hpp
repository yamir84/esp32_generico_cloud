; /* -------------------------------------------------------------------
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
 
#include <PubSubClient.h>

// -------------------------------------------------------------------
// Definir valores a las variables MQTT
// -------------------------------------------------------------------
String      mqtt_willTopic          = PathMqttTopic("status");
String      mqtt_willMessage        = "{\"connected\": false, \"username\": \""+String(mqtt_user)+"\" }";
int         mqtt_willQoS            = 0;
boolean     mqtt_willRetain         = false;



WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

char topic[150];

String mqtt_data = "";

long lastMqttReconnectAttempt = 0;
long lasMsg = 0;

// -------------------------------------------------------------------
// DEFINICION DE FUNCIONES
// -------------------------------------------------------------------
boolean mqtt_connect();
void callback(char *topic, byte *payload, unsigned int length);
void mqttloop();
void mqtt_publish();
String Json();
void mqtt_response(String method, String type, String msg, String value);

// -------------------------------------------------------------------
// MQTT Connect
// -------------------------------------------------------------------
boolean mqtt_connect()
{
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);
    log("MQTT", "Intentando conexión al Broker MQTT ...");
    // https://pubsubclient.knolleary.net/api.html
    // https://www.amebaiot.com/zh/rtl8722dm-arduino-api-pubsubclient/

    // boolean connect (clientID)
    // boolean connect (clientID, willTopic, willQoS, willRetain,willMessage)
    // boolean connect (clientID, username, password)
    // boolean connect (clientID, username, password, willTopic, willQoS, willRetain, willMessage)
    // * boolean connect (clientID, username, password, willTopic, willQoS, willRetain, willMessage, cleanSession)

    /*
        Parámetros id: ID de cliente, un identificador de cadena único
        usuario: nombre de usuario para autenticación, valor predeterminado NULL
        pass: contraseña para autenticación, valor predeterminado NULL
        willTopic: el tema que utilizará el mensaje
        willQoS: la calidad del servicio que utilizará el mensaje will
        willRetain: si el testamento debe publicarse con el indicador de retención
        willMessage: la carga del mensaje del testamento
        cleanSession: Si cleansession se establece en true , se eliminarán todos los temas suscritos
    */

    if(mqttClient.connect(mqtt_id, mqtt_user, mqtt_password, mqtt_willTopic.c_str(), mqtt_willQoS, mqtt_willRetain, mqtt_willMessage.c_str())){
        log("INFO","Conectado al Broker MQTT -> "+ String(mqtt_server));

        mqttClient.setBufferSize(1024*5);  // definimos el buffer de mqtt a 5KB
        log("INFO", "Buffer MQTT Size: " +String(mqttClient.getBufferSize())+ " Bytes");

        String topic_subscribe = PathMqttTopic("command");
        topic_subscribe.toCharArray(topic, 150);
        // Nos suscribimos a comandos Topico: v1/device/usuario/dispositivo/comando
        // boolean subscribe (topic)
        // * boolean subscribe (topic, [qos])
        // qos - optional the qos to subscribe at (int: 0 or 1 only)
        // Función para suscribirnos al Topico
        if(mqttClient.subscribe(topic)){
            log("INFO","Suscrito al tópico: " + String(topic));
        }else{
            log("ERROR","MQTT - Falló la suscripción"); 
        }
        // int publish (topic, payload)
        //  * int publish (topic, payload, retained)
        // int publish (topic, payload, length, retained)
        // topic - the topic to publish to (const char[])
        // payload - the message to publish (const char[])
        // retained - informacion retenida (boolean)
        // length - the length of the message (byte)
        String mqtt_willMessageCon = "{\"connected\": true, \"username\": \""+String(mqtt_user)+"\" }";
        mqttClient.publish(mqtt_willTopic.c_str(), mqtt_willMessageCon.c_str() );
    }else{
        log("ERROR","MQTT - Falló, código de error = " + String(mqttClient.state()));
        return (0);
    }
    return (1);
}

// -------------------------------------------------------------------
// Manejo de los Mensajes Entrantes --- controlar los relays
// -------------------------------------------------------------------
void callback(char *topic, byte *payload, unsigned int length){
    String command = "";
    String str_topic(topic);

    for(int16_t i = 0; i < length; i++){
        command += (char)payload[i];
    }   
    command.trim();
    log("INFO","MQTT Tópico  --> " + str_topic);
    log("INFO","MQTT Mensaje --> " + command);
    // creación de un JSON para manejar la data string que llega
    DynamicJsonDocument JsonCommand(1024);
    DeserializationError error = deserializeJson(JsonCommand, command);
    // manejo de errores si no es un JSON
    if (error){
        mqtt_response("Desconocido", "Desconocido", "", "{\"msg\": \"¡Error, no es un formato JSON!\"}"); 
        return;
    }
    // manejo de errores si no es un JSON soportado
    if(!JsonCommand.containsKey("method") || !JsonCommand.containsKey("type")){
        mqtt_response("Desconocido", "Desconocido", "", "{\"msg\": \"¡Error, formato JSON no soportado!\" }"); 
        return;
    }

    if(strcmp(JsonCommand["method"], "POST") == 0 && strcmp(JsonCommand["type"], "RELAYS") == 0){
        // {"method": "POST", "type": "RELAYS", "data":{"protocol": "MQTT", "output": "RELAY1", "value": false }}
        if(apiPostOnOffRelays(JsonCommand["data"])){
            if(settingsSave()) {
                mqtt_response(JsonCommand["method"], JsonCommand["type"], JsonCommand["data"]["output"], "{\"value\": true}" );
                mqtt_publish();
            }
        }else{
            if(settingsSave()) {
                mqtt_response(JsonCommand["method"], JsonCommand["type"], JsonCommand["data"]["output"], "{\"value\": false}" );
                mqtt_publish();
            }
        }  
    }else if(strcmp(JsonCommand["method"], "POST") == 0 && strcmp(JsonCommand["type"], "DIMMER") == 0){
        // {"method": "POST", "type": "DIMMER", "data":{"protocol": "MQTT", "output": "Dimmer", "value": 50 }}
        apiPostDimmer(JsonCommand["data"]);
        // respuesta al cliente MQTT
        mqtt_response(JsonCommand["method"], JsonCommand["type"], JsonCommand["data"]["output"], "{ \"value\":"+ String(dim) +"}");
        mqtt_publish();
    }else{
       mqtt_response("Desconocido", "Desconocido", "", "{ \"msg\": \"¡Error, no es un comando soportado!\" }"); 
    }

}

// -------------------------------------------------------------------
// Manejo de los Mensajes de respuesta a los comandos por MQTT
// ------------------------------------------------------------------- 
void mqtt_response(String method, String type, String msg, String value){

    String data = "";
    DynamicJsonDocument jsonDoc(10240);
    // DynamicJsonDocument jsonData(10240);
    // String data = {\"value\": true };
    // deserializeJson(jsonData, data);
    // jsonData["value"] = true
    DynamicJsonDocument jsonData(10240);
    deserializeJson(jsonData, value);

    jsonDoc["method"] = method;
    jsonDoc["type"] = type;
    JsonObject dataObj = jsonDoc.createNestedObject("data");
    dataObj["msg"]= msg;
    dataObj["value"]= jsonData;
    serializeJson(jsonDoc, data);

    // preparar el mensaje para MQTT // usuario/serialnumber/response
    String topic = PathMqttTopic("response");
    mqttClient.publish(topic.c_str(), data.c_str());

}

// -------------------------------------------------------------------
// MQTT Loop Principal
// -------------------------------------------------------------------
void mqttloop(){
    if (!mqttClient.connected()){
        long now = millis();
        if ((now < 60000) || ((now - lastMqttReconnectAttempt) > 120000)){
            lastMqttReconnectAttempt = now;
            if(mqtt_connect()){
                lastMqttReconnectAttempt = 0;
            }
        }            
    }else{
        mqttClient.loop();
    }
}
// -------------------------------------------------------------------
// Manejo de los Mensajes Salientes
// ------------------------------------------------------------------- 
void mqtt_publish(){
    String topic = PathMqttTopic("device");
    mqtt_data = Json();
    mqttClient.publish(topic.c_str(), mqtt_data.c_str());
    mqtt_data = "";
}
// -------------------------------------------------------------------
// JSON con información del Dispositivo para envio por MQTT
// ------------------------------------------------------------------- 
String Json(){
    String response;
    DynamicJsonDocument jsonDoc(3000);
    // Capturar el valor de temperatura
    readSensor();
    // Crear JSON
    jsonDoc["deviceMqttId"]             = mqtt_id;
    jsonDoc["deviceSerial"]             = DeviceID();
    jsonDoc["deviceManufacturer"]       = device_manufacturer;
    jsonDoc["deviceFwVersion"]          = device_fw_version;
    jsonDoc["deviceHwVersion"]          = device_hw_version;
    jsonDoc["deviceSdk"]                = ESP.getSdkVersion();

    JsonObject dataObj                  = jsonDoc.createNestedObject("data");
	dataObj["deviceRamSizeKB"]          = ESP.getHeapSize()/1024;
	dataObj["deviceRamAvailableKB"]     = ESP.getFreeHeap()/1024;
    dataObj["deviceSpiffsSizeKB"]       = SPIFFS.totalBytes() / 1024;
    dataObj["deviceSpiffsUsedKB"]       = SPIFFS.usedBytes() / 1024;
    dataObj["deviceActiveTimeSeconds"]  = longTimeStr(millis() / 1000);
    dataObj["deviceCpuClockMhz"]        = getCpuFrequencyMhz();
    dataObj["deviceFlashSizeMB"]        = ESP.getFlashChipSize() / (1024.0 * 1024);

    dataObj["deviceRelay1Status"]       = RELAY1_STATUS ? true : false;
	dataObj["deviceRelay2Status"]       = RELAY2_STATUS ? true : false;
	dataObj["deviceDimmer"]             = dim;
	dataObj["deviceCpuTempC"]           = TempCPUValue();
	dataObj["deviceDS18B20TempC"]       = temperatureC;
    dataObj["deviceDS18B20TempF"]       = temperatureF;
    dataObj["deviceRestarts"]           = device_restart;
	dataObj["wifiRssiStatus"]           = WiFi.RSSI();
	dataObj["wifiQuality"]              = getRSSIasQuality(WiFi.RSSI());
	dataObj["wifiIPv4"]                 = ipStr(WiFi.localIP());

    jsonDoc["jsonVersion"]              = "1.0.0";

    serializeJson(jsonDoc, response);
    return response;
}

