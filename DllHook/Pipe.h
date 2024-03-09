#pragma once
#include"common.h"


namespace Pipe
{
	struct PipeIO
	{
		static std::thread PipeInit;

		static std::queue<std::string> OutQueue;
		static std::mutex OutQueuemtx;

		static AutoHandle<> MsgPipeH;
		static AutoHandle<> CtrlPipeH;

		PipeIO& operator<<(const std::string&);
		PipeIO& operator<<(const auto&& str)
		{
			std::stringstream temp;
			temp << str;
			std::unique_lock lock(PipeIO::OutQueuemtx, std::try_to_lock);
			OutQueue.emplace(temp.str());
			return *this;
		}
		PipeIO& operator>>(char*);

	};
	extern PipeIO pout, pin, io;
	extern std::string pendl;

}