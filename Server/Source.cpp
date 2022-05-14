//server code
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
			if (socket.Bind(IPEndpoint("0.0.0.0", 80)) == MResult::M_Success) 
			{
				std::cout << "Socket Successfully bound to ort 4790" << std::endl;

			}
			else
			{
				std::cerr << "Failed to bind socket to port 4790" << std::endl;
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