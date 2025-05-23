/*
ESP8266_ArtNetNode v2.0.0
Copyright (c) 2016, Matthew Tong
https://github.com/mtongnz/ESP8266_ArtNetNode_v2

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.
If not, see http://www.gnu.org/licenses/
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "source.h"
#include "store.h"
#include "ajax.h"
#include "IPHelper.h"
#include "display.h"
#include "startFunctions.h"
#include "update.h"
#include <ArduinoOTA.h>

void doNodeReport()
{
	if (nextNodeReport > millis())
		return;

	char c[ARTNET_NODE_REPORT_LENGTH];

	if (nodeErrorTimeout > millis())
		nextNodeReport = millis() + 2000;
	else
		nextNodeReport = millis() + 5000;

	if (nodeError[0] != '\0' && !nodeErrorShowing && nodeErrorTimeout > millis())
	{

		nodeErrorShowing = true;
		strcpy(c, nodeError);
	}
	else
	{
		nodeErrorShowing = false;

		strcpy(c, "OK: PortA:");

		switch (deviceSettings.portAmode)
		{
		case TYPE_DMX_OUT:
			sprintf(c, "%s DMX Out", c);
			break;

		case TYPE_RDM_OUT:
			sprintf(c, "%s RDM Out", c);
			break;

		case TYPE_DMX_IN:
			sprintf(c, "%s DMX In", c);
			break;
		}

#ifndef ONE_PORT
		sprintf(c, "%s. PortB:", c);

		switch (deviceSettings.portBmode)
		{
		case TYPE_DMX_OUT:
			sprintf(c, "%s DMX Out", c);
			break;

		case TYPE_RDM_OUT:
			sprintf(c, "%s RDM Out", c);
			break;
		}
#endif
	}

	artRDM.setNodeReport(c, ARTNET_RC_POWER_OK);
}

void portSetup()
{
	if (deviceSettings.portAmode == TYPE_DMX_OUT || deviceSettings.portAmode == TYPE_RDM_OUT)
	{
#ifndef ESP_01
		setStatusLed(STATUS_LED_A, BLUE);
#endif

		dmxA.begin(DMX_DIR_A, artRDM.getDMX(portA[0], portA[1]));
		if (deviceSettings.portAmode == TYPE_RDM_OUT && !dmxA.rdmEnabled())
		{
			dmxA.rdmEnable(ESTA_MAN, ESTA_DEV);
			dmxA.rdmSetCallBack(rdmReceivedA);
			dmxA.todSetCallBack(sendTodA);
		}
	}
	else if (deviceSettings.portAmode == TYPE_DMX_IN)
	{
#ifndef ESP_01
		setStatusLed(STATUS_LED_A, CYAN);
#endif

		dmxA.begin(DMX_DIR_A, artRDM.getDMX(portA[0], portA[1]));
		dmxA.dmxIn(true);
		dmxA.setInputCallback(dmxIn);

		dataIn = (byte *)os_malloc(sizeof(byte) * 512);
		memset(dataIn, 0, 512);
	}
	else if (deviceSettings.portAmode == TYPE_WS2812)
	{
#ifndef ESP_01
		setStatusLed(STATUS_LED_A, GREEN);
#endif

		digitalWrite(DMX_DIR_A, HIGH);
	}

#ifndef ONE_PORT
	if (deviceSettings.portBmode == TYPE_DMX_OUT || deviceSettings.portBmode == TYPE_RDM_OUT)
	{
		setStatusLed(STATUS_LED_B, BLUE);

		dmxB.begin(DMX_DIR_B, artRDM.getDMX(portB[0], portB[1]));
		if (deviceSettings.portBmode == TYPE_RDM_OUT && !dmxB.rdmEnabled())
		{
			dmxB.rdmEnable(ESTA_MAN, ESTA_DEV);
			dmxB.rdmSetCallBack(rdmReceivedB);
			dmxB.todSetCallBack(sendTodB);
		}
	}
	else if (deviceSettings.portBmode == TYPE_WS2812)
	{
		setStatusLed(STATUS_LED_B, GREEN);

		digitalWrite(DMX_DIR_B, HIGH);
	}
#endif

	pixDriver.allowInterruptSingle = WS2812_ALLOW_INT_SINGLE;
	pixDriver.allowInterruptDouble = WS2812_ALLOW_INT_DOUBLE;
}

void artStart()
{
	// Initialise out ArtNet
	if (isHotspot)
		artRDM.init(deviceSettings.hotspotIp, deviceSettings.hotspotSubnet, true, deviceSettings.nodeName, deviceSettings.longName, ARTNET_OEM, ESTA_MAN, MAC_array);
	else
		artRDM.init(deviceSettings.ip, deviceSettings.subnet, deviceSettings.dhcpEnable, deviceSettings.nodeName, deviceSettings.longName, ARTNET_OEM, ESTA_MAN, MAC_array);

	// Set firmware
	artRDM.setFirmwareVersion(ART_FIRM_VERSION);

	// Add Group
	portA[0] = artRDM.addGroup(deviceSettings.portAnet, deviceSettings.portAsub);

	bool e131 = (deviceSettings.portAprot == PROT_ARTNET_SACN) ? true : false;

	// WS2812 uses TYPE_DMX_OUT - the rest use the value assigned
	if (deviceSettings.portAmode == TYPE_WS2812)
		portA[1] = artRDM.addPort(portA[0], 0, deviceSettings.portAuni, TYPE_DMX_OUT, deviceSettings.portAmerge);
	else
		portA[1] = artRDM.addPort(portA[0], 0, deviceSettings.portAuni, deviceSettings.portAmode, deviceSettings.portAmerge);

	artRDM.setE131(portA[0], portA[1], e131);
	artRDM.setE131Uni(portA[0], portA[1], deviceSettings.portAsACNuni);

	
#ifndef ONE_PORT
	// Add Group
	portB[0] = artRDM.addGroup(deviceSettings.portBnet, deviceSettings.portBsub);
	e131 = (deviceSettings.portBprot == PROT_ARTNET_SACN) ? true : false;

	// WS2812 uses TYPE_DMX_OUT - the rest use the value assigned
	if (deviceSettings.portBmode == TYPE_WS2812)
		portB[1] = artRDM.addPort(portB[0], 0, deviceSettings.portBuni, TYPE_DMX_OUT, deviceSettings.portBmerge);
	else
		portB[1] = artRDM.addPort(portB[0], 0, deviceSettings.portBuni, deviceSettings.portBmode, deviceSettings.portBmerge);

	artRDM.setE131(portB[0], portB[1], e131);
	artRDM.setE131Uni(portB[0], portB[1], deviceSettings.portBsACNuni);

	#endif

	// Add required callback functions
	artRDM.setArtDMXCallback(dmxHandle);
	artRDM.setArtRDMCallback(rdmHandle);
	artRDM.setArtSyncCallback(syncHandle);
	artRDM.setArtIPCallback(ipHandle);
	artRDM.setArtAddressCallback(addressHandle);
	artRDM.setTODRequestCallback(todRequest);
	artRDM.setTODFlushCallback(todFlush);

	switch (resetInfo.reason)
	{
	case REASON_DEFAULT_RST: // normal start
	case REASON_EXT_SYS_RST:
	case REASON_SOFT_RESTART:
		artRDM.setNodeReport(strdup("OK: Device started"), ARTNET_RC_POWER_OK);
		nextNodeReport = millis() + 4000;
		break;

	case REASON_WDT_RST:
		artRDM.setNodeReport(strdup("ERROR: (HWDT) Unexpected device restart"), ARTNET_RC_POWER_FAIL);
		strcpy(nodeError, "Restart error: HWDT");
		nextNodeReport = millis() + 10000;
		nodeErrorTimeout = millis() + 30000;
		break;
	case REASON_EXCEPTION_RST:
		artRDM.setNodeReport(strdup("ERROR: (EXCP) Unexpected device restart"), ARTNET_RC_POWER_FAIL);
		strcpy(nodeError, "Restart error: EXCP");
		nextNodeReport = millis() + 10000;
		nodeErrorTimeout = millis() + 30000;
		break;
	case REASON_SOFT_WDT_RST:
		artRDM.setNodeReport(strdup("ERROR: (SWDT) Unexpected device restart"), ARTNET_RC_POWER_FAIL);
		strcpy(nodeError, "Error on Restart: SWDT");
		nextNodeReport = millis() + 10000;
		nodeErrorTimeout = millis() + 30000;
		break;
	case REASON_DEEP_SLEEP_AWAKE:
		// not used
		break;
	}

	// Start artnet
	artRDM.begin();

	yield();
}

void webStart()
{
	webServer.on("/", []()
							 {
    artRDM.pause();
    webServer.send_P(200, typeHTML, mainPage);
    webServer.sendHeader("Connection", "close");
    yield();
    artRDM.begin(); });

	webServer.on("/style.css", []()
							 {
								 // artRDM.pause();

								 File f = SPIFFS.open("/style.css", "r");

								 // If no style.css in SPIFFS, send default
								 if (!f)
									 webServer.send_P(200, typeCSS, css);
								 else
									 webServer.send(200, typeCSS, f.readString());
								 // webServer.streamFile(f, typeCSS);

								 f.close();
								 webServer.sendHeader("Connection", "close");

								 yield();
								 // artRDM.begin();
							 });

	webServer.on("/ajax", HTTP_POST, ajaxHandle);

	webServer.on("/style", []()
							 {
    webServer.send_P(200, typeHTML, cssUploadPage);
    webServer.sendHeader("Connection", "close"); });

	webServer.on("/script.js", []()
							 {
    webServer.send_P(200, "text/javascript", scriptJs);
    webServer.sendHeader("Connection", "close"); });

	webServer.on("/favicon.svg", []()
							 {
		webServer.send_P(200, "image/svg+xml", favicon);
		webServer.sendHeader("Connection", "close"); });

	webServer.on("/style_delete", []()
							 {
    if (SPIFFS.exists("/style.css"))
      SPIFFS.remove("/style.css");
        
    webServer.send(200, "text/plain", "style.css deleted.  The default style is now in use.");
    webServer.sendHeader("Connection", "close"); });

	webServer.on("/style_upload", HTTP_POST, []()
							 { webServer.send(200, "text/plain", "Upload successful!"); }, []()
							 {
    HTTPUpload& upload = webServer.upload();
    
    if(upload.status == UPLOAD_FILE_START){
      String filename = upload.filename;
      if(!filename.startsWith("/")) filename = "/"+filename;
      fsUploadFile = SPIFFS.open(filename, "w");
      filename = String();
      
    } else if(upload.status == UPLOAD_FILE_WRITE){
      if(fsUploadFile)
        fsUploadFile.write(upload.buf, upload.currentSize);
        
    } else if(upload.status == UPLOAD_FILE_END){
      if(fsUploadFile) {
        fsUploadFile.close();
        
        //if (upload.filename != "/style.css")
        //  SPIFFS.rename(upload.filename, "/style.css");
      }
    } });

	webServer.on("/cert_upload", HTTP_POST, []()
							 { webServer.send(200, "text/plain", "Upload successful!"); }, []()
							 {
    HTTPUpload& upload = webServer.upload();
    
    if(upload.status == UPLOAD_FILE_START){
      String filename = upload.filename;
      if(!filename.startsWith("/")) filename = "/"+filename;
      fsUploadFile = SPIFFS.open(filename, "w");
      filename = String();
      
    } else if(upload.status == UPLOAD_FILE_WRITE){
      if(fsUploadFile)
        fsUploadFile.write(upload.buf, upload.currentSize);
        
    } else if(upload.status == UPLOAD_FILE_END){
      if(fsUploadFile) {
        fsUploadFile.close();
      }
    } });

	webServer.onNotFound([]()
											 { webServer.send(404, "text/plain", "Page not found"); });

	webServer.begin();

	yield();
}

void wifiStart()
{
	// If it's the default WiFi SSID, make it unique
	if (strcmp(deviceSettings.hotspotSSID, "DMXCube mini") == 0 || deviceSettings.hotspotSSID[0] == '\0')
		sprintf(deviceSettings.hotspotSSID, "DMXCube mini_%05u", (ESP.getChipId() & 0xFF));

	if (deviceSettings.standAloneEnable)
	{
		startHotspot();

		deviceSettings.ip = deviceSettings.hotspotIp;
		deviceSettings.subnet = deviceSettings.hotspotSubnet;
		deviceSettings.broadcast = getBroadcastIP(deviceSettings.subnet, deviceSettings.ip);

		return;
	}

	if (deviceSettings.wifiSSID[0] != '\0')
	{
		if (deviceSettings.wpa2Enterprise)
		{
			wifi_set_opmode(STATION_MODE);
			struct station_config wifi_config;
			memset(&wifi_config, 0, sizeof(wifi_config));
			strcpy((char *)wifi_config.ssid, deviceSettings.wifiSSID);
			wifi_station_set_config(&wifi_config);
			// DISABLE authentication using certificates - But risk leaking your password to someone claiming to be "eduroam"
			wifi_station_clear_cert_key();
			wifi_station_clear_enterprise_ca_cert();
			// Authenticate using username/password
			wifi_station_set_wpa2_enterprise_auth(1);
			wifi_station_set_enterprise_identity((uint8 *)deviceSettings.wifiUsername, strlen(deviceSettings.wifiUsername));
			wifi_station_set_enterprise_username((uint8 *)deviceSettings.wifiUsername, strlen(deviceSettings.wifiUsername));
			wifi_station_set_enterprise_password((uint8 *)deviceSettings.wifiPass, strlen(deviceSettings.wifiPass));

			wifi_station_connect();
		}
		else
		{
			WiFi.hostname(deviceSettings.hostName);
			wifi_station_set_hostname(deviceSettings.hostName);
			WiFi.begin(deviceSettings.wifiSSID, deviceSettings.wifiPass);
			WiFi.mode(WIFI_STA);
		}
		WiFi.hostname(deviceSettings.nodeName);

		unsigned long startTime = millis();
		unsigned long length = (deviceSettings.hotspotDelay * 1000);
		unsigned long endTime = startTime + length;
		if (deviceSettings.dhcpEnable)
		{
			int j = 0;
			u8g2.setCursor(0, 20);
			while (WiFi.status() != WL_CONNECTED && endTime > millis())
			{
				yield();
				wl_status_t status = WiFi.status();
				if (status == WL_CONNECT_FAILED)
				{
					startHotspot();
					break;
				}
				u8g2.setFont(u8g2_font_5x7_mf);
				u8g2.drawStr(0, 10, String("Connecting to " + String(deviceSettings.wifiSSID) + "...").c_str());
				unsigned long elapsedTime = millis() - startTime;

				// for(int i = 0;i<((elapsedTime*32)/length);i++)
				//	u8g2.drawStr((i*4)%128, 20+(round(i/32)*10), ".");
				u8g2.drawFrame(0, 12, 128, 10);
				u8g2.drawBox(0, 12, (elapsedTime * 128) / length, 10);
				u8g2.drawStr(0, 30, String(String("Starting hotspot in ") + String((length - elapsedTime) / 1000) + String("s...")).c_str());
				if (deviceSettings.wpa2Enterprise)
					u8g2.drawStr(0, 40, "WPA2 Enterprise");
				u8g2.sendBuffer();
				u8g2.clearBuffer();
				j++;
			}
			if (millis() >= endTime)
				startHotspot();

			deviceSettings.ip = WiFi.localIP();
			deviceSettings.subnet = WiFi.subnetMask();

			if (deviceSettings.gateway == INADDR_NONE)
				deviceSettings.gateway = WiFi.gatewayIP();

			deviceSettings.broadcast = getBroadcastIP(deviceSettings.subnet, deviceSettings.ip);
		}
		else
			WiFi.config(deviceSettings.ip, deviceSettings.gateway, deviceSettings.subnet);

		// sprintf(wifiStatus, "Wifi connected.  Signal: %ld<br />SSID: %s", WiFi.RSSI(), deviceSettings.wifiSSID);
		sprintf(wifiStatus, "Wifi connected.<br />SSID: %s", deviceSettings.wifiSSID);
		WiFi.macAddress(MAC_array);
	}
	else
		startHotspot();

	yield();
}

void startHotspot()
{
	yield();

	if (strcmp(deviceSettings.hotspotSSID, "DMXCube mini W") == 0)
	{
		// If the hotspot SSID is the default, make it unique
		char ssidBuffer[24];
		snprintf(ssidBuffer, sizeof(ssidBuffer), " %04X", system_get_chip_id());
		strcpy(deviceSettings.hotspotSSID, String(deviceSettings.hotspotSSID + String(ssidBuffer)).c_str());
	}
	WiFi.mode(WIFI_AP);
	WiFi.softAP(deviceSettings.hotspotSSID, deviceSettings.hotspotPass);
	WiFi.softAPConfig(deviceSettings.hotspotIp, deviceSettings.hotspotIp, deviceSettings.hotspotSubnet);

	sprintf(wifiStatus, "No Wifi. Hotspot started.<br />\nHotspot SSID: %s", deviceSettings.hotspotSSID);
	WiFi.macAddress(MAC_array);

	isHotspot = true;

	if (deviceSettings.standAloneEnable)
		return;

	webStart();
	/*
		unsigned long endTime = millis() + 30000;
		unsigned long lastUpdate = millis();
		// Stay here if not in stand alone mode - no dmx or artnet
		while ((endTime + (encoderPos * 1000)) > millis() || wifi_softap_get_station_num() > 0)
		{


			webServer.handleClient();
			yield();
		}

		ESP.restart();
		isHotspot = false;
		*/
}