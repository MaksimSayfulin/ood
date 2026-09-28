#include "SysInfo.h"
#include <thread>

unsigned ToMB(const unsigned long long b) {
    return b / 1024 / 1024;
}

#if defined(_WIN32)
    #include <windows.h>
    #include <versionhelpers.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <sys/sysinfo.h>
    #include <sys/utsname.h>
#endif

std::string SysInfo::GetOSName()
{
    #if defined(_WIN32)
        return "Windows";
    #elif defined(__linux__)
        utsname u{};
        if (uname(&u) == 0) {
            return u.sysname;
        }
        return "Linux";
    #else
        return "Unknown OS";
    #endif
}

std::string SysInfo::GetOSVersion()
{
    #if defined(_WIN32)
        if (IsWindows10OrGreater()) {
            return "Windows 10 or Greater";
        }
        if (IsWindows8OrGreater()) {
            return "Windows 8 or Greater";
        }
        if (IsWindows7OrGreater()) {
            return "Windows 7 or Greater";
        }
        return "Older Windows";
    #elif defined(__linux__)
        utsname u{};
        if (uname(&u) == 0) {
            return std::string(u.release) + " (" + u.version + ")";
        }
        return "Unknown Linux Version";
    #else
        return "Unknown";
    #endif
}

uint64_t SysInfo::GetFreeMemory()
{
    #if defined(_WIN32)
        MEMORYSTATUSEX mem = { sizeof(mem) };
        if (GlobalMemoryStatusEx(&mem)) {
            return ToMB(mem.ullAvailPhys);
        }
        return 0;
    #elif defined(__linux__)
        struct sysinfo info{};
        if (sysinfo(&info) == 0) {
            return ToMB(info.freeram * info.mem_unit);
        }
        return 0;
    #else
        return 0;
    #endif
}

uint64_t SysInfo::GetTotalMemory()
{
    #if defined(_WIN32)
        MEMORYSTATUSEX mem = { sizeof(mem) };
        if (GlobalMemoryStatusEx(&mem)) {
            return ToMB(mem.ullTotalPhys);
        }
        return 0;
    #elif defined(__linux__)
        struct sysinfo info{};
        if (sysinfo(&info) == 0) {
            return ToMB(info.totalram * info.mem_unit);
        }
        return 0;
    #else
        return 0;
    #endif
}

unsigned SysInfo::GetProcessorCount()
{
    #if defined(_WIN32)
        SYSTEM_INFO sys;
        GetNativeSystemInfo(&sys);
        return sys.dwNumberOfProcessors;
    #elif defined(__linux__)
        return sysconf(_SC_NPROCESSORS_ONLN);
    #else
        return 1;
    #endif
}