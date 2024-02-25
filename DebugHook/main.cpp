//包含用于调试头文件
#include"DebugHook.h"

int main()
{
    auto a = DebugHook::SearchProcess(L"notepad++.exe");
    if (a != nullptr)
    {
        DebugHook::InjectDll(a->th32ProcessID, L"DllHook.dll");
        std::cout << "Inject success !!" << std::endl;
    }





    system("pause");
}

