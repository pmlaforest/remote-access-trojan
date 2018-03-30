/*********************************************************************
commClientServer.cpp - Fonctions de communication par socket
écrit par: Pierre-Marc Laforest
Date: 2017-04-05
**********************************************************************/

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#include "comClientServer.h"

using namespace std;

//Defining the buffer size
#define BUFF_SIZE 1024
#define PORT_NUMB 3000
#define TAILLE_DES_BLOCS  1023
#define FICHIER_CMD_ENREGISTREES "outputCommand.txt"

void server_initComAvecClient(short nPort, SOCKET* listenSocketPtr, SOCKET* remoteSocketPtr)
{
  sockaddr_in saServer;
  char szBuf[BUFF_SIZE];
  int nRet;

  // Create a TCP/IP stream socket to "listen" with
  *listenSocketPtr = socket(AF_INET, SOCK_STREAM, 0);
  if (*listenSocketPtr == -1)
    {
      std::cout << "socket() error" << std::endl;
      return;
    }

  // Fill in the address structure
  saServer.sin_family = AF_INET;
  saServer.sin_addr.s_addr = INADDR_ANY;
  saServer.sin_port = htons(nPort);

  nRet = bind(*listenSocketPtr, (sockaddr*) &saServer, sizeof(struct sockaddr));
  if (nRet == -1)
    {
      std::cout << "bind() error" << std::endl;      
      close(*listenSocketPtr);
      return;
    }

  nRet = gethostname(szBuf, sizeof(szBuf));
  if (nRet == -1)
    {
      std::cout << "gethostname() error" << std::endl;      
      close(*listenSocketPtr);
      return;
    }

  nRet = listen(*listenSocketPtr, SOMAXCONN);
  if (nRet == -1)
    {
      std::cout << "listen() error" << std::endl;      
      close(*listenSocketPtr);
      return;
    }

  *remoteSocketPtr = accept(*listenSocketPtr, NULL, NULL);
  if (*remoteSocketPtr == -1)
    {
      std::cout << "accept() error" << std::endl;      
      close(*listenSocketPtr);
      return;
    }
}

void client_initComAvecServer(char *szServer, short nPort, SOCKET* theSocketPtr)
{
  // Find the server
  struct hostent *lpHostEntry;
  sockaddr_in saServer;
  int nRet;
	
  lpHostEntry = gethostbyname(szServer);
  if (lpHostEntry == NULL)
    {
      std::cout << "gethostbyname() error" << std::endl;      
      return;
    }

  *theSocketPtr = socket(AF_INET, SOCK_STREAM, 0);
  if (*theSocketPtr == -1)
    {
      std::cout << "socket() error" << std::endl;      
      return;
    }

  // Fill in the address structure
  saServer.sin_family = AF_INET;
  memcpy(&saServer.sin_addr, lpHostEntry->h_addr_list[0], lpHostEntry->h_length);
  saServer.sin_port = htons(nPort);

  nRet = connect(*theSocketPtr, (sockaddr *)&saServer, sizeof(struct sockaddr));
  if (nRet == -1)
    {
      std::cout << "connect() error" << std::endl;      
      close(*theSocketPtr);
      return;
    }
}

