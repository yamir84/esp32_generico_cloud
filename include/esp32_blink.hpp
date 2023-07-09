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

bool ioBlink = false;
unsigned long milOld;
int rndTemp = 0;

// -------------------------------------------------------------------
// DEFINICION DE FUNCIONES
// -------------------------------------------------------------------
void setOffSingle(int _pin);
void setOnSingle(int _pin);
void setOnOffSingle(int _pin, bool status);
void blinkSingle(int speed, int _pin);
void blinkSingleAsy(int timeHigh, int timeLow, int _pin);
void blinkRandomSingle(int minTime, int maxTime, int _pin);

// -------------------------------------------------------------------
// Off un Led/Relay/Actuador
// -------------------------------------------------------------------
void setOffSingle(int _pin){
    digitalWrite(_pin, LOW); // 0
}
// -------------------------------------------------------------------
// On un Led/Relay/Actuador
// -------------------------------------------------------------------
void setOnSingle(int _pin){
    digitalWrite(_pin, HIGH); // 1
}
// -------------------------------------------------------------------
// On/Off un Led/Relay/Actuador segun Estados
// -------------------------------------------------------------------
void setOnOffSingle(int _pin, bool status){
    if(status){
        digitalWrite(_pin, HIGH); 
    }else{
        digitalWrite(_pin, LOW);
    }          
}
// -------------------------------------------------------------------
// Simple blinking function - Pestañeo para Alarmas tiempo variable
// -------------------------------------------------------------------
void blinkSingle(int speed, int _pin){
    if((milOld + speed) < millis()){
        milOld = millis();
        if(ioBlink == false){
            digitalWrite(_pin, HIGH);
            ioBlink = true;
        }else{
            digitalWrite(_pin, LOW);
            ioBlink = false;
        }
    }
}
// -------------------------------------------------------------------
// Parpadeo Asincrónico Tiempo On y Tiempo Off tiempo variable
// -------------------------------------------------------------------
void blinkSingleAsy(int timeHigh, int timeLow, int _pin){
    if(ioBlink == false){
        if((milOld + timeHigh) < millis()){
            milOld = millis();
            digitalWrite(_pin, LOW);
            ioBlink = true;
        }
    }else{
        if((milOld + timeLow) < millis()){
            milOld = millis();
            digitalWrite(_pin, HIGH);
            ioBlink = false;
        }
    }
}
// -------------------------------------------------------------------
// Blinking with randomised delay como para TX/RX de Datos
// -------------------------------------------------------------------
void blinkRandomSingle(int minTime, int maxTime, int _pin){
    if((milOld + rndTemp) < millis()){
        milOld = millis();
        rndTemp = random(minTime, maxTime);
        if(ioBlink == false){
            digitalWrite(_pin, HIGH);
            ioBlink = true;
        }else{
            digitalWrite(_pin, LOW);
            ioBlink = false;
        }
    }
}