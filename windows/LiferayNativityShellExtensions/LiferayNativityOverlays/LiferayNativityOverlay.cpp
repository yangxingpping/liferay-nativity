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
#include "nanomsg/pubsub.h"
#include <assert.h>
#include "json/json.h"

#include "iconconf.h"

#include "fmt/format.h"

#include <atlstr.h>
#include <string>

using namespace std;

#pragma comment(lib, "shlwapi.lib")

extern HINSTANCE instanceHandle;

#define IDM_DISPLAY 0
#define IDB_OK 101

void sendLog(std::string log)
{
	/*string logpath = "C:\\Users\\abc\\Desktop\\Release\\x64\aaaaa.log";
	FILE* ap = fopen(logpath.c_str(), "a");
	if (ap!=NULL)
	{
		fwrite(log.c_str(), log.length(), 1, ap);
		fclose(ap);
		ap = NULL;
	}*/
	/*char logaddr[] = "tcp://10.8.8.3:10087";
	int sock;
	sock = nn_socket(AF_SP, NN_PUB);
	nn_connect(sock, logaddr);
	nn_send(sock, log.c_str(), log.length(), 0);
	nn_close(sock);*/
}

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
	
	/*if (!_IsOverlaysEnabled())
	{
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}

	if (!FileUtil::IsFileFiltered2(pwszPath))
	{
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}*/

	_icon = _IsMonitoredFileStateNanomsg(pwszPath);

	return MAKE_HRESULT(S_OK, 0, 0);
}

IFACEMETHODIMP LiferayNativityOverlay::GetOverlayInfo(PWSTR pwszIconFile, int cchMax, int* pIndex, DWORD* pdwFlags)
{
	*pIndex = 0;

	*pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;

	std::string path = fmt::format("C:\\XPStyle\\{}.ico", (int)(_icon)).c_str();
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
	auto naddr = iconconf::getNanoAddr();
	int nnop = 0;
	void* recvbuf = nullptr;
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
			ret = IconType::Frozen;
			nn_close(_nanomsgsocket);
			_nanomsgsocket = -1;
			return ret;
		}
	}

	Json::Value jsonRoot;

	jsonRoot[NATIVITY_COMMAND] = NATIVITY_GET_FILE_ICON_ID;
	jsonRoot[NATIVITY_VALUE] = StringUtil::toString(filePath);
	jsonRoot["classpointer"] = fmt::format("{}", fmt::ptr(this));

	Json::FastWriter jsonWriter;

	string* message = new string();

	message->append(jsonWriter.write(jsonRoot));

	wstring* response = new wstring();
	nnop = nn_send(_nanomsgsocket, message->c_str(), message->size(), 0);
	if (nnop != message->length())
	{
		delete message;
		delete response;
		ret = IconType::NetError;
		return ret;
	}
	Json::Reader jsonReader;
	Json::Value jsonResponse;
	nnop = nn_recv(_nanomsgsocket, &recvbuf, NN_MSG, 0);
	if (nnop < 0)
	{
		delete message;
		delete response;
		ret = IconType::Uploading;
		return ret;
	}
	string strrep((char*)recvbuf, (char*)(recvbuf)+nnop);
	
	int state = atoi(strrep.c_str());

	ret = static_cast<IconType>(state);

	delete message;
	delete response;
	nn_freemsg(recvbuf);
	recvbuf = nullptr;
	return ret;
}
