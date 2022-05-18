//Kazmos Server code
//Custom server by DarkX:  Licensed as Mirage Source for educational purposes
#include "modTypes.h"

int main()
{

	if (Network::Initialize())
	{
		std::cout << "Winsock api successfully initialized" << std::endl;
		Socket socket;
		if (socket.Create() == MResult::M_Success)
		{
			std::cout << "successfully created Socket." << std::endl;
			if (socket.Listen(IPEndpoint("::", 4000), 5) == MResult::M_Success)
			{
				std::cout << "Socket Successfully Listening on Port 4000" << std::endl;
				Socket newConnection;
				if (socket.Accept(newConnection) == MResult::M_Success)
				{
					std::cout << "New Connection accepted" << std::endl;

					Packet packet;
					std::string string1, string2;
					
					while (true)
					{
						MResult result = newConnection.Recv(packet);
						if (result != MResult::M_Success)
							break;

						try
						{
							packet >> string1 >> string2;
						}
						catch (PacketException & exception)
						{
							std::cout << exception.what() << std::endl;
							break;
						}
						std::cout << string1 << std::endl;
						std::cout << string2 << std::endl;
					}

					newConnection.Close();
				
				}
				else
				{
					std::cerr << "Failed to accept new connection" << std::endl;
				}
			}
			else
			{
				std::cerr << "Failed to Listen" << std::endl;
			}
			socket.Close();
		}
		else
		{
			std::cerr << "Failed to Create." << std::endl;
		}
	}
	Network::Shutdown();
	return 0;
}