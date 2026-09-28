#include <iostream>
#include "SysInfo.h"

int main() {
    std::cout << "OS name: " << SysInfo::GetOSName() << std::endl;
    std::cout << "OS version: " << SysInfo::GetOSVersion() << std::endl;
    std::cout << "Processors: " << SysInfo::GetProcessorCount() << std::endl;
    std::cout << "Free memory: " << SysInfo::GetFreeMemory() << " MB" << std::endl;
    std::cout << "Total memory: " << SysInfo::GetTotalMemory() << " MB" << std::endl;

}
