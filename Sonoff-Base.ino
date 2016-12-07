/*
   1MB flash sizee
   sonoff header
   1 - vcc 3v3
   2 - rx
   3 - tx
   4 - gnd
   5 - gpio 14
   esp8266 connections
   gpio  0 - button
   gpio 12 - relay
   gpio 13 - green led - active low
   gpio 14 - pin 5 on header
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "appconfig.h"
#include "wificonfig.h"

/* ----------------------------------------------------------- */

#define SONOFF_BUTTON    0
#define SONOFF_RELAY    12
#define SONOFF_LED      13
#define SONOFF_INPUT    14

#define BUTTON          0
#define LED_PIN         13
#define LED_ON          LOW
#define LED_OFF         HIGH

/* ----------------------------------------------------------- */

WiFiServer server(80);

/* ----------------------------------------------------------- */

void setup() {

    Serial.begin(115200);
    Serial.println("Booting");

    setupOTA("SonoffBase");

    server.begin();
    Serial.println("Server started on port 80");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LED_ON);

    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

/* ----------------------------------------------------------- */

void loop() {

    delay(100);
    //tick();
    ArduinoOTA.handle();

    WiFiClient client = server.available();
    if (!client) {
        return;
    }
    
    // Wait until the client sends some data
    Serial.println("new client");
    while(!client.available()){
        delay(1);
    }
    
    // Read the first line of the request
    String req = client.readStringUntil('\r');
    Serial.print("Req: ");
    Serial.println(req);
    client.flush();
    
    // Match the request
    int val;
    if (req.indexOf("/gpio/0") != -1) {
        val = 0;
    }
    else if (req.indexOf("/gpio/1") != -1) {
        val = 1;
    }
    /*else {
        Serial.println("invalid request");
        client.stop();
        return;
    }*/
    
    // Set GPIO2 according to the request
    setLED(val);
    
    client.flush();
    
    // Prepare the response
    String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
    s += (val)?"high":"low";
    s += "</html>\n";
    
    // Send the response to the client
    client.print(s);
    delay(1);
    Serial.println("Client disonnected");
    
    // The client will actually be disconnected 
    // when the function returns and 'client' object is detroyed

}

void tick() {
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void setLED(int val) {
    digitalWrite(LED_PIN, !val);
}

void setupOTA(char* host) {
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }
    
    ArduinoOTA.setHostname(host);
    ArduinoOTA.onStart([]() {
        Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
}
