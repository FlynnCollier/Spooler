#include "ESPmDNS.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"

#include "src/storage.h"
#include "src/spooler.h"

#define SERIAL_SPEED 115200

const char* ssid = "Spooler";
const char* password = NULL;
IPAddress local_IP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
AsyncWebServer server(80);
Spooler spooler;

void setup() {
  startStorage();
  startWebServer();
  spooler.Init();
}

void startWebServer() {
  WiFi.persistent(false);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  MDNS.begin("spooler");

  server
    .serveStatic("/", SPIFFS, "/")
    .setCacheControl("max-age=600")
    .setDefaultFile("index.html");

  server.on("/jog", HTTP_POST, [](AsyncWebServerRequest *request){
    int degrees = 1;
    if (request->hasParam("degrees"))
      degrees = request->getParam("degrees")->value().toInt();

    spooler.Jog(degrees);
    request->send(200);
  });

  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
    int gauge;
    if (request->hasParam("gauge")) {
      gauge = request->getParam("gauge")->value().toInt();
    } else {
      request->send(SPIFFS, "/index.html", String());
      return;
    }

    spooler.Start(gauge);
    request->send(SPIFFS, "/index.html", String());
  });
  
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    spooler.Stop();
    request->send(SPIFFS, "/index.html", String());
  });

  server.begin();
}

void loop() {
  spooler.Process();
}
