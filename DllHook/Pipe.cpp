#include"Pipe.h"

namespace Pipe
{

	constexpr auto& MsgPipeName = L"\\\\.\\pipe\\MsgPipe";
	constexpr auto& CtrlPipeName = L"\\\\.\\pipe\\CtrlPipe";

	std::queue<std::string> PipeIO::OutQueue;
	std::mutex PipeIO::OutQueuemtx;
	AutoHandle<> PipeIO::MsgPipeH{};
	std::stringstream PipeIO::ss;
	std::mutex PipeIO::ssmtx;

	const PipeIO pout, pin, io;
	const std::string pendl("\n");
	std::thread PipeIO::PipeInit([]
		{
			MsgPipeH = CreateNamedPipeW(MsgPipeName, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE, 1, 0, 0, 0, nullptr);
			if (MsgPipeH)std::cout << "pipe open success" << std::endl;

			std::thread([]
				{
					std::unique_lock lock(PipeIO::OutQueuemtx, std::defer_lock);
					while (true)
					{
						if (ConnectNamedPipe(MsgPipeH, nullptr))std::cout << "客户端连接成功" << std::endl;
						else if (GetLastError() == ERROR_NO_DATA)
						{
							DisconnectNamedPipe(MsgPipeH);
							std::cout << "断开连接成功" << std::endl;
						}

						size_t len = OutQueue.size();
						for (int i = 0; i != len; i++)
						{
							lock.lock();
							std::string& str = OutQueue.front();
							lock.unlock();
							DWORD len;
							if (!WriteFile(MsgPipeH, str.c_str(), (DWORD)str.size() + 1, &len, nullptr))
							{
								if (GetLastError() == ERROR_NO_DATA)
								{
									DisconnectNamedPipe(MsgPipeH);
									std::cout << "断开连接成功" << std::endl;
									break;
								}
							}
							lock.lock();
							OutQueue.pop();
							lock.unlock();
						}
						Sleep(10);
					}
				}).detach();
			PipeInit.detach();
		});

	PipeIO& PipeIO::operator>>(char*)
	{
		return *this;
	}
}

