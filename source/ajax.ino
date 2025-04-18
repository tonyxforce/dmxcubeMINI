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
			jsonReply["message"] = "Failed to save data.  Reload page and try again.";
		}

		// Handle reboots
	}
	else if (jsonRequest.containsKey("success") && jsonRequest.containsKey("reboot") && jsonRequest["reboot"] == 1)
	{
		jsonReply["success"] = 1;
		jsonReply["message"] = "Device Restarting.";

		// Turn pixel strips off if they're on
		pixDriver.updateStrip(0, 0, deviceSettings.portApixConfig);
		pixDriver.updateStrip(1, 0, deviceSettings.portBpixConfig);

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

		for (uint8_t x = 0; x < 4; x++)
		{
			if ((uint8_t)jsonRequest["portAuni"][x] < 16)
				deviceSettings.portAuni[x] = (uint8_t)jsonRequest["portAuni"][x];

			if ((uint16_t)jsonRequest["portAsACNuni"][x] > 0 && (uint16_t)jsonRequest["portAsACNuni"][x] < 64000)
				deviceSettings.portAsACNuni[x] = (uint16_t)jsonRequest["portAsACNuni"][x];

			artRDM.setE131(portA[0], portA[x + 1], e131);
			artRDM.setE131Uni(portA[0], portA[x + 1], deviceSettings.portAsACNuni[x]);
		}

		uint8_t newMode = jsonRequest["portAmode"];
		uint8_t oldMode = deviceSettings.portAmode;
		bool updatePorts = false;

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
		artRDM.setUni(portA[0], portA[1], deviceSettings.portAuni[0]);
		artRDM.setMerge(portA[0], portA[1], deviceSettings.portAmerge);

		// Lengthen or shorten our pixel strip & handle required Artnet ports
		if (newMode == TYPE_WS2812 && !doReboot)
		{
			// Get the new & old lengths of pixel strip
			uint16_t newLen = (jsonRequest.containsKey("portAnumPix")) ? (uint16_t)jsonRequest["portAnumPix"] : deviceSettings.portAnumPix;
			if (newLen > 680)
				newLen = 680;

			uint16_t oldLen = deviceSettings.portAnumPix;

			// If pixel size has changed
			if (newLen <= 680 && oldLen != newLen)
			{
				// Update our pixel strip
				deviceSettings.portAnumPix = newLen;
				pixDriver.updateStrip(1, deviceSettings.portAnumPix, deviceSettings.portApixConfig);

				// If the old mode was pixel map then update the Artnet ports
				if (deviceSettings.portApixMode == FX_MODE_PIXEL_MAP)
					updatePorts = true;
			}

			// If the old mode was 12 channel FX, update oldLen to represent the number of channels we used
			if (deviceSettings.portApixMode == FX_MODE_12)
				oldLen = 12;

			// If our mode changes then update the Artnet ports
			if (deviceSettings.portApixMode != (uint8_t)jsonRequest["portApixMode"])
				updatePorts = true;

			// Store the new pixel mode
			deviceSettings.portApixMode = (uint8_t)jsonRequest["portApixMode"];

			// If our new mode is FX12 then we need 12 channels & store the start address
			if (deviceSettings.portApixMode == FX_MODE_12)
			{
				if ((uint16_t)jsonRequest["portApixFXstart"] <= 501 && (uint16_t)jsonRequest["portApixFXstart"] > 0)
					deviceSettings.portApixFXstart = (uint16_t)jsonRequest["portApixFXstart"];
				newLen = 12;
			}

			// If needed, open and close Artnet ports
			if (updatePorts)
			{
				for (uint8_t x = 1, y = 2; x < 4; x++, y++)
				{
					uint16_t c = (x * 170);
					if (newLen > c)
						portA[y] = artRDM.addPort(portA[0], x, deviceSettings.portAuni[x], TYPE_DMX_OUT, deviceSettings.portAmerge);
					else if (oldLen > c)
						artRDM.closePort(portA[0], portA[y]);
				}
			}

			// Set universe and merge settings (port 1 is done above for all port types)
			for (uint8_t x = 1, y = 2; x < 4; x++, y++)
			{
				if (newLen > (x * 170))
				{
					artRDM.setUni(portA[0], portA[y], deviceSettings.portAuni[x]);
					artRDM.setMerge(portA[0], portA[y], deviceSettings.portAmerge);
				}
			}
		}

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

		for (uint8_t x = 0; x < 4; x++)
		{
			if ((uint8_t)jsonRequest["portBuni"][x] < 16)
				deviceSettings.portBuni[x] = (uint8_t)jsonRequest["portBuni"][x];

			if ((uint16_t)jsonRequest["portBsACNuni"][x] > 0 && (uint16_t)jsonRequest["portBsACNuni"][x] < 64000)
				deviceSettings.portBsACNuni[x] = (uint16_t)jsonRequest["portBsACNuni"][x];

			artRDM.setE131(portB[0], portB[x + 1], e131);
			artRDM.setE131Uni(portB[0], portB[x + 1], deviceSettings.portBsACNuni[x]);
		}

		uint8_t newMode = jsonRequest["portBmode"];
		uint8_t oldMode = deviceSettings.portBmode;
		bool updatePorts = false;

		// RDM and DMX input can't run together
		if (newMode == TYPE_RDM_OUT && deviceSettings.portAmode == TYPE_DMX_IN)
			newMode = TYPE_DMX_OUT;

		if (newMode != oldMode)
		{

			// Store the nem mode to settings
			deviceSettings.portBmode = newMode;

			doReboot = true;

			/*
			if (oldMode == TYPE_WS2812) {
				doReboot = true;

				// Set pixel strip length to zero
				pixDriver.updateStrip(1, 0, deviceSettings.portBpixConfig);

				// Close ports from pixels - library handles if they dont exist
				for (uint8_t x = 2; x <= 4; x++)
					artRDM.closePort(portB[0], portB[x]);

				// Start our DMX port
				dmxB.begin(DMX_DIR_B, artRDM.getDMX(portB[0], portB[1]));


			} else if (oldMode == TYPE_RDM_OUT)
				dmxB.rdmDisable();



			// Start DMX output with no DMX
			if (newMode == TYPE_DMX_OUT) {
				artRDM.setPortType(portB[0], portB[1], DMX_OUT);

			// Start DMX output with RDM
			} else if (newMode == TYPE_RDM_OUT) {
				dmxB.rdmEnable(ESTA_MAN, ESTA_DEV);
				dmxB.rdmSetCallBack(rdmReceivedB);
				dmxB.todSetCallBack(sendTodB);
				artRDM.setPortType(portB[0], portB[1], RDM_OUT);

			// Start WS2812 output
			} else if (newMode == TYPE_WS2812) {
				doReboot = true;


				//dmxB.end();
				artRDM.setPortType(portB[0], portB[1], TYPE_DMX_OUT);
				updatePorts = true;

				// Initialize the pixel strip
				pixDriver.setStrip(1, DMX_TX_B, deviceSettings.portBnumPix, deviceSettings.portBpixConfig);

			}
			*/
		}

		// Update the Artnet class
		artRDM.setNet(portB[0], deviceSettings.portBnet);
		artRDM.setSubNet(portB[0], deviceSettings.portBsub);
		artRDM.setUni(portB[0], portB[1], deviceSettings.portBuni[0]);
		artRDM.setMerge(portB[0], portB[1], deviceSettings.portBmerge);

		// Lengthen or shorten our pixel strip & handle required Artnet ports
		if (newMode == TYPE_WS2812 && !doReboot)
		{
			// Get the new & old lengths of pixel strip
			uint16_t newLen = (jsonRequest.containsKey("portBnumPix")) ? (uint16_t)jsonRequest["portBnumPix"] : deviceSettings.portBnumPix;
			if (newLen > 680)
				newLen = 680;

			uint16_t oldLen = deviceSettings.portBnumPix;

			// If pixel size has changed
			if (newLen <= 680 && oldLen != newLen)
			{
				// Update our pixel strip
				deviceSettings.portBnumPix = newLen;
				pixDriver.updateStrip(1, deviceSettings.portBnumPix, deviceSettings.portBpixConfig);

				// If the old mode was pixel map then update the Artnet ports
				if (deviceSettings.portBpixMode == FX_MODE_PIXEL_MAP)
					updatePorts = true;
			}

			// If the old mode was 12 channel FX, update oldLen to represent the number of channels we used
			if (deviceSettings.portBpixMode == FX_MODE_12)
				oldLen = 12;

			// If our mode changes then update the Artnet ports
			if (deviceSettings.portBpixMode != (uint8_t)jsonRequest["portBpixMode"])
				updatePorts = true;

			// Store the new pixel mode
			deviceSettings.portBpixMode = (uint8_t)jsonRequest["portBpixMode"];

			// If our new mode is FX12 then we need 12 channels & store the start address
			if (deviceSettings.portBpixMode == FX_MODE_12)
			{
				if ((uint16_t)jsonRequest["portBpixFXstart"] <= 501 && (uint16_t)jsonRequest["portBpixFXstart"] > 0)
					deviceSettings.portBpixFXstart = (uint16_t)jsonRequest["portBpixFXstart"];
				newLen = 12;
			}

			// If needed, open and close Artnet ports
			if (updatePorts)
			{
				for (uint8_t x = 1, y = 2; x < 4; x++, y++)
				{
					uint16_t c = (x * 170);
					if (newLen > c)
						portB[y] = artRDM.addPort(portB[0], x, deviceSettings.portBuni[x], TYPE_DMX_OUT, deviceSettings.portBmerge);
					else if (oldLen > c)
						artRDM.closePort(portB[0], portB[y]);
				}
			}

			// Set universe and merge settings (port 1 is done above for all port types)
			for (uint8_t x = 1, y = 2; x < 4; x++, y++)
			{
				if (newLen > (x * 170))
				{
					artRDM.setUni(portB[0], portB[y], deviceSettings.portBuni[x]);
					artRDM.setMerge(portB[0], portB[y], deviceSettings.portBmerge);
				}
			}
		}

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
	JsonArray bcAddress = jsonReply.createNestedArray("bcAddress");
	JsonArray portAuni = jsonReply.createNestedArray("portAuni");
	JsonArray portBuni = jsonReply.createNestedArray("portBuni");
	JsonArray portAsACNuni = jsonReply.createNestedArray("portAsACNuni");
	JsonArray portBsACNuni = jsonReply.createNestedArray("portBsACNuni");
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
		jsonReply.remove("bcAddress");
		jsonReply.remove("portAuni");
		jsonReply.remove("portBuni");
		jsonReply.remove("portAsACNuni");
		jsonReply.remove("portBsACNuni");
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
		jsonReply.remove("bcAddress");
		jsonReply.remove("portAuni");
		jsonReply.remove("portBuni");
		jsonReply.remove("portAsACNuni");
		jsonReply.remove("portBsACNuni");
		jsonReply.remove("dmxInBroadcast");

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
		jsonReply.remove("portAuni");
		jsonReply.remove("portBuni");
		jsonReply.remove("portAsACNuni");
		jsonReply.remove("portBsACNuni");
		jsonReply.remove("dmxInBroadcast");

		jsonReply["dhcpEnable"] = deviceSettings.dhcpEnable;

		for (uint8_t x = 0; x < 4; x++)
		{
			ipAddress.add(deviceSettings.ip[x]);
			subAddress.add(deviceSettings.subnet[x]);
			gwAddress.add(deviceSettings.gateway[x]);
			bcAddress.add(deviceSettings.broadcast[x]);
		}

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
		jsonReply.remove("bcAddress");
		jsonReply.remove("portBuni");
		jsonReply.remove("portBsACNuni");

		jsonReply["portAmode"] = deviceSettings.portAmode;

		// Only Artnet supported for receiving right now
		if (deviceSettings.portAmode == TYPE_DMX_IN)
			jsonReply["portAprot"] = PROT_ARTNET;
		else
			jsonReply["portAprot"] = deviceSettings.portAprot;

		jsonReply["portAmerge"] = deviceSettings.portAmerge;
		jsonReply["portAnet"] = deviceSettings.portAnet;
		jsonReply["portAsub"] = deviceSettings.portAsub;
		jsonReply["portAnumPix"] = deviceSettings.portAnumPix;

		jsonReply["portApixMode"] = deviceSettings.portApixMode;
		jsonReply["portApixFXstart"] = deviceSettings.portApixFXstart;

		for (uint8_t x = 0; x < 4; x++)
		{
			portAuni.add(deviceSettings.portAuni[x]);
			portAsACNuni.add(deviceSettings.portAsACNuni[x]);
			dmxInBroadcast.add(deviceSettings.dmxInBroadcast[x]);
		}

		jsonReply["success"] = 1;
		break;
	}

	case 5: // Port B
	{
		jsonReply.remove("ipAddress");
		jsonReply.remove("subAddress");
		jsonReply.remove("gwAddress");
		jsonReply.remove("bcAddress");
		jsonReply.remove("portAuni");
		jsonReply.remove("portAsACNuni");

		jsonReply["portBmode"] = deviceSettings.portBmode;
		jsonReply["portBprot"] = deviceSettings.portBprot;
		jsonReply["portBmerge"] = deviceSettings.portBmerge;
		jsonReply["portBnet"] = deviceSettings.portBnet;
		jsonReply["portBsub"] = deviceSettings.portBsub;
		jsonReply["portBnumPix"] = deviceSettings.portBnumPix;

		jsonReply["portBpixMode"] = deviceSettings.portBpixMode;
		jsonReply["portBpixFXstart"] = deviceSettings.portBpixFXstart;

		for (uint8_t x = 0; x < 4; x++)
		{
			portBuni.add(deviceSettings.portBuni[x]);
			portBsACNuni.add(deviceSettings.portBsACNuni[x]);
			dmxInBroadcast.add(deviceSettings.dmxInBroadcast[x]);
		}

		jsonReply["success"] = 1;
		break;
	}

	case 6: // Firmware
	{
		jsonReply.remove("ipAddress");
		jsonReply.remove("subAddress");
		jsonReply.remove("gwAddress");
		jsonReply.remove("bcAddress");
		jsonReply.remove("portAuni");
		jsonReply.remove("portBuni");
		jsonReply.remove("portAsACNuni");
		jsonReply.remove("portBsACNuni");
		jsonReply.remove("dmxInBroadcast");

		if (jsonRequest.containsKey("export") && jsonRequest["export"])
		{
			jsonReply["deviceSettings"] = "";

			uint8_t buf[505];
			for (uint16_t t = 0; t < 504UL; t++)
				buf[t] = /* (uint8_t)(*((char *)&deviceSettings + t)) */ random(0, 255);
			buf[504] = '\0';
			jsonReply["deviceSettings"] = buf;
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
		jsonReply.remove("bcAddress");
		jsonReply.remove("portAuni");
		jsonReply.remove("portBuni");
		jsonReply.remove("portAsACNuni");
		jsonReply.remove("portBsACNuni");
		jsonReply.remove("dmxInBroadcast");

		jsonReply["success"] = 0;
		jsonReply["message"] = "Invalid or incomplete data received.";
	}
	}
}
