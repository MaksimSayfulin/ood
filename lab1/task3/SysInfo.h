#pragma once
#include <cstdint>
#include <string>

class SysInfo
{
public:
    static std::string GetOSName() ;
    static std::string GetOSVersion() ;
    static uint64_t GetFreeMemory() ;
    static uint64_t GetTotalMemory() ;
    static unsigned GetProcessorCount() ;
};
