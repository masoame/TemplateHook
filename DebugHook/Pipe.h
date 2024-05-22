#pragma once
#include"common.hpp"
namespace Pipe
{
    struct ctrlframe
    {
        DWORD framelen;
        std::unique_ptr<char[]> buf;
    };
    
    struct PipeIO
    {
        static BOOL PipeLink();
        static BOOL PipeClose();
        static HANDLE trid;

        static AutoHandle<> LogPipeH;
        static AutoHandle<> CtrlPipeH;
        
        static std::queue<ctrlframe> CtrlQueue;
        static std::queue<std::string> LogQueue;
        static std::mutex LogQueuemtx;
        static std::mutex CtrlQueuemtx;

        const PipeIO& operator >>(std::string& str)
        {
            std::unique_lock lock(LogQueuemtx, std::try_to_lock);
            if (!LogQueue.empty())
            {
                str = LogQueue.front();
                LogQueue.pop();
            }
            return *this;
        }
        const PipeIO& operator <<(ctrlframe&& cf)
        {
            std::unique_lock lock(CtrlQueuemtx, std::try_to_lock);
            CtrlQueue.emplace(std::forward<ctrlframe>(cf));
            return *this;
        }
    };
}