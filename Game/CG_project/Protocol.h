#pragma once

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>

// 동시에 참가 가능한 최대 플레이어 수
#define MAX_PLAYERS 4

// 기어 상태
enum GearState { PARK, REVERSE, NEUTRAL, DRIVE };

// =============================
//  키 입력 상태 (클라이언트 → 서버)
// =============================
struct PlayerKey {
    bool W_Pressed;
    bool SPACE_Pressed;
    bool Q_Pressed;
    bool E_Pressed;

    // 현재 핸들 회전 값 (z축 회전)
    float handle_rotate_z;
};

// =============================
//  플레이어 차량 상태 (서버 → 클라이언트)
// =============================
struct PlayerData {
    int playerID;

    // 차량 위치
    float car_dx;
    float car_dy;
    float car_dz;

    // 차체 회전 (Y축)
    float car_rotateY;

    // 앞바퀴 회전 / 바퀴 굴림 각도
    float front_wheels_rotateY;
    float wheel_rect_rotateX;

    // 현재 기어 / 속도
    GearState currentGear;
    float     car_speed;
};

// =============================
//  플레이어 게임 통계 (충돌/주차 정보 등)
// =============================
struct PlayerGameStats {
    int   PlayerID;       // 플레이어 ID
    int   CollisionCount; // 충돌 횟수
    float ParkingSec;     // 주차 완료까지 걸린 시간(초)
    bool  IsParked;       // 주차 완료 여부
    bool  IsEnterParking; // 주차 구역 안에 들어와 있는지 여부
};

// =============================
//  패킷 타입 구분용 enum
// =============================
enum PacketType : uint8_t {
    C2S_PlayerUpdate = 0, // 클라 → 서버 : 입력 전송
    S2C_GameStart,        // 서버 → 클라 : 게임 시작 알림
    S2C_GameStateUpdate,  // 서버 → 클라 : 매 틱 상태 업데이트
    S2C_GameOver,         // 서버 → 클라 : 게임 종료/결과 (구조체는 다른 데 있을 수도 있음)
    C2S_ReportParked,     // 클라 → 서버 : 주차 완료 보고
    C2S_GameReady,        // 클라 → 서버 : 준비 완료
    S2C_PlayerIdResponse, // 서버 → 클라 : 플레이어 ID 할당
};

// =============================
//  개별 패킷 구조체
// =============================

// 클라이언트 → 서버 : 입력 전송 패킷
struct C2S_PlayerUpdatePacket {
    PacketType type = C2S_PlayerUpdate;
    PlayerKey  myData;
};

// 서버 → 클라이언트 : 게임 시작 알림 패킷
struct S2C_GameStartPacket {
    PacketType type = S2C_GameStart;
};

// 서버 → 클라이언트 : 매 틱마다 전체 게임 상태 전송
struct S2C_GameStateUpdatePacket {
    PacketType      type = S2C_GameStateUpdate;
    int             srvElapsedSec;                        // 서버 기준 경과 시간(초)
    PlayerData      playerData[MAX_PLAYERS];              // 각 플레이어 차량 상태
    PlayerGameStats PlayerStats[MAX_PLAYERS];             // 각 플레이어 게임 통계
};

// 서버 → 클라이언트 : 접속 시 플레이어 ID 알려주는 패킷
struct S2C_PlayerIdResponsePacket {
    PacketType type = S2C_PlayerIdResponse;
    int        PlayerID; // 0 ~ 3
};

#endif // PROTOCOL_H_
