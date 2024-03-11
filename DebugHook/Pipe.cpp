#include"Pipe.h"
namespace Pipe
{
	constexpr const auto& LogPipeName = L"\\\\.\\pipe\\LogPipe";
	constexpr const auto& CtrlPipeName = L"\\\\.\\pipe\\CtrlPipe";

	AutoHandle<> PipeIO::LogPipeH{};
	AutoHandle<> PipeIO::CtrlPipeH{};


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

		}).detach();

		if(CtrlPipeH)
		std::thread([]
		{

		}).detach();


		PipeInit.detach();
	});
}
