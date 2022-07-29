/**
 * Copyright (c) 2000-2013 Liferay, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#include "LiferayNativityOverlay.h"
#include "fmt/format.h"

using namespace std;

#pragma comment(lib, "shlwapi.lib")

extern HINSTANCE instanceHandle;

#define IDM_DISPLAY 0
#define IDB_OK 101


LiferayNativityOverlay::LiferayNativityOverlay(IconType type): _communicationSocket(0), _referenceCount(1), _icon(type)
{
}

LiferayNativityOverlay::~LiferayNativityOverlay(void)
{
}

IFACEMETHODIMP_(ULONG) LiferayNativityOverlay::AddRef()
{
	return InterlockedIncrement(&_referenceCount);
}

IFACEMETHODIMP LiferayNativityOverlay::QueryInterface(REFIID riid, void** ppv)
{
	HRESULT hResult = S_OK;

	if (IsEqualIID(IID_IUnknown, riid) ||  IsEqualIID(IID_IShellIconOverlayIdentifier, riid))
	{
		*ppv = static_cast<IShellIconOverlayIdentifier*>(this);
	}
	else
	{
		hResult = E_NOINTERFACE;
		*ppv = NULL;
	}

	if (*ppv)
	{
		AddRef();
	}

	return hResult;
}

IFACEMETHODIMP_(ULONG) LiferayNativityOverlay::Release()
{
	ULONG cRef = InterlockedDecrement(&_referenceCount);
	if (0 == cRef)
	{
		delete this;
	}

	return cRef;
}

IFACEMETHODIMP LiferayNativityOverlay::GetPriority(int* pPriority)
{
	pPriority = 0;

	return S_OK;
}

IFACEMETHODIMP LiferayNativityOverlay::IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib)
{
	auto isvv = _GetIconType(pwszPath);
	if (_icon == isvv)
	{
		/*auto vecho = fmt::format("++++true cur {}, my class {} pt {}", (int)(isvv), (int)(_icon), fmt::ptr(this));
		string* rep = new string();
		_communicationSocket->SendMessageReceiveResponseNano(vecho, rep);
		delete rep;*/

		return S_OK;
	}
	else
	{
		/*auto vecho = fmt::format("----false cur {}, my class {}, pt {}", (int)(isvv), (int)(_icon), fmt::ptr(this));
		string* rep = new string();
		_communicationSocket->SendMessageReceiveResponseNano(vecho, rep);
		delete rep;*/
		return S_FALSE;
	}
}

IFACEMETHODIMP LiferayNativityOverlay::GetOverlayInfo(PWSTR pwszIconFile, int cchMax, int* pIndex, DWORD* pdwFlags)
{
	*pIndex = 0;

	*pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;

	

	std::string path = fmt::format("C:\\XPStyle\\{}.ico", ((int)_icon)).c_str();
	auto wpath = StringUtil::toWstring(path);
	wcscpy_s(pwszIconFile, cchMax, wpath.c_str());
	return S_OK;
}

bool LiferayNativityOverlay::_IsOverlaysEnabled()
{
	int* enable = new int();
	bool success = false;

	if (RegistryUtil::ReadRegistry(REGISTRY_ROOT_KEY, REGISTRY_ENABLE_OVERLAY, enable))
	{
		if (enable)
		{
			success = true;
		}
	}

	delete enable;

	return success;
}

IconType LiferayNativityOverlay::_GetIconType(const wchar_t* filePath)
{
	IconType ret = IconType::NetError;
	if (_communicationSocket == 0)
	{
		_communicationSocket = new CommunicationSocket(PORT);
	}

	string req = fmt::format("lllllllllllllllllllllllllllllllllllll {}", (int)(_icon));
	string* response = new string();
	bool bret = _communicationSocket->SendMessageReceiveResponseNano(req , response);
	string vv2 = *response;
	/*{
		string str = fmt::format("0f{}", vv2);
		_communicationSocket->SendMessageReceiveResponseNano(str, response);
		ret = IconType::Burning;
		return ret;
	}*/
	ret = (IconType)(atoi(vv2.c_str()));
	delete response;
	response = nullptr;
	return ret;
}
