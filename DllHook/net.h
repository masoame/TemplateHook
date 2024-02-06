#pragma once
#include"common.h"
namespace net
{
	extern bool isopen;
	extern SOCKET serverSock;
	extern sockaddr_in address;

	extern BOOL init(const char* ip, u_short port);
}
