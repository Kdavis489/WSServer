//Kazmos Server code
//Custom server by DarkX:  Licensed as Mirage Source for educational purposes
//thanks to learning tutorials from: 
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
			if (socket.Conenct(IPEndpoint("127.0.0.1", 4000)) == MResult::M_Success)
			{
				std::cout << "Successfully connected to server!" << std::endl;

				Packet packet;
				packet << std::string("1st string");
				packet << std::string("2nd string");
				while (true)
				{
					MResult result = socket.Send(packet);
					if (result != MResult::M_Success)
						break;

					std::cout << "Attempting to send chunk of data..." << std::endl;
					Sleep(500);
				}

			}
			else
			{
				std::cout << "Failed to connect to server" << std::endl;
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