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

#include "CommunicationSocket.h"

#include "nanomsg/reqrep.h"

#include "fmt/format.h"

#include <WinSock2.h>
#include <Ws2def.h>
#include <windows.h>

using namespace std;

#define DEFAULT_BUFLEN 4096

CommunicationSocket::CommunicationSocket(int port): _port(port), s(-1)
{
	WSADATA wsaData;

	HRESULT hResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (FAILED(hResult))
	{
		int error = WSAGetLastError();
	}
}

CommunicationSocket::~CommunicationSocket()
{
	WSACleanup();
}

bool CommunicationSocket::ReceiveResponseOnly(wstring* message)
{
	SOCKET clientSocket = INVALID_SOCKET;

	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (clientSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();

		return false;
	}

	struct sockaddr_in clientService;

	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr("10.8.8.3");
	clientService.sin_port = htons(_port);

	HRESULT hResult = connect(clientSocket, (SOCKADDR*) &clientService, sizeof(clientService));

	if (FAILED(hResult))
	{
		int error = WSAGetLastError();

		closesocket(clientSocket);

		return false;
	}

	int bytesRead;

	do
	{
		char rec_buf[DEFAULT_BUFLEN];
		bytesRead = recv(clientSocket, rec_buf, DEFAULT_BUFLEN, MSG_WAITALL);

		if (bytesRead > 0)
		{
			wchar_t* buf = new wchar_t[ bytesRead / 2 + 1];
			int value;

			int j = 0;

			for (int i = 0; i < bytesRead; i += 2)
			{
				value = rec_buf[i] << rec_buf[i + 1];

				buf[j] = btowc(value);

				j++;
			}

			buf[j] = 0;

			message->append(buf);

			delete[] buf;
		}
	}
	while (bytesRead > 0);

	hResult = shutdown(clientSocket, SD_BOTH);

	if (FAILED(hResult))
	{
		int error = WSAGetLastError();

		closesocket(clientSocket);

		return false;
	}

	closesocket(clientSocket);

	return true;
}

bool CommunicationSocket::_ConvertData(wchar_t* buf, int bytesRead, char* rec_buf)
{
	int value;

	int j = 0;

	for (int i = 0; i < bytesRead; i += 2)
	{
		value = rec_buf[i] << rec_buf[i + 1];

		buf[j] = btowc(value);

		j++;
	}

	return true;
}



bool CommunicationSocket::SendMessageReceiveResponse(const wchar_t* message, wstring* response)
{
	SOCKET clientSocket = INVALID_SOCKET;

	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (clientSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();

		return false;
	}

	struct sockaddr_in clientService;

	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr("10.8.8.3");
	clientService.sin_port = htons(_port);

	HRESULT hResult = connect(clientSocket, (SOCKADDR*) &clientService, sizeof(clientService));

	if (FAILED(hResult))
	{
		int error = WSAGetLastError();

		closesocket(clientSocket);

		return false;
	}

	size_t result = send(clientSocket, (char*)message, (int)(wcslen(message) * 2), 0);

	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		closesocket(clientSocket);

		return false;
	}

	// shutdown the connection since no more data will be sent

	result = shutdown(clientSocket, SD_SEND);

	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		closesocket(clientSocket);

		return false;
	}

	char rec_buf[DEFAULT_BUFLEN];

	int bytesRead = recv(clientSocket, rec_buf, DEFAULT_BUFLEN, MSG_WAITALL);

	if (bytesRead == SOCKET_ERROR || bytesRead == 0)
	{
		int error = WSAGetLastError();

		closesocket(clientSocket);

		return false;
	}

	wchar_t* buf = new wchar_t[ bytesRead / 2 ];

	int value;
	int j = 0;

	for (int i = 0; i < bytesRead; i += 2)
	{
		value = rec_buf[i] << rec_buf[i + 1];

		buf[j] = btowc(value);

		j++;
	}

	*response = buf;

	delete[] buf;

	result = shutdown(clientSocket, SD_BOTH);

	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		closesocket(clientSocket);

		return false;
	}

	closesocket(clientSocket);

	return true;
}

bool CommunicationSocket::SendMessageReceiveResponseNano(std::string& req, std::string* rep)
{
	bool ret = false;
	*rep =fmt::format("{}", 10).c_str();
	if (s == -1)
	{
		int timeo = 1000;
		s = nn_socket(AF_SP, NN_REQ);
		nn_setsockopt(s, NN_SOL_SOCKET, NN_SNDTIMEO,
			&timeo, sizeof(timeo));

		nn_setsockopt(s, NN_SOL_SOCKET, NN_RCVTIMEO,
			&timeo, sizeof(timeo));
	}
	auto addr = iconconf::getNanoAddr();
	int op = nn_connect(s, addr.c_str());
	if (op < 0)
	{
		nn_close(s);
		s = -1;
		return ret;
	}
	char rec_buf[DEFAULT_BUFLEN];

	op = nn_send(s, req.c_str(), req.length(), 0);
	if (op != req.length())
	{
		*rep = fmt::format("{}", 11).c_str();
		nn_close(s);
		s = -1;
		return ret;
	}

	op = nn_recv(s, rec_buf, DEFAULT_BUFLEN, 0);
	if (op > 0)
	{
		*rep = rec_buf[0];
	}
	ret = true;
	return ret;
}