void envoiFichierParSocket(char* nomDuFichier, SOCKET remoteSocket)
{
  char szBuf[BUFF_SIZE];
  char ackBuf[BUFF_SIZE];
  int nRet, ack;
  fstream fichierAEnvoyer;
  char tailleFichier[50];
  unsigned int begin, end, tailleFichierInt;

  // Envoi de la taille du fichier
  fichierAEnvoyer.open(nomDuFichier, ios::binary | ios::in);

  if (fichierAEnvoyer.is_open())
    {
      begin = fichierAEnvoyer.tellg();
      fichierAEnvoyer.seekg(0, ios::end);
      end = fichierAEnvoyer.tellg();
      fichierAEnvoyer.seekg(0, ios::beg);
      tailleFichierInt = end - begin;
    }
  else
    {
      cout << "Impossible douvrir le fichier\n";
      return;
    }

  sprintf(tailleFichier, "%d", tailleFichierInt);
  strcpy(szBuf, tailleFichier);
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
  if (nRet == -1)
    {
      std::cout << "recv() error" << std::endl;      
      close(theSocket);
      return;
    }
  unsigned int tailleDuFichier = std::stoi(szBuf);
  unsigned int nbPaquets = tailleDuFichier / sizeof(szBuf);
  unsigned int resteDernierPaquet = tailleDuFichier % sizeof(szBuf);

  // Reception des paquets
  FILE* fichierAEcrirePtr;
  fichierAEcrirePtr = fopen(nomDuFichierARecevoir, "w+b");

  unsigned int cumul = 0;
  unsigned int tailleDesBlocs = TAILLE_DES_BLOCS;
	
  for (int noPaquet = 0; noPaquet < nbPaquets; noPaquet++)
    {
      do {  
	nRet = recv(theSocket, szBuf, sizeof(szBuf), 0); 

	if (nRet == sizeof(szBuf))
	  {
	    ack = 1;
	  }
	else
	  {
	    ack = 0;
	  }

	sprintf(ackBuf, "%d", ack);			
	send(theSocket, ackBuf, sizeof(ackBuf), 0);

      }while (ack != 1);

      fwrite(szBuf, 1, nRet, fichierAEcrirePtr);
    }
  // Reception du dernier paquet
  do {

    nRet = recv(theSocket, szBuf, sizeof(szBuf), 0);

    if (nRet == sizeof(szBuf))
      {
	ack = 1;
      }
    else
      {
	ack = 0;
      }

    sprintf(ackBuf, "%d", ack);
    send(theSocket, ackBuf, sizeof(ackBuf), 0);
	  
  } while (ack != 1);

  fwrite(szBuf, 1, resteDernierPaquet, fichierAEcrirePtr);
  fclose(fichierAEcrirePtr);
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
  if (nRet == -1)
    {
      std::cout << "recv() error" << std::endl;      
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
	if (nRet == -1)
	  {
	    std::cout << "recv() error" << std::endl;      	    
	    return -1;
	  }
	else if (nRet == sizeof(szBuf))
	  {
	    ack = 1;
	  }
	else
	  {
	    ack = 0;
	  }

	sprintf(ackBuf, "%d", ack);
	send(socketPourCommuniquer, ackBuf, sizeof(ackBuf), 0);

      } while (ack != 1);

      for (int charNo = 0; charNo < sizeof(szBuf); charNo++)
	{
	  strARetourner[posInStr++] = szBuf[charNo];
	}
    }

  // Dernier paquet
  do {
    nRet = recv(socketPourCommuniquer, szBuf, sizeof(szBuf), 0);
    if (nRet == -1)
      {
	std::cout << "recv() error" << std::endl;      	    	
	return -1;
      }
    else if (nRet == sizeof(szBuf))
      {
	ack = 1;
      }
    else
      {
	ack = 0;
      }

    sprintf(ackBuf, "%d", ack);
    send(socketPourCommuniquer, ackBuf, sizeof(ackBuf), 0);
  } while (ack != 1);
	
  for (int charNo = 0; charNo < resteDernierPaquet; charNo++)
    {
      strARetourner[posInStr++] = szBuf[charNo];
    }

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

  sprintf(lengthStr, "%d", lengthOfStr);
  strcpy(szBuf, lengthStr);
  int nbPaquets = lengthOfStr / sizeof(szBuf);
  int resteDernierPaquet = lengthOfStr % sizeof(szBuf);

  //Envoie de la taille de la string
  nRet = send(socketPourCommuniquer, szBuf, sizeof(szBuf), 0);
  if (nRet == -1)
    {
      std::cout << "send() error" << std::endl;      	    	      
      close(socketPourCommuniquer);
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
	if (nRet == -1)
	  {
	    std::cout << "send() error" << std::endl;      	    	      	
	    close(socketPourCommuniquer);
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
    if (nRet == -1)
      {
	std::cout << "send() error" << std::endl;      	    	      		
	close(socketPourCommuniquer);
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

  fichierAAfficher = fopen(nomDuFichierAAfficher, "rb");

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
  close(*socketAFermerPtr);
}

void terminerCom(void)
{
  ;
  //WSACleanup();
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
