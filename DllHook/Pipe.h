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

		template<typename T>
		const PipeIO& operator<<(T&& str) const
		{
			ss << std::forward<T>(str);
			std::unique_lock lockqueue(OutQueuemtx, std::try_to_lock);
			std::unique_lock lockss(ssmtx, std::try_to_lock);
			OutQueue.emplace(ss.str());
			ss.str("");
			ss.clear();
			return *this;
		}
		PipeIO& operator>>(char*);
	private:
		static std::stringstream ss;
		static std::mutex ssmtx;
	};
	extern const PipeIO pout, pin, io;
	extern const std::string pendl;

}