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

#include "nanomsg/reqrep.h"
#include <assert.h>
#include "json/json.h"

#include <atlstr.h>

using namespace std;

#pragma comment(lib, "shlwapi.lib")

extern HINSTANCE instanceHandle;

#define IDM_DISPLAY 0
#define IDB_OK 101

LiferayNativityOverlay::LiferayNativityOverlay(): _communicationSocket(0), _referenceCount(1), _nanomsgsocket(-1)
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
	HRESULT hRef = S_FALSE;
	
	if (!_IsOverlaysEnabled())
	{
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}

	if (!FileUtil::IsFileFiltered(pwszPath))
	{
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}

	_icon = _IsMonitoredFileStateNanomsg(pwszPath);

	return MAKE_HRESULT(S_OK, 0, 0);
}

IFACEMETHODIMP LiferayNativityOverlay::GetOverlayInfo(PWSTR pwszIconFile, int cchMax, int* pIndex, DWORD* pdwFlags)
{
	*pIndex = 0;

	*pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;

	if (GetModuleFileName(instanceHandle, pwszIconFile, cchMax) == 0)
	{
		HRESULT hResult = HRESULT_FROM_WIN32(GetLastError());

		return hResult;
	}

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

bool LiferayNativityOverlay::_IsMonitoredFileState(const wchar_t* filePath)
{
	bool needed = false;

	if (_communicationSocket == 0)
	{
		_communicationSocket = new CommunicationSocket(PORT);
	}

	Json::Value jsonRoot;

	jsonRoot[NATIVITY_COMMAND] = NATIVITY_GET_FILE_ICON_ID;
	jsonRoot[NATIVITY_VALUE] = StringUtil::toString(filePath);

	Json::FastWriter jsonWriter;

	wstring* message = new wstring();

	message->append(StringUtil::toWstring(jsonWriter.write(jsonRoot)));

	wstring* response = new wstring();

	if (!_communicationSocket->SendMessageReceiveResponse(message->c_str(), response))
	{
		delete message;
		delete response;

		return false;
	}

	Json::Reader jsonReader;
	Json::Value jsonResponse;

	if (!jsonReader.parse(StringUtil::toString(*response), jsonResponse))
	{
		delete message;
		delete response;

		return false;
	}

	Json::Value jsonValue = jsonResponse.get(NATIVITY_VALUE, "");

	wstring valueString = StringUtil::toWstring(jsonValue.asString());

	if (valueString.size() == 0)
	{
		delete message;
		delete response;

		return false;
	}

	int state = _wtoi(valueString.c_str());

	if (state == OVERLAY_ID)
	{
		needed = true;
	}

	delete message;
	delete response;

	return needed;
}

IconType LiferayNativityOverlay::_IsMonitoredFileStateNanomsg(const wchar_t* filePath)
{
	IconType ret = IconType::NetError;
	auto naddr = _GetNanomsgAddr();
	int nnop = 0;
	if (_nanomsgsocket == -1)
	{
		_nanomsgsocket = nn_socket(AF_SP, NN_REQ);
		if (_nanomsgsocket == -1)
		{
			return ret;
		}
		nnop = nn_connect(_nanomsgsocket, naddr.c_str());
		if (nnop != 0)
		{
			ret = IconType::NetError;
			nn_close(_nanomsgsocket);
			_nanomsgsocket = -1;
			return ret;
		}
	}

	Json::Value jsonRoot;

	jsonRoot[NATIVITY_COMMAND] = NATIVITY_GET_FILE_ICON_ID;
	jsonRoot[NATIVITY_VALUE] = StringUtil::toString(filePath);

	Json::FastWriter jsonWriter;

	wstring* message = new wstring();

	message->append(StringUtil::toWstring(jsonWriter.write(jsonRoot)));

	wstring* response = new wstring();

	if (!_communicationSocket->SendMessageReceiveResponse(message->c_str(), response))
	{
		delete message;
		delete response;

		ret = IconType::NetError;
		return ret;
	}

	Json::Reader jsonReader;
	Json::Value jsonResponse;

	if (!jsonReader.parse(StringUtil::toString(*response), jsonResponse))
	{
		delete message;
		delete response;
		ret = IconType::UploadFailed;
		return ret;
	}

	Json::Value jsonValue = jsonResponse.get(NATIVITY_VALUE, "");

	wstring valueString = StringUtil::toWstring(jsonValue.asString());

	if (valueString.size() == 0)
	{
		delete message;
		delete response;

		ret = IconType::UploadFailed;
		return ret;
	}

	int state = _wtoi(valueString.c_str());

	ret = static_cast<IconType>(state);

	delete message;
	delete response;

	return ret;
}

std::string LiferayNativityOverlay::_GetNanomsgAddr()
{
	//can get from registry
	return "tcp://127.0.0.1:/10086";
}
