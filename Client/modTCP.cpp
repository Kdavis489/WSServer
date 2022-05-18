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
    assert(ipversion == IPVersion::IPv4 || ipversion == IPVersion::IPv6);
}

MResult Socket::Create()
{
    assert(ipversion == IPVersion::IPv4 || ipversion == IPVersion::IPv6);
    if (handle != INVALID_SOCKET)
    {
        return MResult::P_GenericError;
    }

    handle = socket((ipversion == IPVersion::IPv4) ? AF_INET : AF_INET6, SOCK_STREAM, IPPROTO_TCP);

    if (handle == INVALID_SOCKET)
    {
        int error = WSAGetLastError();
        return MResult::P_GenericError;
    }

    if (SetSocketOption(SocketOption::TCP_NoDelay, true) != MResult::M_Success)
    {
        return MResult::P_GenericError;
    }

    return MResult::M_Success;
}

MResult Socket::Close()
{
    if (handle == INVALID_SOCKET)
    {
        return MResult::P_GenericError;
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
    if (ipversion == IPVersion::IPv4)
    {
        sockaddr_in addr = endpoint.GetSockaddrIPv4();
        int result = bind(handle, (sockaddr*)(&addr), sizeof(sockaddr_in));

        if (result == INVALID_SOCKET)
        {
            int error = WSAGetLastError();
            return MResult::P_GenericError;
        }
    }
    else
    {
        sockaddr_in6 addr = endpoint.GetSockaddrIPv6();
        int result = bind(handle, (sockaddr*)(&addr), sizeof(sockaddr_in6));

        if (result == INVALID_SOCKET)
        {
            int error = WSAGetLastError();
            return MResult::P_GenericError;
        }
    }

    return MResult::M_Success;
}

MResult Socket::Listen(IPEndpoint endpoint, int backlog)
{
    if (ipversion == IPVersion::IPv6)
    {
        if (SetSocketOption(SocketOption::IPV6_ONLY, FALSE) != MResult::M_Success)
        {
            return MResult::P_GenericError;
        }
    }
    
    if (Bind(endpoint) != MResult::M_Success) {
        return MResult::P_GenericError;
    }

    if (listen(handle, backlog) != 0) {
        int error = WSAGetLastError();
        return MResult::M_Success;
    }

    return MResult();
}

MResult Socket::Accept(Socket& outSocket)
{
    assert(ipversion == IPVersion::IPv4 || ipversion == IPVersion::IPv6);
    if (ipversion == IPVersion::IPv4)
    {
        sockaddr_in addr = {};
        int len = sizeof(sockaddr_in);
        SocketHandle acceptedConnectionHandle = accept(handle, (sockaddr*)(&addr), &len);
        if (acceptedConnectionHandle == INVALID_SOCKET)
        {
            int error = WSAGetLastError();
            return MResult::P_GenericError;
        }
        IPEndpoint newConnectionEndpoint((sockaddr*)&addr);
        std::cout << "New connection accepted" << std::endl;
        outSocket = Socket(IPVersion::IPv4, acceptedConnectionHandle);

    }
    else //IPv6 begin
    {
        sockaddr_in6 addr = {};
        int len = sizeof(sockaddr_in6);
        SocketHandle acceptedConnectionHandle = accept(handle, (sockaddr*)(&addr), &len);
        if (acceptedConnectionHandle == INVALID_SOCKET)
        {
            int error = WSAGetLastError();
            return MResult::P_GenericError;
        }
        IPEndpoint newConnectionEndpoint((sockaddr*)&addr);
        std::cout << "New connection accepted" << std::endl;
        outSocket = Socket(IPVersion::IPv6, acceptedConnectionHandle);
    }

    return MResult::M_Success;
}

MResult Socket::Conenct(IPEndpoint endpoint)
{
    int result = 0;
    if (ipversion == IPVersion::IPv4)
    {
        sockaddr_in addr = endpoint.GetSockaddrIPv4();
        result = connect(handle, (sockaddr*)(&addr), sizeof(sockaddr_in));
    }
    else
    {
        sockaddr_in6 addr = endpoint.GetSockaddrIPv6();
        result = connect(handle, (sockaddr*)(&addr), sizeof(sockaddr_in6));
    }
    if (result != 0)
    {
        int error = WSAGetLastError();
        return MResult::P_GenericError;
    }

    return MResult::M_Success;
}

MResult Socket::Send(const void* data, int numberOfBytes, int& bytesSent)
{
    bytesSent = send(handle, (const char*)data, numberOfBytes, NULL);
    if (bytesSent == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        return MResult::P_GenericError;
    }
    return MResult::M_Success;
}

MResult Socket::Recv(void* destination, int numberOfBytes, int& bytesReceived)
{
    bytesReceived = recv(handle, (char*)destination, numberOfBytes, NULL);
    if (bytesReceived == 0) // if connection was gracefully closed
    {
        return MResult::P_GenericError;
    }
    if (bytesReceived == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        return MResult::P_GenericError;
    }
    return MResult::M_Success;
}

MResult Socket::SendAll(const void* data, int numberOfBytes)
{
    int totalBytesSent = 0;

    while (totalBytesSent < numberOfBytes)
    {
        int bytesRemaining = numberOfBytes - totalBytesSent;
        int bytesSent = 0;
        char* bufferOffset = (char*)data + totalBytesSent;
        MResult result = Send(bufferOffset, bytesRemaining, bytesSent);
        if (result != MResult::M_Success)
        {
            return MResult::P_GenericError;
        }
        totalBytesSent += bytesSent;
    }
    return MResult::M_Success;
}

MResult Socket::RecvAll(void* destination, int numberOfBytes)
{

    int totalBytesReceived = 0;

    while (totalBytesReceived < numberOfBytes)
    {
        int bytesRemaining = numberOfBytes - totalBytesReceived;
        int bytesReceived = 0;
        char* bufferOffset = (char*)destination + totalBytesReceived;
        MResult result = Recv(bufferOffset, bytesRemaining, bytesReceived);
        if (result != MResult::M_Success)
        {
            return MResult::P_GenericError;
        }
        totalBytesReceived += bytesReceived;
    }
    return MResult::M_Success;
}

MResult Socket::Send(Packet & packet)
{
    uint16_t encodedPacketSize = htons(packet.buffer.size());
    MResult result = SendAll(&encodedPacketSize, sizeof(uint16_t));
    if (result != MResult::M_Success)
    return MResult::P_GenericError;

    result = SendAll(packet.buffer.data(), packet.buffer.size());
    if (result != MResult::M_Success)
        return MResult::P_GenericError;

        return MResult::M_Success;
}

MResult Socket::Recv(Packet & packet)
{
    packet.Clear();
    uint16_t encodedSize = 0;
    MResult result = RecvAll(&encodedSize, sizeof(uint16_t));
        return MResult::P_GenericError;

    uint16_t bufferSize = ntohs(encodedSize);

    if (bufferSize > M_MaxPacketSize)
        return MResult::P_GenericError;

    packet.buffer.resize(bufferSize);
    result = RecvAll(&packet.buffer[0], bufferSize);
    if (result != MResult::M_Success)
        return MResult::P_GenericError;
 
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

//void Socket::Print(std::string hostname, std::string ip_string, std::vector<uint8_t> ip_bytes, unsigned short port)
//{
//   switch (ipversion)
//    {
//    case Socket::IPVersion::IPv4:
//        std::cout << " IP Version IPv4" << std::endl;
//        break;
//    case Socket::IPVersion::IPv6:
//        std::cout << " IP Version IPv6" << std::endl;
//        break;
//    default:
//        std::cout << "IP version Unknown" << std::endl;
//    }
//    std::cout << "Hostname: " << std::endl;
//    std::cout << "IP: " << ip_string << std::endl;
//    std::cout << "Port: " << port << std::endl;
//    std::cout << "IP bytes..." << std::endl;
//    for (auto& digit : ip_bytes)
//    {
//        std::cout << (int)digit << std::endl;
//    }
//}

MResult Socket::SetSocketOption(SocketOption option, BOOL value)
{
    int result = 0;
    switch (option)
    {
    case SocketOption::TCP_NoDelay:
        result = setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
        break;
    case SocketOption::IPV6_ONLY:
        result = setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*) & value, sizeof(value));
        break;
    default:
        return MResult::P_GenericError;
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

    //IPv4 begin
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

    //attempt to resolve host
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    addrinfo* hostinfo = nullptr;
    result = getaddrinfo(ip, NULL, &hints, &hostinfo);
    if (result == 0)
    {
        sockaddr_in* host_addr = reinterpret_cast<sockaddr_in*>(hostinfo->ai_addr);

        ip_string.resize(16);
        inet_ntop(AF_INET, &host_addr->sin_addr, &ip_string[0], 16);
        hostname = ip;
        ULONG ip_long = host_addr->sin_addr.S_un.S_addr;
        ip_bytes.resize(sizeof(ULONG));
        memcpy(&ip_bytes[0], &ip_long, sizeof(ULONG));

        ipversion = Socket::IPVersion::IPv4;

        freeaddrinfo(hostinfo);
        return;
    }//end IPv4

     //IPv6 begin
    in6_addr addr6;
    result = inet_pton(AF_INET6, ip, &addr6);
    if (result == 1)
    {
        ip_string = ip;
        hostname = ip;

        ip_bytes.resize(16);
        memcpy(&ip_bytes[0], &addr6.u, 16);

        ipversion = Socket::IPVersion::IPv6;
        return;
    }

    //attempt to resolve host
    addrinfo hintsv6 = {};
    hints.ai_family = AF_INET6;
    addrinfo* hostinfov6 = nullptr;
    result = getaddrinfo(ip, NULL, &hints, &hostinfov6);
    if (result == 0)
    {
        sockaddr_in6* host_addr = reinterpret_cast<sockaddr_in6*>(hostinfov6->ai_addr);

        ip_string.resize(46);
        inet_ntop(AF_INET6, &host_addr->sin6_addr, &ip_string[0], 46);

        hostname = ip;

        ip_bytes.resize(16);
        memcpy(&ip_bytes[0], &host_addr->sin6_addr, 16);

        ipversion = Socket::IPVersion::IPv6;

        freeaddrinfo(hostinfov6);
        return;
    }

}

IPEndpoint::IPEndpoint(sockaddr* addr)
{
    assert(addr->sa_family == AF_INET || addr->sa_family == AF_INET6);
    if (addr->sa_family == AF_INET)//IPv4 protocols
    {
        sockaddr_in* addrv4 = reinterpret_cast<sockaddr_in*>(addr);
        Socket::IPVersion ipversion = Socket::IPVersion::IPv4;
        port = ntohs(addrv4->sin_port);
        ip_bytes.resize(sizeof(ULONG));
        memcpy(&ip_bytes[0], &addrv4->sin_addr, sizeof(ULONG));
        ip_string.resize(16);
        inet_ntop(AF_INET, &addrv4->sin_addr, &ip_string[0], 16);
        hostname = ip_string;
    }
    else  //IPv6 protocols below
    {
        sockaddr_in6* addrv6 = reinterpret_cast<sockaddr_in6*>(addr);
        Socket::IPVersion ipversion = Socket::IPVersion::IPv6;
        port = ntohs(addrv6->sin6_port);
        ip_bytes.resize(16);
        memcpy(&ip_bytes[0], &addrv6->sin6_addr, 16);
        ip_string.resize(46);
        inet_ntop(AF_INET6, &addrv6->sin6_addr, &ip_string[0], 46);
        hostname = ip_string;
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
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr, &ip_bytes[0], sizeof(ULONG));
    addr.sin_port = htons(port);

    return addr;
}

sockaddr_in6 IPEndpoint::GetSockaddrIPv6()
{
    //    assert(ipversion == Socket::IPVersion::IPv6);
    sockaddr_in6 addr = {};
    addr.sin6_family = AF_INET6;
    memcpy(&addr.sin6_addr, &ip_bytes[0], 46);
    addr.sin6_port = htons(port);

    return addr;
}
