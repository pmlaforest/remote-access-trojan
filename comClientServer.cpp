/*********************************************************************
commClientServer.cpp - Fonctions de communication par socket
écrit par: Pierre-Marc Laforest
Date: 2017-04-05
**********************************************************************/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include  <direct.h>
#include "comClientServer.h"


using namespace std;

//Defining the buffer size
#define BUFF_SIZE 1024
#define PORT_NUMB 3000
#define TAILLE_DES_BLOCS  1023
#define FICHIER_CMD_ENREGISTREES "outputCommand.txt"

// Helper macro for displaying errors
#define PRINTERROR(s)	cout<<"\n"<<s<<":"<<WSAGetLastError()

void server_initComAvecClient(short nPort, SOCKET* listenSocketPtr, SOCKET* remoteSocketPtr)
{
	// Create a TCP/IP stream socket to "listen" with
	*listenSocketPtr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*listenSocketPtr == INVALID_SOCKET)
	{
		PRINTERROR("socket()");
		return;
	}

	// Fill in the address structure
	SOCKADDR_IN saServer;
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;
	saServer.sin_port = htons(nPort);

	int nRet;
	nRet = bind(*listenSocketPtr, (LPSOCKADDR)&saServer, sizeof(struct sockaddr));
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("bind()");
		closesocket(*listenSocketPtr);
		return;
	}

	int nLen;
	nLen = sizeof(SOCKADDR);
	char szBuf[BUFF_SIZE];

	nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("gethostname()");
		closesocket(*listenSocketPtr);
		return;
	}

	nRet = listen(*listenSocketPtr, SOMAXCONN);
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("listen()");
		closesocket(*listenSocketPtr);
		return;
	}

	*remoteSocketPtr = accept(*listenSocketPtr, NULL, NULL);
	if (*remoteSocketPtr == INVALID_SOCKET)
	{
		PRINTERROR("accept()");
		closesocket(*listenSocketPtr);
		return;
	}
}

void client_initComAvecServer(char *szServer, short nPort, SOCKET* theSocketPtr)
{
	// Find the server
	LPHOSTENT lpHostEntry;

	lpHostEntry = gethostbyname(szServer);
	if (lpHostEntry == NULL)
	{
		PRINTERROR("gethostbyname()");
		return;
	}

	*theSocketPtr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*theSocketPtr == INVALID_SOCKET)
	{
		PRINTERROR("socket()");
		return;
	}

	// Fill in the address structure
	SOCKADDR_IN saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	saServer.sin_port = htons(nPort);

	int nRet = connect(*theSocketPtr, (LPSOCKADDR)&saServer, sizeof(struct sockaddr));
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("socket()");
		closesocket(*theSocketPtr);
		return;
	}
	return;
}

void envoiFichierParSocket(char* nomDuFichier, SOCKET remoteSocket)
{
	char szBuf[BUFF_SIZE];
	char ackBuf[BUFF_SIZE];
	int nRet, ack;

	// Envoi de la taille du fichier
	fstream fichierAEnvoyer;
	char tailleFichier[50];
	unsigned int begin, end, tailleFichierInt;
	fichierAEnvoyer.open(nomDuFichier, ios::binary | ios::in);

	if (fichierAEnvoyer.is_open())
	{
		begin = fichierAEnvoyer.tellg();
		fichierAEnvoyer.seekg(0, ios::end);
		end = fichierAEnvoyer.tellg();
		fichierAEnvoyer.seekg(0, ios::beg);
		tailleFichierInt = end - begin;
	}
	else { cout << "Impossible douvrir le fichier\n"; return; }

	_itoa_s(tailleFichierInt, tailleFichier, 10);
	strcpy_s(szBuf, tailleFichier);
	nRet = send(remoteSocket, szBuf, sizeof(szBuf), 0);

	unsigned int tailleDuFichier = std::stoi(szBuf);
	unsigned int nbPaquets = tailleDuFichier / sizeof(szBuf);
	unsigned int resteDernierPaquet = tailleDuFichier % sizeof(szBuf);

	// Envoi des premiers paquets
	for (int noPaquet = 0; noPaquet < nbPaquets; noPaquet++)
	{
		fichierAEnvoyer.read(szBuf, sizeof(szBuf));
		do { 
			ack = 0;
			nRet = send(remoteSocket, szBuf, sizeof(szBuf), 0);
			nRet = recv(remoteSocket, ackBuf, sizeof(ackBuf), 0);
			ack = atoi(ackBuf);
		} while (ack != 1);
	}
	// Envoi du dernier paquet
	fichierAEnvoyer.read(szBuf, resteDernierPaquet);
	do {
		ack = 0;
		nRet = send(remoteSocket, szBuf, sizeof(szBuf), 0);
		nRet = recv(remoteSocket, ackBuf, sizeof(ackBuf), 0);
		ack = atoi(ackBuf);
	} while (ack != 1);
	
	fichierAEnvoyer.close();
	return;
}

