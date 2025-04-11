#include <Arduino.h>
#include <ESP8266httpUpdate.h>

void updateSetup();
void updateLoop();
bool checkForUpdate();
void getLatestVersion();
void doUpdate();

extern const char updateServer[];
#define params "?fw=" + FIRMWARE_VERSION + "&model=miniw"

extern bool webUpdateAvail;

extern bool isUpdating;

extern String latestFirm;