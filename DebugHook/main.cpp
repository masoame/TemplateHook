#include"DebugHook.h"
#include"Pipe.h"
#include"AnalyzePE_API.hpp"

int main()
{
	auto a = AnalyzePE::LoadFile("D:\\vs\\TemplateHook\\x64\\Debug\\test.exe");
	if (a.get() == nullptr)return -1;
	AnalyzePE::PrintPE_Message(a.get());

	system("pause");
}