#ifndef MODTYPES_H
#define MODTYPES_H
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdint.h>
#include <WinSock2.h>
#include <iostream>
#include <assert.h>
#include <string>
#include <vector>
#include <WS2tcpip.h>
//type defines
typedef SOCKET SocketHandle;

//constants
const int M_MaxPacketSize = 8192;

const int M_MaxAccounts = 200;
const int M_MaxPlayers = 100;
const int M_MaxInv = 20;
const int M_MaxSpells = 10;
const int M_MaxMaps = 1000;
const int M_MaxShops = 250;
const int M_MaxItems = 250;
const int M_MaxQuests = 250;
const int M_MaxMapX = 30;
const int M_MaxMapY = 30;


enum class MResult
{
	M_Success,
	P_GenericError,
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
	IPEndpoint(sockaddr* addr);
	std::vector<uint8_t> GetIPBytes();
	std::string GetHostname();
	std::string GetIPString();
	unsigned short GetPort();
	sockaddr_in GetSockaddrIPv4();
	sockaddr_in6 GetSockaddrIPv6();
private:
	std::string hostname;
	std::string ip_string;
	std::vector<uint8_t> ip_bytes;
	unsigned short port = 0;
};

enum class PacketType : uint16_t
{
	PT_Invalid,
	PT_ChatMessage,
	PT_IntegerArray,
};

struct Packet
{
public:
	Packet(PacketType packetType = PacketType::PT_Invalid);
	PacketType GetPacketType();
	void AssignPacketType(PacketType packetType);

	void Clear();
	void Append(const void* data, uint32_t size);

	//insert operation
	Packet & operator << (uint32_t data);
	//extract operation
	Packet & operator >> (uint32_t & data);
	//insert operation
	Packet & operator << (const std::string & data);
	//extract operation
	Packet & operator >> (std::string & data);

	uint32_t extractionOffset = 0;
	std::vector<char> buffer;
};

//struct GamePackets
//{
	//account packets
//	Packet Login;
//	Packet deleteAcc;
//	Packet createAcc;

	//game state packet
//	Packet inGame;
//	Packet inEditor;

	//player packet
//	Packet Character;
//	Packet Inventory;
//	Packet 
//};

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
		TCP_NoDelay,
		IPV6_ONLY, //TRUE = only IPv6con connect. FALSE = IPv4 and IPv6 can both connect
	};

	Socket(IPVersion ipversion = IPVersion::IPv4, SocketHandle handle = INVALID_SOCKET);
	MResult Create();
	MResult Close();
	MResult Bind(IPEndpoint endpoint);
	MResult Listen(IPEndpoint endpoint, int backlog = 5);
	MResult Accept(Socket & outSocket);
	MResult Conenct(IPEndpoint endpoint);
	MResult Send(const void* data, int numberOfBytes, int& bytesSent);
	MResult Recv(void* destination, int numberOfBytes, int& bytesReceived);
	MResult SendAll(const void* data, int numberOfBytes);
	MResult RecvAll(void* destination, int numberOfBytes);
	MResult Send(Packet & packet);
	MResult Recv(Packet & packet);
	SocketHandle GetHandle();
	IPVersion GetIPVersion();
//	void Print(std::string hostname, std::string ip_string, std::vector<uint8_t> ip_bytes, unsigned short port);
private:
	MResult SetSocketOption(SocketOption option, BOOL value);
	IPVersion ipversion = IPVersion::IPv4;
	SocketHandle handle = INVALID_SOCKET;
};