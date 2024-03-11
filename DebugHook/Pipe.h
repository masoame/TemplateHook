#pragma once
#include"common.h"
namespace Pipe
{
    struct ctrlframe
    {
        DWORD framelen;
        std::unique_ptr<char[]> buf;
    };
    
    struct PipeIO
    {
        static std::stringstream Ctrlss;
        static std::stringstream Logss;

        static std::mutex Ctrlssmtx;
        static std::mutex Logssmtx;

        static std::queue<ctrlframe> CtrlQueue;
        static std::queue<std::string> LogQueue;

        const PipeIO& operator <<(auto&& str)
        {
            std::unique_lock lock(Ctrlssmtx,std::try_to_lock);
            Ctrlss << std::forward(str);
            return *this;
        }
        const PipeIO& operator >>(std::string& str)
        {
            if(!LogQueue.empty()) str=LogQueue.front();
            return *this;
        }
    };
}