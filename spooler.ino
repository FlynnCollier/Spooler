#include "ESPAsyncWebServer.h"
#include "src/webServer.h"
#include "src/storage.h"
#include "src/spooler.h"

WebServer server;
Spooler spooler;

void setup() {
  startStorage();

  server.Start();
  wireUpServer();
  
  spooler.Init();
}

void wireUpServer() {

  server.webSite->on("/jog", HTTP_POST, [](AsyncWebServerRequest *request){
    int degrees = 1;
    if (request->hasParam("degrees"))
      degrees = request->getParam("degrees")->value().toInt();

    spooler.Jog(degrees);
    request->send(200);
  });

  server.webSite->on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
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
  
  server.webSite->on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    spooler.Stop();
    request->send(SPIFFS, "/index.html", String());
  });

  server.webSite->begin();
}

void loop() {
  spooler.Process();
}
