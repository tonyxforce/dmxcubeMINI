#ifndef STORE_H
#define STORE_H

#include <EEPROM.h>
#include <ESP8266WiFi.h>

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


// Change this if the settings structure changes
#define CONFIG_VERSION "007"

// Dont change this
#define CONFIG_START 0

enum fx_mode {
  FX_MODE_PIXEL_MAP = 0,
  FX_MODE_12 = 1
};

enum p_type {
  TYPE_DMX_OUT = 0,
  TYPE_RDM_OUT = 1,
  TYPE_DMX_IN = 2,
  TYPE_WS2812 = 3
};

enum p_protocol {
  PROT_ARTNET = 0,
  PROT_ARTNET_SACN = 1
};

enum p_merge {
  MERGE_LTP = 0,
  MERGE_HTP = 1
};

struct StoreStruct {
  // StoreStruct version
  char version[4];

  // Device settings:
  IPAddress ip,
	subnet,
	gateway,
	broadcast,
	
	hotspotIp,
	hotspotSubnet,
	hotspotBroadcast,
	
	dmxInBroadcast;


  bool dhcpEnable, standAloneEnable;

  char nodeName[18],
	longName[64],
	
	wifiSSID[32], wifiPass[64], wifiUsername[32],
	hotspotSSID[32], hotspotPass[64];
	
	char hostName[18];
  
	uint16_t hotspotDelay;

  uint8_t portAmode, portBmode,
	portAprot, portBprot,
	portAmerge, portBmerge;
	
  uint8_t portAnet, portAsub, portAuni, portBnet, portBsub, portBuni, portAsACNuni, portBsACNuni;
  uint8_t resetCounter, wdtCounter;
	bool allowOTA;
	bool autoRefresh;
	bool wpa2Enterprise;
  bool startupUpdates;
};
extern StoreStruct deviceSettings;
#define STORESIZE sizeof(StoreStruct)


void eepromSave();
void eepromLoad();



#endif