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
#include "source.h"
#include <ArduinoJson.h>
#include "store.h"
#include "update.h"
#include "IPHelper.h"

bool _doUpdate = 0;

void ajaxHandle()
{
	DynamicJsonDocument jsonRequestDoc(512);
	JsonObject jsonRequest = jsonRequestDoc.to<JsonObject>();
	DeserializationError err = deserializeJson(jsonRequestDoc, String(webServer.arg("plain")).c_str());

	DynamicJsonDocument jsonReplyDoc(4096);
	JsonObject jsonReply = jsonReplyDoc.to<JsonObject>();

	if (err)
	{
		jsonReplyDoc["err"] = err.f_str();
	};
	String reply;
	jsonReply["message"] = "";

	if (jsonRequest.containsKey("success") && jsonRequest["success"] == 1 && jsonRequest.containsKey("page"))
	{
		if (ajaxSave((uint8_t)jsonRequest["page"], jsonRequest, jsonRequestDoc))
		{
			ajaxLoad((uint8_t)jsonRequest["page"], jsonReply, jsonReplyDoc, jsonRequest, jsonRequestDoc);

			if (jsonReply["message"] == "")
				jsonReply["message"] = "Settings Saved";
		}
		else
		{
			jsonReply["success"] = 0;
			jsonReply["message"] = "Failed to save data. Reload page and try again.";
		}

		// Handle reboots
	}
	else if (jsonRequest.containsKey("success") && jsonRequest.containsKey("reboot") && jsonRequest["reboot"] == 1)
	{
		jsonReply["success"] = 1;
		jsonReply["message"] = "Device Restarting.";

		doReboot = true;
	}

	// jsonReply.printTo(reply);
	serializeJson(jsonReply, reply);
	webServer.sendHeader("Access-Control-Allow-Origin", "*");
	webServer.send(200, "application/json", reply);

	if (_doUpdate)
	{
		doUpdate();
	}
}

