#include "store.h"
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

StoreStruct deviceSettings = {
	CONFIG_VERSION,
  
  // The default values
  IPAddress(2,0,0,1), //ip
	IPAddress(255,0,0,0), //subnet
	IPAddress(2,0,0,1), //gatewy
	IPAddress(2,255,255,255), //broadcast
	
	IPAddress(2,0,0,1), //hotspotIP
	IPAddress(255,0,0,0), //hotspotSubnet
	IPAddress(2,255,255,255), //hotspotBroadcast
	
	IPAddress(2,255,255,255), //dmxInBroadcast


  true, false, //dhcpEnable, standAloneEnable

  "DMXCube mini W", // nodeName
	"DMXCube mini Wireless", // longName
	"", "", "", // WiFi SSID, WiFi Password, WiFi Username
	"DMXCube mini W", "DMXCube2024", // hotspotSSID, hotspotPass
	
	"cubeminiw", //hostname
  
	15, //hotspotdelay

  TYPE_DMX_OUT, TYPE_DMX_OUT, // port A/B mode
	PROT_ARTNET, PROT_ARTNET, // port A/B protocol
	MERGE_HTP, MERGE_HTP, // port A/B merge mode
  0, 0, 0, // port A net, sub, uni
	0, 0, 1, // port B net, sub, uni
	0, 1, // port A/B sACN uni
  
	0, 0, // resetCounter, wdtCounter
	
	true, //allowOTA
	true, //autoRefresh
	false,//wpa2Enterprise
	false//startupUpdates
};


void eepromSave() {
  for (uint16_t t = 0; t < STORESIZE; t++)
    EEPROM.write(CONFIG_START + t, *((char*)&deviceSettings + t));
  
  EEPROM.commit();
}

void eepromLoad() {
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2]) {

    // Store defaults for if we need them
    StoreStruct tmpStore;
    tmpStore = deviceSettings;
    
    // Copy data to deviceSettings structure
    for (uint16_t t = 0; t < sizeof(deviceSettings); t++)
      *((char*)&deviceSettings + t) = EEPROM.read(CONFIG_START + t);
    
    // If we want to restore all our settings
    if (deviceSettings.resetCounter >= 5 || deviceSettings.wdtCounter >= 10) {
      deviceSettings.wdtCounter = 0;
      deviceSettings.resetCounter = 0;

      // Store defaults back into main settings
      deviceSettings = tmpStore;
    }


  // If config files dont match, save defaults then erase the ESP config to clear away any residue
  } else {
    eepromSave();
    delay(500);
    
    ESP.eraseConfig();
		while(1);
  }
}