#include "Arduino.h"
#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"
#include "ESPmDNS.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#define MOTOR_STEPS 200
#define MOTOR_C_RPM 60

#define MOTOR_Z_RPM 60

#define ENABLE_C 27
#define DIR_C 25
#define STEP_C 26

#define ENABLE_Z 33
#define DIR_Z 14
#define STEP_Z 32

#define MICROSTEPS 16

#define SERIAL_SPEED 115200

bool isProcessing = false;
int turns;
int gauge;

BasicStepperDriver stepperC(MOTOR_STEPS, DIR_C, STEP_C);
BasicStepperDriver stepperZ(MOTOR_STEPS, DIR_Z, STEP_Z);
SyncDriver controller(stepperZ, stepperC);

const char* ssid = "Spooler";
const char* password = NULL;
IPAddress local_IP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  startFiles();
  startWebServer();

  stepperC.begin(MOTOR_C_RPM, MICROSTEPS);
  stepperZ.begin(MOTOR_Z_RPM, MICROSTEPS);

  digitalWrite(ENABLE_C, LOW);
  digitalWrite(ENABLE_Z, LOW);
}

void startFiles() {
  bool filesAreOnline = SPIFFS.begin(true);
  if (!filesAreOnline) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  Serial.println("Files:");
  while(file){
      Serial.println(file.name());
      file = root.openNextFile();
  }
}

void startWebServer() {
  WiFi.persistent(false);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  MDNS.begin("spooler");
  Serial.println("WebServer started.");

  server
    .serveStatic("/", SPIFFS, "/")
    .setCacheControl("max-age=600")
    .setDefaultFile("index.html");

  server.on("/jog", HTTP_POST, [](AsyncWebServerRequest *request){
    int degrees = 1;
    if (request->hasParam("degrees"))
      degrees = request->getParam("degrees")->value().toInt();

    Serial.println("jog:" + String(degrees));
    stepperZ.rotate(degrees);
    request->send(200);
  });

  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("turns")) {
      turns = request->getParam("turns")->value().toInt();
    } else {
      request->send(SPIFFS, "/index.html", String());
      return;
    }

    if (request->hasParam("gauge")) {
      gauge = request->getParam("gauge")->value().toInt();
    } else {
      request->send(SPIFFS, "/index.html", String());
      return;
    }

    Serial.println("started");
    isProcessing = true;
    request->send(SPIFFS, "/index.html", String());
  });
  
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    isProcessing = false;
    Serial.println("stop");
    request->send(SPIFFS, "/index.html", String());
  });

  server.begin();
}

void loop() {
  if (!isProcessing) 
    return;

  for (int r = 0; r < turns; r++) {
    controller.rotate(1, gauge);
    if (!isProcessing) break;
  }
  isProcessing = false;
}