bool ajaxSave(uint8_t page, JsonObject jsonRequest, DynamicJsonDocument jsonRequestDoc)
{
	// This is a load request, not a save
	if (jsonRequest.size() == 2)
		return true;

	switch (page)
	{
	case 1: // Device Status
	{
		// We don't need to save anything for this.  Go straight to load
		return true;
		break;
	}

	case 2: // Wifi
	{
		strcpy(deviceSettings.wifiUsername, jsonRequest["wifiUsername"]);
		strcpy(deviceSettings.wifiSSID, jsonRequest["wifiSSID"]);
		strcpy(deviceSettings.wifiPass, jsonRequest["wifiPass"]);
		strcpy(deviceSettings.hotspotSSID, jsonRequest["hotspotSSID"]);
		strcpy(deviceSettings.hotspotPass, jsonRequest["hotspotPass"]);
		deviceSettings.hotspotDelay = (uint8_t)jsonRequest["hotspotDelay"];
		deviceSettings.standAloneEnable = (bool)jsonRequest["standAloneEnable"];
		deviceSettings.wpa2Enterprise = (bool)jsonRequest["wpa2Enterprise"];

		eepromSave();

		if (!deviceSettings.standAloneEnable)
			doReboot = true;
		return true;
		break;
	}

	case 3: // IP Address & Node Name
	{
		deviceSettings.ip = IPAddress(jsonRequest["ipAddress"][0], jsonRequest["ipAddress"][1], jsonRequest["ipAddress"][2], jsonRequest["ipAddress"][3]);
		deviceSettings.subnet = IPAddress(jsonRequest["subAddress"][0], jsonRequest["subAddress"][1], jsonRequest["subAddress"][2], jsonRequest["subAddress"][3]);
		deviceSettings.gateway = IPAddress(jsonRequest["gwAddress"][0], jsonRequest["gwAddress"][1], jsonRequest["gwAddress"][2], jsonRequest["gwAddress"][3]);
		deviceSettings.broadcast = uint32_t(deviceSettings.ip) | (~uint32_t(deviceSettings.subnet));
		// deviceSettings.broadcast = {~deviceSettings.subnet[0] | (deviceSettings.ip[0] & deviceSettings.subnet[0]), ~deviceSettings.subnet[1] | (deviceSettings.ip[1] & deviceSettings.subnet[1]), ~deviceSettings.subnet[2] | (deviceSettings.ip[2] & deviceSettings.subnet[2]), ~deviceSettings.subnet[3] | (deviceSettings.ip[3] & deviceSettings.subnet[3])};

		if (!isHotspot && (bool)jsonRequest["dhcpEnable"] != deviceSettings.dhcpEnable)
		{

			if ((bool)jsonRequest["dhcpEnable"])
			{
				/*
				// Re-enable DHCP
				WiFi.begin(deviceSettings.wifiSSID, deviceSettings.wifiPass);

				// Wait for an IP
				while (WiFi.status() != WL_CONNECTED)
					yield();

				// Save settings to struct
				deviceSettings.ip = WiFi.localIP();
				deviceSettings.subnet = WiFi.subnetMask();
				deviceSettings.broadcast = {~deviceSettings.subnet[0] | (deviceSettings.ip[0] & deviceSettings.subnet[0]), ~deviceSettings.subnet[1] | (deviceSettings.ip[1] & deviceSettings.subnet[1]), ~deviceSettings.subnet[2] | (deviceSettings.ip[2] & deviceSettings.subnet[2]), ~deviceSettings.subnet[3] | (deviceSettings.ip[3] & deviceSettings.subnet[3])};
				*/

				deviceSettings.gateway = INADDR_NONE;
			}
			/*
			else {
				// Set static IP
				WiFi.config(deviceSettings.ip,deviceSettings.ip,deviceSettings.ip,deviceSettings.subnet);
			}

			// Add any changes to artnet settings - this will send correct artpollreply
			artRDM.setIP(deviceSettings.ip, deviceSettings.subnet);
			artRDM.setDHCP(deviceSettings.dhcpEnable);
			*/
		}

		if (!isHotspot)
		{
			artRDM.setShortName(deviceSettings.nodeName);
			artRDM.setLongName(deviceSettings.longName);
		}

		deviceSettings.dhcpEnable = (bool)jsonRequest["dhcpEnable"];

		deviceSettings.allowOTA = (bool)jsonRequest["allowOTA"];
		deviceSettings.autoRefresh = (bool)jsonRequest["autoRefresh"];
		strcpy(deviceSettings.hostName, jsonRequest["hostName"]);

		eepromSave();
		return true;
		break;
	}

	case 4: // Port A
	{
		deviceSettings.portAprot = (uint8_t)jsonRequest["portAprot"];
		bool e131 = (deviceSettings.portAprot == PROT_ARTNET_SACN) ? true : false;

		deviceSettings.portAmerge = (uint8_t)jsonRequest["portAmerge"];

		if ((uint8_t)jsonRequest["portAnet"] < 128)
			deviceSettings.portAnet = (uint8_t)jsonRequest["portAnet"];

		if ((uint8_t)jsonRequest["portAsub"] < 16)
			deviceSettings.portAsub = (uint8_t)jsonRequest["portAsub"];

		if ((uint8_t)jsonRequest["portAuni"] < 16)
			deviceSettings.portAuni = (uint8_t)jsonRequest["portAuni"];

		if ((uint16_t)jsonRequest["portAsACNuni"] > 0 && (uint16_t)jsonRequest["portAsACNuni"] < 64000)
			deviceSettings.portAsACNuni = (uint16_t)jsonRequest["portAsACNuni"];

		artRDM.setE131(portA[0], portA[1], e131);
		artRDM.setE131Uni(portA[0], portA[1], deviceSettings.portAsACNuni);

		uint8_t newMode = jsonRequest["portAmode"];
		uint8_t oldMode = deviceSettings.portAmode;

#ifndef ONE_PORT
		// RDM and DMX input can't run together
		if (newMode == TYPE_DMX_IN && deviceSettings.portBmode == TYPE_RDM_OUT)
		{
			deviceSettings.portBmode = TYPE_DMX_OUT;
			dmxB.rdmDisable();
		}
#endif

		if (newMode == TYPE_DMX_IN && jsonRequest.containsKey("dmxInBroadcast"))
			deviceSettings.dmxInBroadcast = IPAddress(jsonRequest["dmxInBroadcast"][0], jsonRequest["dmxInBroadcast"][1], jsonRequest["dmxInBroadcast"][2], jsonRequest["dmxInBroadcast"][3]);

		if (newMode != oldMode)
		{

			// Store the nem mode to settings
			deviceSettings.portAmode = newMode;

			doReboot = true;
			/*
			if (oldMode == TYPE_WS2812) {
				doReboot = true;

				// Set pixel strip length to zero
				pixDriver.updateStrip(0, 0, deviceSettings.portApixConfig);

				// Close ports from pixels - library handles if they dont exist
				for (uint8_t x = 2; x <= 4; x++)
					artRDM.closePort(portA[0], portA[x]);

				// Start our DMX port
				dmxA.begin(DMX_DIR_A, artRDM.getDMX(portA[0], portA[1]));

			} else if (oldMode == TYPE_DMX_IN)
				dmxA.dmxIn(false);
			else if (oldMode == TYPE_RDM_OUT)
				dmxA.rdmDisable();

			// Start DMX output with no DMX
			if (newMode == TYPE_DMX_OUT) {


				artRDM.setPortType(portA[0], portA[1], DMX_OUT);

			// Start DMX output with RDM
			} else if (newMode == TYPE_RDM_OUT) {
				dmxA.rdmEnable(ESTA_MAN, ESTA_DEV);
				dmxA.rdmSetCallBack(rdmReceivedA);
				dmxA.todSetCallBack(sendTodA);
				artRDM.setPortType(portA[0], portA[1], RDM_OUT);

			// Start DMX input
			} else if (newMode == TYPE_DMX_IN) {
				dmxA.dmxIn(true);
				dmxA.setInputCallback(dmxIn);

				artRDM.setPortType(portA[0], portA[1], DMX_IN);

			// Start WS2812 output
			} else if (newMode == TYPE_WS2812) {
				doReboot = true;

				dmxA.end();

				artRDM.setPortType(portA[0], portA[1], TYPE_DMX_OUT);
				updatePorts = true;

				// Initialize the pixel strip
				pixDriver.setStrip(0, DMX_TX_A, deviceSettings.portAnumPix, deviceSettings.portApixConfig);

			}
			*/
		}

		// Update the Artnet class
		artRDM.setNet(portA[0], deviceSettings.portAnet);
		artRDM.setSubNet(portA[0], deviceSettings.portAsub);
		artRDM.setUni(portA[0], portA[1], deviceSettings.portAuni);
		artRDM.setMerge(portA[0], portA[1], deviceSettings.portAmerge);

		artRDM.artPollReply();

		eepromSave();
		return true;
	}
	break;

	case 5: // Port B
#ifndef ONE_PORT
	{
		deviceSettings.portBprot = (uint8_t)jsonRequest["portBprot"];
		bool e131 = (deviceSettings.portBprot == PROT_ARTNET_SACN) ? true : false;

		deviceSettings.portBmerge = (uint8_t)jsonRequest["portBmerge"];

		if ((uint8_t)jsonRequest["portBnet"] < 128)
			deviceSettings.portBnet = (uint8_t)jsonRequest["portBnet"];

		if ((uint8_t)jsonRequest["portBsub"] < 16)
			deviceSettings.portBsub = (uint8_t)jsonRequest["portBsub"];

		if ((uint8_t)jsonRequest["portBuni"] < 16)
			deviceSettings.portBuni = (uint8_t)jsonRequest["portBuni"];

		if ((uint16_t)jsonRequest["portBsACNuni"] > 0 && (uint16_t)jsonRequest["portBsACNuni"] < 64000)
			deviceSettings.portBsACNuni = (uint16_t)jsonRequest["portBsACNuni"];

		artRDM.setE131(portB[0], portB[1], e131);
		artRDM.setE131Uni(portB[0], portB[1], deviceSettings.portBsACNuni);

		uint8_t newMode = jsonRequest["portBmode"];
		uint8_t oldMode = deviceSettings.portBmode;

		// RDM and DMX input can't run together
		if (newMode == TYPE_RDM_OUT && deviceSettings.portAmode == TYPE_DMX_IN)
			newMode = TYPE_DMX_OUT;

		if (newMode != oldMode)
		{

			// Store the nem mode to settings
			deviceSettings.portBmode = newMode;

			doReboot = true;
		}

		// Update the Artnet class
		artRDM.setNet(portB[0], deviceSettings.portBnet);
		artRDM.setSubNet(portB[0], deviceSettings.portBsub);
		artRDM.setUni(portB[0], portB[1], deviceSettings.portBuni);
		artRDM.setMerge(portB[0], portB[1], deviceSettings.portBmerge);

		artRDM.artPollReply();

		eepromSave();
		return true;
	}
#endif
	break;

	case 6: // Firmware
	{
		if (jsonRequest.containsKey("checkUpdate") && jsonRequest["checkUpdate"])
		{
			checkForUpdate();
		};
		if (jsonRequest.containsKey("doUpdate") && jsonRequest["doUpdate"])
		{
			_doUpdate = 1;
		}
		deviceSettings.startupUpdates = jsonRequest["startupUpdates"];
		eepromSave();
		break;
	}

	default:
		// Catch errors
		return false;
	}
	return true;
}

