#include "IPHelper.h"

String IPToString(IPAddress ip)
{
	String dot = ".";
	return ip[0] + dot + ip[1] + dot + ip[2] + dot + ip[3];
}

IPAddress getBroadcastIP(IPAddress subnet, IPAddress ip)
{
	return {
			static_cast<uint8_t>(~subnet[0] | (ip[0] & subnet[0])),
			static_cast<uint8_t>(~subnet[1] | (ip[1] & subnet[1])),
			static_cast<uint8_t>(~subnet[2] | (ip[2] & subnet[2])),
			static_cast<uint8_t>(~subnet[3] | (ip[3] & subnet[3]))};
};