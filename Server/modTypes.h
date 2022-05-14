#ifndef MODTYPES_H
#define MODTYPES_H
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>
#include <iostream>
#include <assert.h>
#include <string>
#include <vector>
#include <WS2tcpip.h>

typedef SOCKET SocketHandle;

enum class MResult
{
	M_Success,
	M_NYI,
};

	struct Network
	{
	public:
		static bool Initialize();
		static void Shutdown();
	};

	struct IPEndpoint
	{
	public:
		IPEndpoint(const char* ip, unsigned short port);
//		Socket::IPVersion GetIPVersion();
		std::vector<uint8_t> GetIPBytes();
		std::string GetHostname();
		std::string GetIPString();
		unsigned short GetPort();
		sockaddr_in GetSockaddrIPv4();
	private:
//		Socket::IPVersion ipversion = Socket::IPVersion::Unknown;
		std::string hostname;
		std::string ip_string;
		std::vector<uint8_t> ip_bytes;
		unsigned short port = 0;
	};

	struct Socket
	{
	public:
		enum class IPVersion
		{
			Unknown,
			IPv4,
			IPv6
		};

		enum class SocketOption
		{
			TCP_NoDelay
		};

		Socket(IPVersion ipversion = IPVersion::IPv4, SocketHandle handle = INVALID_SOCKET);
		MResult Create();
		MResult Close();
		MResult Bind(IPEndpoint endpoint);
		SocketHandle GetHandle();
		IPVersion GetIPVersion();
	private:
		MResult SetSocketOption(SocketOption option, BOOL value);
		IPVersion ipversion = IPVersion::IPv4;
		SocketHandle handle = INVALID_SOCKET;
	};