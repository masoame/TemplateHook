#include"Pipe.h"
namespace Pipe
{
	constexpr const auto& LogPipeName = L"\\\\.\\pipe\\LogPipe";
	constexpr const auto& CtrlPipeName = L"\\\\.\\pipe\\CtrlPipe";

	AutoHandle<> PipeIO::LogPipeH{};
	AutoHandle<> PipeIO::CtrlPipeH{};

	std::mutex PipeIO::LogQueuemtx;
	std::mutex PipeIO::CtrlQueuemtx;

	std::queue<ctrlframe> PipeIO::CtrlQueue;
    std::queue<std::string> PipeIO::LogQueue;


	std::thread PipeIO::PipeInit([]
	{
		SECURITY_ATTRIBUTES logsa{0},ctrlsa{0};
		logsa.nLength=sizeof(SECURITY_ATTRIBUTES);
		ctrlsa.nLength=sizeof(SECURITY_ATTRIBUTES);

		LogPipeH = CreateFileW(LogPipeName,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE |FILE_SHARE_DELETE,&logsa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
		CtrlPipeH = CreateFileW(CtrlPipeName,GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,&ctrlsa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);

		if(LogPipeH)
		std::thread([]
		{
			std::cout << "success to link LogServer!!!!"<<std::endl;
			std::unique_lock lock(PipeIO::LogQueuemtx, std::defer_lock);
			std::string temp;
			char buf[1024];
			buf[1023] = 0;
			DWORD len;
			while (ReadFile(LogPipeH, buf, 1023, &len, nullptr))
			{
				if (len == 1023 && buf[1022] != 0)
				{
					temp += buf;
					continue;
				}
				temp += buf;
				std::cout << temp;
				lock.lock();
				LogQueue.emplace(temp);
				lock.unlock();
				temp = "";
			}

		}).detach();

		if(CtrlPipeH)
		std::thread([]
		{
			std::unique_lock lock(PipeIO::CtrlQueuemtx, std::defer_lock);
			std::cout << "success to link CtrlServer!!!!"<<std::endl;
			size_t len = CtrlQueue.size();
			for (int i = 0; i != len; i++)
			{
				lock.lock();
				ctrlframe& cf = CtrlQueue.front();
				lock.unlock();
				DWORD temp;
				if (WriteFile(LogPipeH, &cf.framelen, sizeof(cf.framelen), &temp, nullptr) && WriteFile(LogPipeH, cf.buf.get(), cf.framelen, &temp, nullptr))
				{
					lock.lock();
					CtrlQueue.pop();
					lock.unlock();
				}
				else if (GetLastError() == ERROR_NO_DATA)  break; 
			}

		}).detach();


		PipeInit.detach();
	});
}
