#include"Pipe.h"
namespace Pipe
{
	constexpr const auto& LogPipeName = L"\\\\.\\pipe\\LogPipe";
	constexpr const auto& CtrlPipeName = L"\\\\.\\pipe\\CtrlPipe";

	AutoHandle<> PipeIO::LogPipeH{};
	AutoHandle<> PipeIO::CtrlPipeH{};

	std::mutex LogQueuemtx;
	std::mutex CtrlQueuemtx;


	std::thread PipeIO::PipeInit([]
	{
		SECURITY_ATTRIBUTES logsa{0},ctrlsa{0};
		logsa.nLength=sizeof(SECURITY_ATTRIBUTES);
		ctrlsa.nLength=sizeof(SECURITY_ATTRIBUTES);

		LogPipeH = CreateFileW(LogPipeName,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE |FILE_SHARE_DELETE,&logsa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
		CtrlPipeH =CreateFileW(CtrlPipeName,GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,&ctrlsa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);

		if(LogPipeH)
		std::thread([]
		{
			std::unique_lock lock(PipeIO::LogQueuemtx, std::defer_lock);
			std::string temp;
			char buf[1024];
			buf[1023] = 0;
			DWORD len;
			while (ReadFile(PipeHandle, buf, 1023, &len, nullptr))
			{
				if (len == 1023 && buf[1022] != 0)
				{
					temp += buf;
					continue;
				}
				temp += buf;
				std::cout << temp;
				temp = "";
			}

		}).detach();

		if(CtrlPipeH)
		std::thread([]
		{

		}).detach();


		PipeInit.detach();
	});
}
