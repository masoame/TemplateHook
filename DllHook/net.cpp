#include"net.h"
namespace net
{
	bool isopen = false;
	SOCKET serverSock = 0;
	sockaddr_in address{ 0 };

	BOOL init(const char* ip, u_short port)
	{
		serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (serverSock == INVALID_SOCKET) return FALSE;

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = inet_addr(ip);
		addr.sin_port = ntohs(port);
		if (connect(serverSock, (sockaddr*)&addr, sizeof(sockaddr)) == SOCKET_ERROR)return FALSE;

		return TRUE;
	}
}