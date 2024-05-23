#include"Pipe.h"

namespace Pipe
{
	constexpr const auto& LogPipeName = L"\\\\.\\pipe\\LogPipe";
	constexpr const auto& CtrlPipeName = L"\\\\.\\pipe\\CtrlPipe";

	std::queue<std::string> PipeIO::OutQueue;
	std::queue<ctrlframe> PipeIO::InQueue;
	std::mutex PipeIO::OutQueuemtx;
	std::mutex PipeIO::InQueuemtx;

	AutoHandle<> PipeIO::LogPipeH{};
	AutoHandle<> PipeIO::CtrlPipeH{};
	std::stringstream PipeIO::ss;
	std::mutex PipeIO::ssmtx;

	const PipeIO pout, pin, io;
	const std::string pendl("\n");
	std::thread PipeIO::PipeInit([]
		{
			LogPipeH = CreateNamedPipeW(LogPipeName, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE, 1, 0, 0, 0, nullptr);
			CtrlPipeH = CreateNamedPipeW(CtrlPipeName, PIPE_ACCESS_INBOUND, PIPE_READMODE_BYTE, 1, 0, 0, 0, nullptr);

			if (LogPipeH)
				std::thread([]
					{
						if (LogPipeH)std::cout << "LogPipe open success" << std::endl;
						std::unique_lock lock(PipeIO::OutQueuemtx, std::defer_lock);
						while (true)
						{
							if (ConnectNamedPipe(LogPipeH, nullptr))std::cout << "LogPipe Client link success!!!" << std::endl;
							else if (GetLastError() == ERROR_NO_DATA)
							{
								DisconnectNamedPipe(LogPipeH);
								std::cout << "LogPipe Client Disconnect" << std::endl;
								continue;
							}
							size_t len = OutQueue.size();
							for (int i = 0; i != len; i++)
							{
								lock.lock();
								std::string& str = OutQueue.front();
								lock.unlock();
								DWORD temp;
								if (WriteFile(LogPipeH, str.c_str(), (DWORD)str.size() + 1, &temp, nullptr))
								{
									lock.lock();
									OutQueue.pop();
									lock.unlock();
								}
								else if (GetLastError() == ERROR_NO_DATA)  break;
							}
							Sleep(10);
						}
					}).detach();

			if (CtrlPipeH)
				std::thread([]
					{
						if (CtrlPipeH)std::cout << "CtrlPipe open success" << std::endl;
						std::unique_lock lock(PipeIO::InQueuemtx, std::defer_lock);
						while (true)
						{
							if (ConnectNamedPipe(CtrlPipeH, nullptr))std::cout << "CtrlPipe Client link success!!!" << std::endl;
							else if (GetLastError() == ERROR_NO_DATA)
							{
								DisconnectNamedPipe(CtrlPipeH);
								std::cout << "CtrlPipe Client Disconnect" << std::endl;
								continue;
							}
							DWORD framelen, temp;
							while (ReadFile(CtrlPipeH, &framelen, sizeof(framelen), &temp, nullptr))
							{
								if (framelen == 0)continue;
								std::unique_ptr<char[]> buf(new char[framelen]);
								if (ReadFile(CtrlPipeH, buf.get(), framelen, &temp, nullptr) && framelen == temp)
								{
									std::cout << buf.get() << std::endl;
									lock.lock();
									InQueue.emplace(ctrlframe{ framelen,std::move(buf) });
									lock.unlock();
								}
							}
							if (GetLastError() == ERROR_NO_DATA)continue;
						}
					}).detach();
			PipeInit.detach();
		});

	const PipeIO& PipeIO::operator<<(auto&& str)const
	{
		ss << std::forward<decltype(str)>(str);
		std::unique_lock lockqueue(OutQueuemtx, std::try_to_lock);
		std::unique_lock lockss(ssmtx, std::try_to_lock);
		OutQueue.emplace(ss.str());
		ss.str("");
		ss.clear();
		return *this;
	}

	const PipeIO& PipeIO::operator>>(ctrlframe& cf)const
	{
		std::unique_lock lock(InQueuemtx, std::try_to_lock);
		cf = std::move(InQueue.front());
		InQueue.pop();
		return *this;
	}
}