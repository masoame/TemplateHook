#include"Pipe.h"

namespace Pipe
{
	constexpr auto& MsgPipeName = L"\\\\.\\pipe\\MsgPipe";
	constexpr auto& CtrlPipeName = L"\\\\.\\pipe\\CtrlPipe";

	std::queue<std::string> PipeIO::OutQueue;
	std::mutex PipeIO::OutQueuemtx;

	AutoHandle<> PipeIO::MsgPipeH{};

	std::thread PipeIO::PipeInit([]
		{
			MsgPipeH = CreateNamedPipeW(MsgPipeName, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE, 1, 0, 0, 0, nullptr);
			if (MsgPipeH)std::cout << "pipe open success" << std::endl;



			std::thread([]
				{
					while (ConnectNamedPipe(MsgPipeH,nullptr))
					{
						//WriteFile()
					}
				}).detach();






			PipeInit.detach();
		});

	PipeIO& PipeIO::operator<<(const std::string& str)
	{
		DWORD sendlen;	
		BOOL status = WriteFile(MsgPipeH, str.c_str(), (DWORD)str.size() + 1, &sendlen, nullptr);
		return *this;
	}
	PipeIO& PipeIO::operator>>(char*)
	{

	}
}

