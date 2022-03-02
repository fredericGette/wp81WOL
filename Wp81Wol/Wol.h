#pragma once

#include <stdio.h>
#include <string>
#include <winsock2.h>

namespace Wol
{
	void send_wol(const std::string& hardware_addr, unsigned port, unsigned long bcast, const std::string& wifiIp);
}