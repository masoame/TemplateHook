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
        static std::thread PipeInit;

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
        const PipeIO& operator <<(ctrlframe&& str)
        {
            std::unique_lock lock(CtrlQueue, std::try_to_lock);
            
            return *this;
        }
    };
}