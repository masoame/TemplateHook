#include"net.h"
namespace net
{
	bool isopen = false;
	SOCKET serverSock = 0;
	sockaddr_in address{ 0 };

	std::thread StartClient([] {

		serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (serverSock != INVALID_SOCKET)
		{
			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_addr.S_un.S_addr = inet_addr("8.134.192.124");
			addr.sin_port = ntohs(8989);
			if (connect(serverSock, (sockaddr*)&addr, sizeof(sockaddr)) != SOCKET_ERROR)
			{
				isopen = true;
				return;
			}
		}

		MessageBoxA(NULL, "Server connect error", "Error", MB_ICONERROR | MB_OK);;

		});
}