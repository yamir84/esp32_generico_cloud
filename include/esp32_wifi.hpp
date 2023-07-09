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

bool wifi_change                    = false;

unsigned long previousMillisWIFI    = 0;
unsigned long intervalWIFI          = 30000;            // 30 Segundos


// -------------------------------------------------------------------
// Declaración de funciones
// -------------------------------------------------------------------
void startClient();
void wifi_setup();
void wifiLoop();

// -------------------------------------------------------------------
// Iniciar WIFI Modo Estación
// -------------------------------------------------------------------
void startClient(){
    WiFi.disconnect(true);
    log("INFO","Iniciando Modo Estación");
    WiFi.mode(WIFI_STA);
    WiFi.hostname(DeviceID());
    WiFi.begin(wifi_ssid, wifi_password);
    log("INFO", "Conectando al SSID " + String(wifi_ssid));
    byte b = 0;
    while( WiFi.status() != WL_CONNECTED && b < 60){
        b++;
        log("WARNING","Intentando conexión WiFi ...");
        vTaskDelay(250);
        digitalWrite(WIFILED, HIGH);
        vTaskDelay(250);
        digitalWrite(WIFILED, LOW);
        blinkSingle(100, WIFILED);        
    }
    if(WiFi.status() == WL_CONNECTED){
        log("INFO","WiFi conectado (" + String(WiFi.RSSI()) + ") dBm IPv4 " + ipStr(WiFi.localIP()));
        blinkRandomSingle(10, 100, WIFILED);
        wifi_change = true;
    }else{
        log("ERROR","WiFi no conectado");        
        blinkRandomSingle(10, 100, WIFILED);
        wifi_change = true;
    }
}
// -------------------------------------------------------------------
// WIFI Setup
// -------------------------------------------------------------------
void wifi_setup(){
    WiFi.disconnect(true);
    startClient();
    if(WiFi.status() == WL_CONNECTED){
        log("INFO","WiFI Modo Estación");
    }
}
// -------------------------------------------------------------------
// Loop Modo Estación
// -------------------------------------------------------------------
byte w = 0;
void wifiLoop(){
    unsigned long currentMillis = millis();
    if(WiFi.status() != WL_CONNECTED && (currentMillis - previousMillisWIFI >= intervalWIFI)){
        w++;
        blinkSingle(100, WIFILED);
        previousMillisWIFI = currentMillis;
        // 2 = 1 minuto
        if(w == 2){
            log("INFO","WiFI Modo Estación");
            startClient(),
            wifi_change = true;
            w = 0;
        }else{
            log("WARNING","SSID " + String(wifi_ssid) + " desconectado ");
        }
    }else{
       blinkSingleAsy(10, 500, WIFILED); 
    }
}
