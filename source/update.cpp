#include "update.h"
#include <ESP8266httpUpdate.h>
#include <ArduinoOTA.h>
#include "display.h"
#include "source.h"

void updateSetup(){
	ArduinoOTA.onProgress([](int a, int b)
	{
		u8g2.setFont(u8g2_font_5x7_mf);
		u8g2.drawStr(0, 50, "ArduinoOTA updating...");
		u8g2.drawFrame(0, 54, 128, 10);
		u8g2.drawBox(0, 54, (a * 128 / b), 10);
		u8g2.sendBuffer();
	});
ArduinoOTA.setHostname("cubeminiw");
ArduinoOTA.begin(); // Starts OTA

;

};
void updateLoop(){

};