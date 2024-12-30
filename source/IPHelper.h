#ifndef IPHELPER_H
#define IPHELPER_H

#include <Arduino.h>
#include <IPAddress.h>

String IPAddressToString(IPAddress ip);
IPAddress getBroadcastIP(IPAddress subnet, IPAddress ip);


#endif