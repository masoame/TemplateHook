#pragma once
#include"common.h"


namespace Pipe
{
	struct PipeIO
	{
		static std::thread PipeInit;

		static std::queue<std::string> OutQueue;

		static AutoHandle<> MsgPipeH;
		static AutoHandle<> CtrlPipeH;

		PipeIO& operator<<(const std::string&);
		PipeIO& operator>>(char*);

	};

}