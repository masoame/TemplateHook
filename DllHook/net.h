#pragma once
#include"common.h"
namespace net
{
	extern bool isopen;
	extern SOCKET serverSock;
	extern sockaddr_in address;

	extern std::thread StartClient;

	enum Request :byte
	{
		OSMessage
	};

	struct TCPFrame
	{
		const byte magic = 0x0721;
		Request type;
		int64_t FrameSize;
	};
}
