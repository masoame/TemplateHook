#include"Pipe.h"
namespace Pipe
{
	AutoHandle<> MsgPipeHandle{ 0 }, CtrlPipeHandle{ 0 };
	bool isexit;
	std::thread PipeInit([]
		{
			SECURITY_ATTRIBUTES sa_out{ 0 }, sa_in{ 0 };
			sa_out.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa_in.nLength = sizeof(SECURITY_ATTRIBUTES);

			MsgPipeHandle = CreateNamedPipeW(MsgPipe, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE, 2, 0, 0, 0, &sa_out);
			if (!MsgPipeHandle) throw "error MsgPipeHandle!!!";
			CtrlPipeHandle = CreateNamedPipeW(CtrlPipe, PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE, 2, 0, 0, 0, &sa_in);
			if (!CtrlPipeHandle) throw "error CtrlPipeHandle!!!";

			std::thread([]
				{
					while (ConnectNamedPipe(CtrlPipeHandle, nullptr) == FALSE)continue;
					DWORD len;
					CtrlFrame temp;
					while (true)
					{
						if (ReadFile(CtrlPipeHandle, &temp, sizeof(CtrlFrame), &len, nullptr) == FALSE)
						{

						}
						else
						{

						}

					}


				}).detach();

		});
}