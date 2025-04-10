#include <Arduino.h>
#include <ESP8266httpUpdate.h>

void updateSetup();
void updateLoop();
bool checkForUpdate();
void doUpdate();

extern const char updateServer[];
#define params "?fw=" + FIRMWARE_VERSION + "&model=miniw"

extern bool webUpdateAvail;