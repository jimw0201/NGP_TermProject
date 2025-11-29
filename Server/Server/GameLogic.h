#pragma once
#include <winsock2.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "Protocol.h"

#define MAX_SPEED 0.01f
#define ACCELERATION 0.001f
#define DECELERATION 0.002f
#define FRICTION 0.001f
#define HANDLE_MAX_ROTATION 900.0f
#define WHEEL_MAX_ROTATION 30.0f
#define WHEEL_SPIN_MULTIPLIER 200.0f

#define CAR_SIZE 0.5f
#define WALL_THICKNESS 0.2f
#define GROUND_SIZE 6.0f
#define M_PI 3.14159265358979323846

// ----------서버 전역 자원 선언(extern)---------------
extern CRITICAL_SECTION g_cs;
extern struct ClientInfo g_clients[MAX_PLAYERS];
extern int g_connectedClients;
extern const float g_round1_StartPos[MAX_PLAYERS][3];
//-----------------------------------------------------

void Server_HandleNC(SOCKET newTCP_Socket, int PlayerID);	// 새 클라이언트 접속 시 초기화

void Server_movement(int PlayerID);							// 물리 연산(키 입력)

void Server_CheckAllCollisions();							// 충돌 및 주차 판정

bool Server_CheckGameOver();								// 게임 종료 판정