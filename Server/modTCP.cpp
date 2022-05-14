#include "modTypes.h"

bool Network::Initialize()
{
	WSADATA wsadata;
	int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (result != 0)
	{
		std::cerr << "failed to initialize winsock api" << std::endl;
		return false;
	}
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
	{
		std::cerr << "Could not find a usable version of the winsock api dll" << std::endl;
		return false;
	}
	return true;
}

void Network::Shutdown()
{
	WSACleanup();
}

Socket::Socket(IPVersion ipversion, SocketHandle handle)
    :ipversion(ipversion), handle(handle)
{
    assert(ipversion == IPVersion::IPv4);
}

MResult Socket::Create()
{
    assert(ipversion == IPVersion::IPv4);
    if (handle != INVALID_SOCKET)
    {
        return MResult::M_NYI;
    }

    handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (handle == INVALID_SOCKET)
    {
        int error = WSAGetLastError();
        return MResult::M_NYI;
    }

    if (SetSocketOption(SocketOption::TCP_NoDelay, true) != MResult::M_Success)
    {
        return MResult::M_NYI;
    }

    return MResult::M_Success;
}

MResult Socket::Close()
{
    if (handle == INVALID_SOCKET)
    {
        return MResult::M_NYI;
    }

    int result = closesocket(handle);
    if (result != 0)
    {
        int error = WSAGetLastError();
    }
    handle = INVALID_SOCKET;
    return MResult::M_Success;
}

MResult Socket::Bind(IPEndpoint endpoint)
{
    sockaddr_in addr = endpoint.GetSockaddrIPv4();
    int result = bind(handle, (sockaddr*)(&addr), sizeof(sockaddr_in));
    if (result != 0)
    {
        int error = WSAGetLastError();
    }
    else
    {
        int error = WSAGetLastError();
        return MResult::M_NYI;
    }
    return MResult::M_Success;
}

SocketHandle Socket::GetHandle()
{

    return handle;
}

Socket::IPVersion Socket::GetIPVersion()
{
    return ipversion;
}

MResult Socket::SetSocketOption(SocketOption option, BOOL value)
{
    int result = 0;
    switch (option)
    {
    case SocketOption::TCP_NoDelay:
        result = setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
        break;
    default:
        return MResult::M_NYI;
    }
    if (result != 0)
    {
        int error = WSAGetLastError();
    }
    return MResult::M_Success;
}

IPEndpoint::IPEndpoint(const char* ip, unsigned short port)
{
    this->port = port;
    Socket::IPVersion ipversion;
    in_addr addr;
    int result = inet_pton(AF_INET, ip, &addr);
    if (result == 1)
    {
        if (addr.S_un.S_addr != INADDR_NONE)
        {
            ip_string = ip;
            hostname = ip;

            ip_bytes.resize(sizeof(ULONG));
            memcpy(&ip_bytes[0], &addr.S_un.S_addr, sizeof(ULONG));

            ipversion = Socket::IPVersion::IPv4;
            return;
        }
    }

    addrinfo hints = {};
    hints.ai_family = AF_INET;
    addrinfo* hostinfo = nullptr;
    result = getaddrinfo(ip, NULL, &hints, &hostinfo);
    if (result == 0)
    {
        sockaddr_in* host_addr = reinterpret_cast<sockaddr_in*>(hostinfo->ai_addr);

       // host_addr->sin_addr.S_un.S_addr
          ip_string.resize(16);
          inet_ntop(AF_INET, &host_addr->sin_addr, &ip_string[0], 16);
          hostname = ip;
          ULONG ip_long = host_addr->sin_addr.S_un.S_addr;
          ip_bytes.resize(sizeof(ULONG));
          memcpy(&ip_bytes[0], &ip_long, sizeof(ULONG));

          ipversion = Socket::IPVersion::IPv4;

        freeaddrinfo(hostinfo);
        return;
    }
}

std::vector<uint8_t> IPEndpoint::GetIPBytes()
{
    return ip_bytes;
}

std::string IPEndpoint::GetHostname()
{
    return hostname;
}

std::string IPEndpoint::GetIPString()
{
    return ip_string;
}

unsigned short IPEndpoint::GetPort()
{
    return port;
}

sockaddr_in IPEndpoint::GetSockaddrIPv4()
{
    //Socket::IPVersion ipversion;
    //assert(ipversion == Socket::IPVersion::IPv4);
        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        memcpy(&addr.sin_addr, &ip_bytes[0], sizeof(ULONG));
        addr.sin_port = htons(port);

        return addr;
}
