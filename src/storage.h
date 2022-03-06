#ifndef STORAGE_H
#define STORAGE_H

#include "SPIFFS.h"

void StartStorage() {
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

#endif