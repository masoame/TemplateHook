#pragma once
#include"common.hpp"

namespace Pipe
{
	struct ctrlframe
	{
		size_t len;
		std::unique_ptr<char[]> buf;
		ctrlframe(size_t ilen, std::unique_ptr<char[]>&& ibuf)noexcept :len(ilen), buf(ibuf.release()) {};
		ctrlframe(ctrlframe&& cf)noexcept :len(cf.len), buf(cf.buf.release()) {};
		void operator=(ctrlframe&& cf)noexcept { len = cf.len; buf = std::move(cf.buf); };
	};
	struct PipeIO
	{
		static std::thread PipeInit;

		static std::queue<std::string> OutQueue;
		static std::queue<ctrlframe> InQueue;
		static std::mutex OutQueuemtx;
		static std::mutex InQueuemtx;

		static AutoHandle<> LogPipeH;
		static AutoHandle<> CtrlPipeH;

		inline const PipeIO& operator<<(auto&& str)const;

		inline const PipeIO& operator>>(ctrlframe& cf)const;
	private:
		static std::stringstream ss;
		static std::mutex ssmtx;
	};
	extern const PipeIO pout, pin, io;
	extern const std::string pendl;
}