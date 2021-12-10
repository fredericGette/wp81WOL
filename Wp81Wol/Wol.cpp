// https://github.com/gumb0/wol.cpp/blob/master/wol.cpp

#include "pch.h"
#include "Wol.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library


void Debug(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[100];
	vsnprintf_s(buffer, sizeof(buffer), format, args);

	OutputDebugStringA(buffer);

	va_end(args);
}

// Attempts to extract hexadecimal from ASCII string.
unsigned get_hex_from_string(const std::string& s)
{
	unsigned hex{ 0 };

	for (size_t i = 0; i < s.length(); ++i) {
		hex <<= 4;
		if (isdigit(s[i])) {
			hex |= s[i] - '0';
		}
		else if (s[i] >= 'a' && s[i] <= 'f') {
			hex |= s[i] - 'a' + 10;
		}
		else if (s[i] >= 'A' && s[i] <= 'F') {
			hex |= s[i] - 'A' + 10;
		}
		else {
			throw std::runtime_error("Failed to parse hexadecimal " + s);
		}
	}
	return hex;
}

// Extract inet address from hardware address.
std::string get_ether(const std::string& hardware_addr)
{
	std::string ether_addr;

	for (size_t i = 0; i < hardware_addr.length();) {
		// Parse two characters at a time.
		unsigned hex = get_hex_from_string(hardware_addr.substr(i, 2));
		i += 2;

		ether_addr += static_cast<char>(hex & 0xFF);

		// We might get a colon here, but it is not required.
		if (hardware_addr[i] == ':')
			++i;
	}

	if (ether_addr.length() != 6) {
		throw std::runtime_error(hardware_addr + " not a valid address");
	}


	return ether_addr;
}


// Sends Wake-On-LAN packet to given address with the given options, where the
// address is in the form "XX:XX:XX:XX:XX:XX" (the colons are optional).
void Wol::send_wol(const std::string& hardware_addr, unsigned port, unsigned long bcast)
{
	// Fetch the hardware address. 
	const std::string ether_addr{ get_ether(hardware_addr) };

	int iResult;
	WSADATA wsaData;

	SOCKET SendSocket = INVALID_SOCKET;

	//----------------------
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		throw std::runtime_error("WSAStartup failed with error : "+ std::to_string(iResult));
	}

	//---------------------------------------------
	// Create a socket for sending data
	SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (SendSocket == INVALID_SOCKET) {
		int error = WSAGetLastError();
		WSACleanup();
		throw std::runtime_error("Socket failed with error: "+std::to_string(error));
	}

	// Build the message to send.
	//   (6 * 0XFF followed by 16 * destination address.) 
	std::string message(6, 0xFF);
	for (size_t i = 0; i < 16; ++i) {
		message += ether_addr;
	}

	// Set socket options. 
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	if (setsockopt(SendSocket, SOL_SOCKET, SO_BROADCAST, (char *)&bOptVal, bOptLen) < 0) {
		int error = WSAGetLastError();
		closesocket(SendSocket);
		WSACleanup();
		throw std::runtime_error("Failed to set socket options: " + std::to_string(error));
	}


	// Set up address 
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = bcast;
	addr.sin_port = htons(port);


	//---------------------------------------------
	// Send a datagram to the receiver
	Debug("Sending a datagram to the receiver...\n");
	iResult = sendto(SendSocket,
		message.c_str(), message.length(), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	if (iResult == SOCKET_ERROR) {
		int error = WSAGetLastError();
		closesocket(SendSocket);
		WSACleanup();
		throw std::runtime_error("sendto failed with error: " + std::to_string(error));
	}

	//---------------------------------------------
	// When the application is finished sending, close the socket.
	Debug("Finished sending. Closing socket.\n");
	iResult = closesocket(SendSocket);
	if (iResult == SOCKET_ERROR) {
		int error = WSAGetLastError();
		WSACleanup();
		throw std::runtime_error("closesocket failed with error: " + std::to_string(error));
	}

	//---------------------------------------------
	// Clean up and quit.
	Debug("Exiting.\n");
	WSACleanup();
}

