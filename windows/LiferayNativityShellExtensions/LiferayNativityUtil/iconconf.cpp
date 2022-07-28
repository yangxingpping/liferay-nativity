
#include "iconconf.h"

#include "WinReg.hpp"
#include "UtilConstants.h"

#include "spdlog/spdlog.h"

using winreg::RegKey;
using winreg::RegException;
using winreg::RegExpected;


bool iconconf::setMountPoint(std::string pt)
{
	bool bret = false;
	std::wstring wpt(pt.begin(), pt.end());
	try
	{
		RegKey key{ HKEY_CURRENT_USER, REGISTRY_ROOT_KEY };
		key.SetStringValue(REGISTRY_MOUNT_POINT, wpt);
		bret = true;
	}
	catch (const RegException& exp)
	{
		SPDLOG_WARN("set registry   {} failed with ", pt);
	}
	catch (...)
	{
		SPDLOG_WARN("set registry   {} failed with unexpected exception", pt);
	}
	return bret;
}

bool iconconf::setNanoAddr(std::string addr)
{
	bool bret = false;
	std::wstring wpt(addr.begin(), addr.end());
	try
	{
		RegKey key{ HKEY_CURRENT_USER, REGISTRY_ROOT_KEY };
		key.SetStringValue(REGISTRY_LOCAL_NNADDR, wpt);
		bret = true;
	}
	catch (const RegException& exp)
	{
		SPDLOG_WARN("set registry   {} failed with {}", addr, exp.what());
	}
	catch (...)
	{
		SPDLOG_WARN("set registry   {} failed with unexpected exception", addr);
	}
	return bret;
}

std::string iconconf::getNanoAddr()
{
	std::string ret;
	try
	{
		RegKey key{ HKEY_CURRENT_USER, REGISTRY_ROOT_KEY };
		auto wret = key.GetStringValue(REGISTRY_LOCAL_NNADDR);

	}
	catch (const RegException& exp)
	{
		SPDLOG_WARN("get registry local nano addr failed with {}", exp.what());
	}
	catch (...)
	{
		SPDLOG_WARN("get registry local nano addr  failed with unexpected exception");
	}
	return ret;
}

