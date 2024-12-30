#ifndef AJAX_H
#define AJAX_H

#include <Arduino.h>
#include <ArduinoJson.h>

void ajaxHandle();
void ajaxLoad(uint8_t page, JsonObject& jsonReply);
bool ajaxSave(uint8_t page, JsonObject& json);

#endif