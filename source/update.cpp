#include "update.h"
#include <ESP8266httpUpdate.h>
#include <ArduinoOTA.h>
#include "display.h"
#include "source.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

bool webUpdateAvail = 0;
const char updateServer[] = "http://www.dmxcube.eu/update";

void updateSetup()
{
	ArduinoOTA.onProgress([](int a, int b)
												{
		if((a*100/b)%10 == 0){
		u8g2.setFont(u8g2_font_5x7_mf);
		u8g2.drawStr(0, 50, "ArduinoOTA updating...");
		u8g2.drawFrame(0, 54, 128, 10);
			u8g2.drawBox(0, 54, (a * 128 / b), 10);
			u8g2.sendBuffer();
		} });
	ArduinoOTA.setHostname("cubeminiw");
	ArduinoOTA.begin(); // Starts OTA

	String serverName = updateServer + String("/checkUpdate.php") + params;

	WiFiClient client;
	HTTPClient http;

	http.begin(client, serverName.c_str());

	int httpResponseCode = http.GET();

	if (httpResponseCode > 0)
	{
		u8g2.clearDisplay();
		u8g2.drawStr(0, 10, "respcode: ");
		u8g2.drawStr(0, 20, String(httpResponseCode).c_str());
		String payload = http.getString();
		u8g2.drawStr(0, 30, String(payload).c_str());
		if (payload == "1")
		{
			u8g2.drawStr(0, 40, "Update available!");
		}
	}
	else
	{
		u8g2.drawStr(0, 10, "Error code: ");
		u8g2.drawStr(0, 20, String(httpResponseCode).c_str());
	}
	// Free resources
	http.end();
	u8g2.sendBuffer();
	delay(2000);
#warning SETUP DELAY
};
void updateLoop() {

};

bool checkForUpdate()
{
	webUpdateAvail = 1;
};

void doUpdate()
{
	if (webUpdateAvail)
	{
		ESPhttpUpdate.update(String(updateServer + String("/fetchUpdate.php") + params));
	}
};