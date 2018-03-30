/*********************************************************************
commClientServerr.h - header pour les fonctions de communication par socket
écrit par: Pierre-Marc Laforest
Date: 2017-04-05
**********************************************************************/
#ifndef COMCLIENTSERVER_H
#define COMCLIENTSERVER_H

#include <sys/socket.h>

typedef int SOCKET;

// Client/Server fonctions
void server_initComAvecClient(short nPort, SOCKET* listenSocketPtr, SOCKET* remoteSocketPtr);
void client_initComAvecServer(char *szServer, short nPort, SOCKET* theSocketPtr);
void fermerCom(SOCKET* socketAFermerPtr);
void terminerCom(void);

//Autres fonctions
int receptionStrParSocket(char* strARetourner, SOCKET socketPourCommuniquer);
void receptionFichierParSocket(char *nomDuFichierARecevoir, SOCKET theSocket);
int envoiStrParSocket(char* strAEnvoyer, SOCKET socketPourCommuniquer);
void envoiFichierParSocket(char* nomDuFichier, SOCKET remoteSocket);
void afficheFichier(char* nomDuFichierAAfficher);
void stripNewLine(char* stringAModifier);

#endif