void receptionFichierParSocket(char *nomDuFichierARecevoir, SOCKET theSocket)
{
	char szBuf[BUFF_SIZE];
	char ackBuf[BUFF_SIZE];
	int nRet, ack;

	// Reception de la taille du fichier
	nRet = recv(theSocket, szBuf, sizeof(szBuf), 0);
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("recv()");
		closesocket(theSocket);
		return;
	}
	unsigned int tailleDuFichier = std::stoi(szBuf);
	unsigned int nbPaquets = tailleDuFichier / sizeof(szBuf);
	unsigned int resteDernierPaquet = tailleDuFichier % sizeof(szBuf);

	// Reception des paquets
	FILE* fichierAEcrirePtr;
	fopen_s(&fichierAEcrirePtr, nomDuFichierARecevoir, "w+b");

	unsigned int cumul = 0;
	unsigned int tailleDesBlocs = TAILLE_DES_BLOCS;
	
	for (int noPaquet = 0; noPaquet < nbPaquets; noPaquet++)
	{
		do {  
			nRet = recv(theSocket, szBuf, sizeof(szBuf), 0); 
			if (nRet == sizeof(szBuf))
				ack = 1;
			else
				ack = 0;
			sprintf_s(ackBuf, "%d", ack);			
			send(theSocket, ackBuf, sizeof(ackBuf), 0);
		} 
		while (ack != 1);
		fwrite(szBuf, 1, nRet, fichierAEcrirePtr);
	}
	// Reception du dernier paquet
	do {
		nRet = recv(theSocket, szBuf, sizeof(szBuf), 0);
		if (nRet == sizeof(szBuf))
			ack = 1;
		else
			ack = 0;
		sprintf_s(ackBuf, "%d", ack);
		send(theSocket, ackBuf, sizeof(ackBuf), 0);
	} while (ack != 1);
	fwrite(szBuf, 1, resteDernierPaquet, fichierAEcrirePtr);

	fclose(fichierAEcrirePtr);
	return;
}

int receptionStrParSocket(char* strARetourner, SOCKET socketPourCommuniquer)
{
	char szBuf[1024];
	char ackBuf[BUFF_SIZE];
	int posInStr = 0;
	int nRet, ack;

	memset(szBuf, 0, sizeof(szBuf));

	// Reception de la taille du string
	nRet = recv(socketPourCommuniquer, szBuf, sizeof(szBuf), 0);
	if (nRet == INVALID_SOCKET)
	{
		PRINTERROR("recv()");
		return -1;
	}

	int lengthOfStr = atoi(szBuf);
	int nbPaquets = lengthOfStr / sizeof(szBuf);
	int resteDernierPaquet = lengthOfStr % sizeof(szBuf);

	//Reception de tous les paquets pleins
	for (int noPaquets = 0; noPaquets < nbPaquets; noPaquets++)
	{
		do {
			nRet = recv(socketPourCommuniquer, szBuf, sizeof(szBuf), 0);
			if (nRet == INVALID_SOCKET)
			{
				PRINTERROR("recv()");
				return -1;
			}
			else if (nRet == sizeof(szBuf))
				ack = 1;
			else
				ack = 0;
			sprintf_s(ackBuf, "%d", ack);
			send(socketPourCommuniquer, ackBuf, sizeof(ackBuf), 0);
		} while (ack != 1);

		for (int charNo = 0; charNo < sizeof(szBuf); charNo++)
			strARetourner[posInStr++] = szBuf[charNo];

	}

	// Dernier paquet
	do {
		nRet = recv(socketPourCommuniquer, szBuf, sizeof(szBuf), 0);
		if (nRet == INVALID_SOCKET)
		{
			PRINTERROR("recv()");
			return -1;
		}
		else if (nRet == sizeof(szBuf))
			ack = 1;
		else
			ack = 0;
		sprintf_s(ackBuf, "%d", ack);
		send(socketPourCommuniquer, ackBuf, sizeof(ackBuf), 0);
	} while (ack != 1);
	
	for (int charNo = 0; charNo < resteDernierPaquet; charNo++)
		strARetourner[posInStr++] = szBuf[charNo];

	return 0;
}

