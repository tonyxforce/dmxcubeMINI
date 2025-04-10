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

String latestFirm = FIRMWARE_VERSION;

bool isUpdating = 0;

WiFiClient wificlient;

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

	ESPhttpUpdate.rebootOnUpdate(true);

	ESPhttpUpdate.onProgress([](int a, int b)
													 {
	webServer.handleClient();
u8g2.setFont(u8g2_font_5x7_mf);
u8g2.drawStr(0, 50, "Automatic updating...");
u8g2.drawFrame(0, 54, 128, 10);
u8g2.drawBox(0, 54, (a * 128 / b), 10);
u8g2.sendBuffer(); });
};

bool checkForUpdate()
{
	if(WiFi.status() != WL_CONNECTED) return false;
	
	String serverName = updateServer + String("/checkUpdate.php") + params;

	HTTPClient http;

	http.begin(wificlient, serverName.c_str());

	int httpResponseCode = http.GET();

	if (httpResponseCode > 0)
	{
		u8g2.clearBuffer();
		String payload = http.getString();
		if (payload == "1")
		{
			webUpdateAvail = true;
		}
	}
	else
	{
		u8g2.drawStr(0, 10, "Error checking for updates: ");
		u8g2.drawStr(0, 20, String(httpResponseCode).c_str());
		u8g2.sendBuffer();
		delay(2000);
	}
	// Free resources
	http.end();

	if (webUpdateAvail)
	{
		getLatestVersion();
	}

	return webUpdateAvail;
};

void doUpdate()
{
	if (webUpdateAvail)
	{
		u8g2.drawStr(0, 10, "Starting update...");
		u8g2.sendBuffer();
		u8g2.clearBuffer();
		HTTPUpdateResult ret = ESPhttpUpdate.update(wificlient, String(updateServer + String("/fetchUpdate.php") + params));
		u8g2.drawStr(0, 10, "Error during updating!");
		switch (ret)
		{
		case HTTP_UPDATE_FAILED:
			u8g2.drawStr(0, 20, "HTTP_UPDATE_FAILED");
			u8g2.drawStr(0, 30, String(ESPhttpUpdate.getLastError()).c_str());
			u8g2.drawStr(0, 40, String(ESPhttpUpdate.getLastErrorString()).c_str());
			u8g2.sendBuffer();
			break;

		case HTTP_UPDATE_NO_UPDATES:
			u8g2.drawStr(0, 20, "HTTP_UPDATE_NO_UPDATES");
			u8g2.sendBuffer();
			break;
		case HTTP_UPDATE_OK:
			u8g2.drawStr(0, 20, "OK");
			u8g2.sendBuffer();
			break;
		};
		delay(10000);
	}
	else
	{
		u8g2.drawStr(0, 10, "No new version!");
		u8g2.sendBuffer();
		delay(2000);
	}
};

void getLatestVersion()
{
	String serverName = updateServer + String("/getLatestVersion.php") + params;

	HTTPClient http;

	http.begin(wificlient, serverName.c_str());

	int httpResponseCode = http.GET();

	if (httpResponseCode > 0)
	{
		latestFirm = http.getString();
	}
	// Free resources
	http.end();
};