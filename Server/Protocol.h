#pragma once

#ifndef PROTOCOL_H_
#define PROTOCOL_H_
#include <stdint.h>

#define MAX_PLAYERS 4

enum GearState { PARK, REVERSE, NEUTRAL, DRIVE };

// 클라가 보내는 방향키 구조체
struct PlayerKey {
	bool W_Pressed;
	bool SPACE_Pressed;
	bool Q_Pressed;
	bool E_Pressed;
	bool P_Pressed;

	float handle_rotate_z;
};

// 서버에서 클라로 보내는 위치, 바퀴, 기어 상태, 차 속도
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

// 충돌 횟수, 주차하는데 걸린 시간, 주차 여부, 주차 구역에 들어갔는지 여부
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
	S2C_PlayerIdResponse,
};

// 클라 방향키 전송 패킷
struct C2S_PlayerUpdatePacket {
	PacketType type = C2S_PlayerUpdate;
	PlayerKey myData;
};

//서버 -> 클라 게임 시작 메시지 전송
struct  S2C_GameStartPacket {
	PacketType type = S2C_GameStart;
};

// 서버-> 클라 전송 패킷,  게임 상태
struct S2C_GameStateUpdatePacket {
	PacketType type = S2C_GameStateUpdate;
	int srvElapsedSec;
	PlayerData playerData[MAX_PLAYERS];
	PlayerGameStats PlayerStats[MAX_PLAYERS];
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

// 서버 -> 클라 PlayerId전송해주는 패킷
struct S2C_PlayerIdResponsePacket {
	PacketType type = S2C_PlayerIdResponse;
	int PlayerID;
};

#endif