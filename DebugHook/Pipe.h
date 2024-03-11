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
        static std::stringstream Msgss;

        static std::mutex Ctrlssmtx;
        static std::mutex Msgssmtx;

        static std::queue<ctrlframe> CtrlQueue;

        const PipeIO& operator <<(auto&& str)
        {
            std::unique_lock lock(Ctrlss,std::try_to_lock);
            Ctrlss << std::forward(str);
            return *this;
        }
    };
}