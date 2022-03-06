#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "ESPmDNS.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "storage.h"

const char* ssid = "Spooler";
const char* password = NULL;
const int port = 80;
IPAddress local_IP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

class WebServer {
    public:
        AsyncWebServer* webSite;

        WebServer() {
            webSite = new AsyncWebServer(port);
        }

        void Start() {
            WiFi.persistent(false);
            WiFi.softAPConfig(local_IP, gateway, subnet);
            WiFi.softAP(ssid, password);
            MDNS.begin("spooler");

            webSite
                ->serveStatic("/", SPIFFS, "/")
                .setCacheControl("max-age=600")
                .setDefaultFile("index.html");
        }
};

#endif