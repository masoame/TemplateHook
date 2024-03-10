#include"Pipe.h"

namespace Pipe
{
	constexpr auto& MsgPipeName = L"\\\\.\\pipe\\MsgPipe";
	constexpr auto& CtrlPipeName = L"\\\\.\\pipe\\CtrlPipe";

	std::queue<std::string> PipeIO::OutQueue;
	std::queue<ctrlframe> PipeIO::InQueue;
	std::mutex PipeIO::OutQueuemtx;
	std::mutex PipeIO::InQueuemtx;

	AutoHandle<> PipeIO::MsgPipeH{};
	AutoHandle<> PipeIO::CtrlPipeH{};
	std::stringstream PipeIO::ss;
	std::mutex PipeIO::ssmtx;

	const PipeIO pout, pin, io;
	const std::string pendl("\n");
	std::thread PipeIO::PipeInit([]
		{
			MsgPipeH = CreateNamedPipeW(MsgPipeName, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE, 1, 0, 0, 0, nullptr);
			if (MsgPipeH)std::cout << "MsgPipe open success" << std::endl;
			CtrlPipeH = CreateNamedPipeW(CtrlPipeName, PIPE_ACCESS_INBOUND, PIPE_READMODE_BYTE, 1, 0, 0, 0, nullptr);
			if (CtrlPipeH)std::cout << "CtrlPipe open success" << std::endl;

			std::thread([]
				{
					std::unique_lock lock(PipeIO::OutQueuemtx, std::defer_lock);
					while (true)
					{
						if (ConnectNamedPipe(MsgPipeH, nullptr))std::cout << "MsgPipe Client link success!!!" << std::endl;
						else if (GetLastError() == ERROR_NO_DATA)
						{
							DisconnectNamedPipe(MsgPipeH);
							std::cout << "MsgPipe Client Disconnect" << std::endl;
							continue;
						}
						size_t len = OutQueue.size();
						for (int i = 0; i != len; i++)
						{
							lock.lock();
							std::string& str = OutQueue.front();
							lock.unlock();
							DWORD temp;
							if (WriteFile(MsgPipeH, str.c_str(), (DWORD)str.size() + 1, &temp, nullptr))
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
			std::thread([]
				{
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
						DWORD framelen,temp;
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

	const PipeIO& PipeIO::operator>>(ctrlframe& cf)const
	{
		std::unique_lock lock(InQueuemtx,std::try_to_lock);
		cf = std::move(InQueue.front());
		InQueue.pop();
		return *this;
	}
<<<<<<< HEAD
=======


>>>>>>> 5a5c637ab10de9330a59549840b722e893214c16
}