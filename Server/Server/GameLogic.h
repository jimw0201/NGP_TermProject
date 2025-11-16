#pragma once
#include <winsock2.h>
#include <math.h>
#include <stdio.h>
#include "Protocol.h"

extern CRITICAL_SECTION g_cs;
extern struct ClientInfo g_clients[MAX_PLAYERS];
extern int g_connectedClients;
extern const float g_round1_StartPos[MAX_PLAYERS][3];

void Server_HandleNC(SOCKET newTCP_Socket, int PlayerID);

void Server_movement(int PlayerID);

void Server_CheckAllCollisions();

bool Server_CheckGameOver();