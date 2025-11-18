#pragma once

#ifndef PROTOCOL_H_
#define PROTOCOL_H_
#include <stdint.h>

#define MAX_PLAYERS 4

enum GearState { PARK, REVERSE, NEUTRAL, DRIVE };

struct PlayerKey {
	bool W_Pressed;
	bool SPACE_Pressed;
	bool Q_Pressed;
	bool E_Pressed;

	float handle_rotate_z;
};

struct PlayerData {
	int playerID;

	float car_dx;
	float car_dy;
	float car_dz;
	float car_rotateY;

	float front_wheels_rotateY;
	float wheel_rect_rotateX;

	GearState currentGear;
	float car_speed;
};

struct PlayerGameStats {
	int PlayerID;
	int CollisionCount;
	float ParkingSec;
	bool IsParked;
	bool IsEnterParking;
};

enum PacketType : uint8_t {
	C2S_PlayerUpdate = 0,
	S2C_PlayerIdResponse = 1
	S2C_GameStart =2 ,
	S2C_GameStateUpdate = 3,
	S2C_GameOver = 4,
	C2S_ReportParked = 5,
	C2S_GameReady = 6,
};

struct C2S_PlayerUpdatePacket {
	PacketType type = C2S_PlayerUpdate;
	int playerID;
	PlayerKey myData;
};

struct S2C_GameStateUpdatePacket {
	PacketType type = S2C_GameStateUpdate;
	int srvElapsedSec;
	PlayerData PlayerStates[MAX_PLAYERS];
	PlayerGameStats PlayerStats[MAX_PLAYERS];
};

struct C2S_ReportParkedPacket {
	PacketType type = C2S_ReportParked;
	int PlayerID;
};

struct S2C_PlayerIdResponse {
	PacketType type = S2C_PlayerIdResponse;
	int PlayerID;
};

struct PlayerFinalScore {
	int playerID;
	float parkingTimeSeconds;
	int collisionCount;

	int timeScore;
	int collisionPenalty;
	int finalScore;
};

enum GameWinner :unit8_t {
	Player_0,
	Player_1,
	Player_2,
	Player_3,
	Draw
};

struct S2C_GameOverPacket {
	PacketType type = S2C_GameOver;
	GameWinner winner[MAX_PLAYERS];
	PlayerFinalScore finalScores[MAX_PLAYERS][3];
};

// 매칭완료, 게임 시작을 알림
struct S2C_GameStartPacket {
	PacketType type = S2C_GameStart;
	int stageID;
};


// 서버에만 존재하는 헤더, 클라 X
struct ClientInfo {
	bool IsConnected;
	int PlayerID;

	SOCKET TCPSocket;
	HANDLE hTCPThread;

	sockaddr_in UDPaddr;
	bool UDPAddrInitialized;

	PlayerKey LastReceivedKeys;

	PlayerData playerData;
	PlayerGameStats playerStats;

	bool Q_PrevServerState = false;
	bool E_PrevServerState = false;
};

#endif
