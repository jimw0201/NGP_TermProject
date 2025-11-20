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
	S2C_GameStart,
	S2C_GameStateUpdate,
	S2C_GameOver,
	C2S_ReportParked,
	C2S_GameReady,
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

struct ClientInfo {
	bool IsConnected;				// 현재 접속 상태
	int PlayerID;					// 0~3 할당된 플레이어

	SOCKET TCPSocket;
	HANDLE hTCPThread;				// 각 클랑이언트의 TCP 처리 스레드 핸들

	sockaddr_in UDPaddr;
	bool UDPAddrInitialized;		// UDP 주소 등록 여부

	PlayerKey LastReceivedKeys;

	PlayerData playerData;
	PlayerGameStats playerStats;

	// Rising-Edge용 직전에 키 눌렸는지 확인하는 변수
	bool Q_PrevServerState = false;
	bool E_PrevServerState = false;
};

#endif