#pragma once
#include"common.h"
namespace Pipe
{
	struct CtrlFrame
	{

	};

	constexpr auto MsgPipe = L"\\\\.\\pipe\\MsgPipe";
	constexpr auto CtrlPipe = L"\\\\.\\pipe\\CtrlPipe";
	extern bool isexit;
	extern std::thread PipeInit;
	extern AutoHandle<> MsgPipeHandle, CtrlPipeHandle;


}