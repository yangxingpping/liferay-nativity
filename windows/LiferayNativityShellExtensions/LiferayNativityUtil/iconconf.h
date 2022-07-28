
#pragma once
#include "inc/export_flags/LiferayNativityUtilExport.h"
#include <string>

class LIFERAYNATIVITYUTIL_EXPORT  iconconf
{
public:
    static bool setMountPoints(std::string pt);
    static bool setNanoAddr(std::string addr);
    static std::wstring getMountPoints();
    static std::string getNanoAddr();

};
