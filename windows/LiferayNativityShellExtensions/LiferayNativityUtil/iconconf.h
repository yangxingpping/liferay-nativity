
#pragma once
#include "inc/export_flags/LiferayNativityUtilExport.h"
#include <string>

class LIFERAYNATIVITYUTIL_EXPORT  iconconf
{
public:
    static bool setMountPoint(std::string pt);
    static bool setNanoAddr(std::string addr);
    static std::string getNanoAddr();
};