int envoiStrParSocket(char* strAEnvoyer, SOCKET socketPourCommuniquer)
{
	int lengthOfStr = strlen(strAEnvoyer);
	char szBuf[1024];
	char ackBuf[BUFF_SIZE];
	char lengthStr[10];
	int posInStr = 0;
	int nRet, ack;

	memset(szBuf, 0, sizeof(szBuf));

	sprintf_s(lengthStr, "%d", lengthOfStr);
	strcpy_s(szBuf, lengthStr);
	int nbPaquets = lengthOfStr / sizeof(szBuf);
	int resteDernierPaquet = lengthOfStr % sizeof(szBuf);

	//Envoie de la taille de la string
	nRet = send(socketPourCommuniquer, szBuf, sizeof(szBuf), 0);
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("send()");
		closesocket(socketPourCommuniquer);
		return -1;
	}

	// Envoi des premiers paquets
	memset(szBuf, 0, sizeof(szBuf));
	for (int noPaquet = 0; noPaquet < nbPaquets; noPaquet++)
	{
		for (int charNo = 0; charNo < sizeof(szBuf); charNo++)
			szBuf[charNo] = strAEnvoyer[posInStr++];
		do {
			ack = 0;
			nRet = send(socketPourCommuniquer, szBuf, sizeof(szBuf), 0);
			if (nRet == SOCKET_ERROR)
			{
				PRINTERROR("send()");
				closesocket(socketPourCommuniquer);
				return -1;
			}
			nRet = recv(socketPourCommuniquer, ackBuf, sizeof(ackBuf), 0);
			ack = atoi(ackBuf);
		} while (ack != 1);
	}


	//Envoi du dernier paquet de la chaine de caracteres
	memset(szBuf, 0, sizeof(szBuf));
	for (int charNo = 0; charNo < resteDernierPaquet; charNo++)
		szBuf[charNo] = strAEnvoyer[posInStr++];
	do {
		ack = 0;
		nRet = send(socketPourCommuniquer, szBuf, sizeof(szBuf), 0);
		if (nRet == SOCKET_ERROR)
		{
			PRINTERROR("send()");
			closesocket(socketPourCommuniquer);
			return -1;
		}
		nRet = recv(socketPourCommuniquer, ackBuf, sizeof(ackBuf), 0);
		ack = atoi(ackBuf);
	} while (ack != 1);

	return 0;
}

void afficheFichier(char* nomDuFichierAAfficher)
{
	FILE* fichierAAfficher;
	char readBuf[500];

	fopen_s(&fichierAAfficher, nomDuFichierAAfficher, "rb");

	while (!feof(fichierAAfficher))
	{
		memset(readBuf, 0, sizeof(readBuf));
		fread(readBuf, 1, sizeof(readBuf) - 1, fichierAAfficher);
		cout << readBuf;
	}
	cout << '\n';	

	fclose(fichierAAfficher);
}

void fermerCom(SOCKET* socketAFermerPtr)
{
	closesocket(*socketAFermerPtr);
}

void terminerCom(void)
{
	WSACleanup();
}

void stripNewLine(char* stringAModifier)
{
	int countChar = 0;

	while (stringAModifier[countChar] != '\0')
	{
		if (stringAModifier[countChar] == '\n')
			stringAModifier[countChar] = '\0';
		countChar++;
	}
}
