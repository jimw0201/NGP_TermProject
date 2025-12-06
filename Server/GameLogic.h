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

// 맵 정보
// 장애물 슬롯 개수
static const int PARKING_COUNT = 4;
static const int OBSTACLE_SLOT_COUNT = 5;

// 장애물 크기 정보
static const float OBSTACLE_WIDTH = CAR_SIZE * 0.7f;
static const float OBSTACLE_HEIGHT = CAR_SIZE * 1.1f;

// 장애물 하나에 대한 정보
struct Obstacle_Info
{
	float x;
	float z;
	float rotYDeg;
	float scaleX;
	float scaleZ;
};

// 현재 스테이지 기준 장애물 정보
// [주차장 인덱스 0~3] [해당 주차장의 장애물 슬롯 0~4]
extern Obstacle_Info g_obstacles[PARKING_COUNT][OBSTACLE_SLOT_COUNT];

// 주차 구역 판정 범위
struct ParkingArea
{
	float xMin, xMax;
	float zMin, zMax;
};

// [주차장 0~3]의 주차 박스
extern ParkingArea g_parkingAreas[PARKING_COUNT];

// 각 주차장의 도착 지점
struct FinishPoint
{
	float x;
	float z;
};

extern FinishPoint g_finishPoints[PARKING_COUNT];

// 맵 스테이지 데이터 초기화
// 서버 시작 시 기본 값 세팅, 라운드가 바뀔 때 마다 스테이지에 맞게 g_obstacles, g_parkingAreas, g_finishPoints 채우기
void Server_LoadStage(int stage);