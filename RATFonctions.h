/*********************************************************************
RATFonctions.cpp - prototype de fonctions RATS pour le client et le serveur
écrit par: Pierre-Marc Laforest
Date: 2017-03-24
**********************************************************************/
#ifndef RATFONCTIONS_H
#define RATFONCTIONS_H

#include "comClientServer.h"

// RATServer fonctions
void RATServer_initComAvecClient(char *szServer, short nPort, SOCKET* remoteSocketServerPtr);
void RATServer_receptionCommandes(char *commandeBuf, int lengthOfCommande, char *argumentsBuf, SOCKET remoteSocketServer);
int RATServer_traiteCommandes(char *commandeAExecuter, char *argumentsCommande, SOCKET remoteSocketServer);
void RATServer_execStart(char *arguments, SOCKET remoteSocketServer);
void RATServer_execCd(char* arguments);

//RATClient fonctions
void RATClient_initComAvecServer(short nPort, SOCKET* listenSocketClientPtr, SOCKET* remoteSocketClientPtr);
void RATClient_envoiCommandes(char *dossierCourant, char *commandeAExecuter, int lengthCommande, char *argumentsCommande, SOCKET remoteSocketClient);
int RATClient_traiteCommandes(char *commandeAExecuter, char *argumentsCommande, char *dossierCourant, SOCKET remoteSocketClient);
void RATClient_execStart(char *arguments, SOCKET remoteSocketClient);

//Autres
void separeCommandeEtArguments(char *commandeBuf, int lengthOfCommandeBuf, char *argumentsBuf);
void ajoutNoCopieNomProgramme(char *nomDuProgramme, int noCopie);

#endif