void ajaxLoad(uint8_t page, JsonObject jsonReply, DynamicJsonDocument jsonReplyDoc, JsonObject jsonRequest, DynamicJsonDocument jsonRequestDoc)
{

	// Create the needed arrays here - doesn't work within the switch below
	JsonArray ipAddress = jsonReply.createNestedArray("ipAddress");
	JsonArray subAddress = jsonReply.createNestedArray("subAddress");
	JsonArray gwAddress = jsonReply.createNestedArray("gwAddress");
	JsonArray dmxInBroadcast = jsonReply.createNestedArray("dmxInBroadcast");

	// Get MAC Address
	char MAC_char[30] = "";
	sprintf(MAC_char, "%02X", MAC_array[0]);
	for (int i = 1; i < 6; ++i)
		sprintf(MAC_char, "%s:%02X", MAC_char, MAC_array[i]);

	jsonReply["macAddress"] = String(MAC_char);

	switch (page)
	{
	case 1: // Device Status
	{
		jsonReply.remove("ipAddress");
		jsonReply.remove("subAddress");
		jsonReply.remove("gwAddress");
		jsonReply.remove("dmxInBroadcast");

		jsonReply["wifiStatus"] = wifiStatus;

		if (isHotspot)
		{
			jsonReply["ipAddressT"] = IPToString(deviceSettings.hotspotIp);
			jsonReply["subAddressT"] = IPToString(deviceSettings.hotspotSubnet);
		}
		else
		{
			jsonReply["ipAddressT"] = IPToString(deviceSettings.ip);
			jsonReply["subAddressT"] = IPToString(deviceSettings.subnet);
		}

		if (isHotspot && !deviceSettings.standAloneEnable)
		{
			jsonReply["portAStatus"] = "Disabled in hotspot mode";
			jsonReply["portBStatus"] = "Disabled in hotspot mode";
		}
		else
		{
			switch (deviceSettings.portAmode)
			{
			case TYPE_DMX_OUT:
				jsonReply["portAStatus"] = "DMX output";
				break;

			case TYPE_RDM_OUT:
				jsonReply["portAStatus"] = "DMX/RDM output";
				break;

			case TYPE_DMX_IN:
				jsonReply["portAStatus"] = "DMX input";
				break;

			case TYPE_WS2812:
				jsonReply["portAStatus"] = "WS2812 mode";
				break;
			}
			switch (deviceSettings.portBmode)
			{
			case TYPE_DMX_OUT:
				jsonReply["portBStatus"] = "DMX output";
				break;

			case TYPE_RDM_OUT:
				jsonReply["portBStatus"] = "DMX/RDM output";
				break;

			case TYPE_DMX_IN:
				jsonReply["portBStatus"] = "DMX input";
				break;

			case TYPE_WS2812:
				jsonReply["portBStatus"] = "WS2812 mode";
				break;
			}
		}

		jsonReply["firmVer"] = FIRMWARE_VERSION;

		jsonReply["success"] = 1;
		break;
	}

	case 2: // Wifi
	{
		jsonReply.remove("ipAddress");
		jsonReply.remove("subAddress");
		jsonReply.remove("gwAddress");
		jsonReply.remove("dmxInBroadcast");
		jsonReply.remove("deviceSettings");

		jsonReply["wifiUsername"] = deviceSettings.wifiUsername;
		jsonReply["wifiSSID"] = deviceSettings.wifiSSID;
		jsonReply["wifiPass"] = deviceSettings.wifiPass;
		jsonReply["hotspotSSID"] = deviceSettings.hotspotSSID;
		jsonReply["hotspotPass"] = deviceSettings.hotspotPass;
		jsonReply["hotspotDelay"] = deviceSettings.hotspotDelay;
		jsonReply["standAloneEnable"] = deviceSettings.standAloneEnable;
		jsonReply["wpa2Enterprise"] = deviceSettings.wpa2Enterprise;

		jsonReply["success"] = 1;
		break;
	}

	case 3: // IP Address & Node Name
	{
		jsonReply.remove("dmxInBroadcast");
		jsonReply.remove("deviceSettings");

		jsonReply["dhcpEnable"] = deviceSettings.dhcpEnable;

		for (uint8_t x = 0; x < 4; x++)
		{
			ipAddress.add(deviceSettings.ip[x]);
			subAddress.add(deviceSettings.subnet[x]);
			gwAddress.add(deviceSettings.gateway[x]);
		};
		jsonReply["bcAddress"] = IPAddressToString(deviceSettings.broadcast);

		jsonReply["allowOTA"] = deviceSettings.allowOTA;
		jsonReply["autoRefresh"] = deviceSettings.autoRefresh;
		jsonReply["hostName"] = deviceSettings.hostName;

		jsonReply["success"] = 1;
		break;
	}

	case 4: // Port A
	{
		jsonReply.remove("ipAddress");
		jsonReply.remove("subAddress");
		jsonReply.remove("gwAddress");

		jsonReply["portAmode"] = deviceSettings.portAmode;

		// Only Artnet supported for receiving right now
		if (deviceSettings.portAmode == TYPE_DMX_IN)
			jsonReply["portAprot"] = PROT_ARTNET;
		else
			jsonReply["portAprot"] = deviceSettings.portAprot;

		jsonReply["portAmerge"] = deviceSettings.portAmerge;
		jsonReply["portAnet"] = deviceSettings.portAnet;
		jsonReply["portAsub"] = deviceSettings.portAsub;

		jsonReply["portAuni"] = deviceSettings.portAuni;
		jsonReply["portAsACNuni"] = deviceSettings.portAsACNuni;
		jsonReply["dmxInBroadcast"] = deviceSettings.dmxInBroadcast;

		jsonReply["success"] = 1;
		break;
	}

	case 5: // Port B
	{
		jsonReply.remove("ipAddress");
		jsonReply.remove("subAddress");
		jsonReply.remove("gwAddress");

		jsonReply["portBmode"] = deviceSettings.portBmode;
		jsonReply["portBprot"] = deviceSettings.portBprot;
		jsonReply["portBmerge"] = deviceSettings.portBmerge;
		jsonReply["portBnet"] = deviceSettings.portBnet;
		jsonReply["portBsub"] = deviceSettings.portBsub;
		jsonReply["portBuni"] = deviceSettings.portBuni;
		jsonReply["portBsACNuni"] = deviceSettings.portBsACNuni;

		for (uint8_t x = 0; x < 4; x++)
		{
			dmxInBroadcast.add(deviceSettings.dmxInBroadcast[x]);
		}

		jsonReply["success"] = 1;
		break;
	}

	case 6: // Firmware
	{

		/* if (jsonRequest.containsKey("export") && jsonRequest["export"])
		{
			for (uint16_t t = 0; t < sizeof(deviceSettings); t++)
				devSet.add(*((uint8_t *)&deviceSettings + t));

			jsonReply["wifiStatus"] = wifiStatus;
			JsonArray devSet_version = devSetO.createNestedArray("version");
			JsonArray devSet_ip = devSetO.createNestedArray("ip");
			JsonArray devSet_subnet = devSetO.createNestedArray("subnet");
			JsonArray devSet_gateway = devSetO.createNestedArray("gateway");
			JsonArray devSet_broadcast = devSetO.createNestedArray("broadcast");
			JsonArray devSet_hotspotIp = devSetO.createNestedArray("hotspotIp");
			JsonArray devSet_hotspotSubnet = devSetO.createNestedArray("hotspotSubnet");
			JsonArray devSet_hotspotBroadcast = devSetO.createNestedArray("hotspotBroadcast");
			JsonArray devSet_dmxInBroadcast = devSetO.createNestedArray("dmxInBroadcast");
			JsonArray devSet_nodeName = devSetO.createNestedArray("nodeName");
			JsonArray devSet_longName = devSetO.createNestedArray("longName");
			JsonArray devSet_wifiSSID = devSetO.createNestedArray("wifiSSID");
			JsonArray devSet_wifiPass = devSetO.createNestedArray("wifiPass");
			JsonArray devSet_wifiUsername = devSetO.createNestedArray("wifiUsername");
			JsonArray devSet_hotspotSSID = devSetO.createNestedArray("hotspotSSID");
			JsonArray devSet_hotspotPass = devSetO.createNestedArray("hotspotPass");
			JsonArray devSet_hostName = devSetO.createNestedArray("hostName");

			for (int i = 0; i < 4; i++)
			{
				devSet_version.add(deviceSettings.version[i]);
				devSet_ip.add(deviceSettings.ip[i]);
				devSet_subnet.add(deviceSettings.subnet[i]);
				devSet_gateway.add(deviceSettings.gateway[i]);
				devSet_broadcast.add(deviceSettings.broadcast[i]);
				devSet_hotspotIp.add(deviceSettings.hotspotIp[i]);
				devSet_hotspotSubnet.add(deviceSettings.hotspotSubnet[i]);
				devSet_hotspotBroadcast.add(deviceSettings.hotspotBroadcast[i]);
				devSet_dmxInBroadcast.add(deviceSettings.dmxInBroadcast[i]);
			};

			for (int i = 0; i < 64; i++)
			{
				devSet_longName.add(deviceSettings.longName[i]);
				devSet_wifiPass.add(deviceSettings.wifiPass[i]);
				devSet_hotspotPass.add(deviceSettings.hotspotPass[i]);
			};
			for (int i = 0; i < 32; i++)
			{
				devSet_wifiSSID.add(deviceSettings.wifiSSID[i]);
				devSet_wifiUsername.add(deviceSettings.wifiUsername[i]);
				devSet_hotspotSSID.add(deviceSettings.hotspotSSID[i]);
			}
			for (int i = 0; i < 18; i++)
			{
				devSet_nodeName.add(deviceSettings.nodeName[i]);
				devSet_hostName.add(deviceSettings.hostName[i]);
			}

			devSetO["dhcpEnable"] = deviceSettings.dhcpEnable;
			devSetO["standAloneEnable"] = deviceSettings.standAloneEnable;
			devSetO["hotspotDelay"] = deviceSettings.hotspotDelay;
			devSetO["portAmode"] = deviceSettings.portAmode;
			devSetO["portBmode"] = deviceSettings.portBmode;
			devSetO["portAprot"] = deviceSettings.portAprot;
			devSetO["portBprot"] = deviceSettings.portBprot;
			devSetO["portAmerge"] = deviceSettings.portAmerge;
			devSetO["portBmerge"] = deviceSettings.portBmerge;
			devSetO["portAnet"] = deviceSettings.portAnet;
			devSetO["portAsub"] = deviceSettings.portAsub;
			devSetO["portBnet"] = deviceSettings.portBnet;
			devSetO["portBsub"] = deviceSettings.portBsub;
			devSetO["resetCounter"] = deviceSettings.resetCounter;
			devSetO["wdtCounter"] = deviceSettings.wdtCounter;
			devSetO["allowOTA"] = deviceSettings.allowOTA;
			devSetO["autoRefresh"] = deviceSettings.autoRefresh;
			devSetO["wpa2Enterprise"] = deviceSettings.wpa2Enterprise;
			devSetO["startupUpdates"] = deviceSettings.startupUpdates;
			devSetO["portAuni"] = deviceSettings.portAuni;
			devSetO["portBuni"] = deviceSettings.portBuni;
			devSetO["portAsACNuni"] = deviceSettings.portAsACNuni;
			devSetO["portBsACNuni"] = deviceSettings.portBsACNuni;
		}
		else */
		{
			jsonReply.remove("ipAddress");
			jsonReply.remove("subAddress");
			jsonReply.remove("gwAddress");
			jsonReply.remove("dmxInBroadcast");
			jsonReply.remove("deviceSettings");
		}

		jsonReply["updateAvail"] = webUpdateAvail;
		jsonReply["firmVer"] = FIRMWARE_VERSION;
		jsonReply["latestVer"] = latestFirm;
		jsonReply["success"] = 1;
		jsonReply["startupUpdates"] = deviceSettings.startupUpdates;
		break;
	}

	default:
	{
		jsonReply.remove("ipAddress");
		jsonReply.remove("subAddress");
		jsonReply.remove("gwAddress");
		jsonReply.remove("portAuni");
		jsonReply.remove("portBuni");
		jsonReply.remove("portAsACNuni");
		jsonReply.remove("portBsACNuni");
		jsonReply.remove("dmxInBroadcast");
		jsonReply.remove("deviceSettings");

		jsonReply["success"] = 0;
		jsonReply["message"] = "Invalid or incomplete data received.";
	}
	}
}
