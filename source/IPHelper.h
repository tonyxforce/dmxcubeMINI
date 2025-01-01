#ifndef IPHELPER_H
#define IPHELPER_H

#include <Arduino.h>
#include <IPAddress.h>

#define IPToString(a) IPAddressToString(a)

String IPAddressToString(IPAddress ip);
IPAddress getBroadcastIP(IPAddress subnet, IPAddress ip);


#endif