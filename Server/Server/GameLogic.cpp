#include "GameLogic.h"

#define MAX_SPEED 0.01f
#define ACCELERATION 0.001f
#define DECELERATION 0.002f
#define FRICTION 0.001f
#define HANDLE_MAX_ROTATION 900.0f
#define WHEEL_MAX_ROTATION 30.0f
#define WHEEL_SPIN_MULTIPLIER 200.0f

void Server_HandleNC(SOCKET newTCP_Socket, int PlayerID) {
    printf("[TCP] Server_HandleNC: 클라이언트 %d 접속 처리 시작\n", PlayerID);

    // 차량 기본 높이
    const float INITIAL_CAR_Y = 0.125f;

    // ID에 따라 미리 정의된 시작 위치와 각도 가져옴
    float startX = g_round1_StartPos[PlayerID][0];
    float startZ = g_round1_StartPos[PlayerID][1];
    float startAngle = g_round1_StartPos[PlayerID][2];

    EnterCriticalSection(&g_cs);
    {
        ClientInfo& client = g_clients[PlayerID];

        // 1. 접속 정보 설정
        client.IsConnected = true;
        client.PlayerID = PlayerID;
        client.TCPSocket = newTCP_Socket;
        client.UDPAddrInitialized = false;
        g_connectedClients++;

        // 2. 입력 키 버퍼 초기화
        memset(&client.LastReceivedKeys, 0, sizeof(PlayerKey));

        // 3. 차량 물리 상태 초기화
        client.playerData.playerID = PlayerID;
        client.playerData.car_dx = startX;
        client.playerData.car_dy = INITIAL_CAR_Y;
        client.playerData.car_dz = startZ;
        client.playerData.car_rotateY = startAngle;
        client.playerData.front_wheels_rotateY = 0.0f;
        client.playerData.currentGear = GearState::DRIVE;
        client.playerData.car_speed = 0.0f;

        // 4. 게임 통계 초기화
        client.playerStats.PlayerID = PlayerID;
        client.playerStats.CollisionCount = 0;
        client.playerStats.ParkingSec = 0.0f;
        client.playerStats.IsParked = false;
        client.playerStats.IsEnterParking = false;

        // 5. 기어 변속용 이전 상태 플래그 초기화
        client.Q_PrevServerState = false;
        client.E_PrevServerState = false;
    }
    LeaveCriticalSection(&g_cs);

    printf("[TCP] 클라이언트 %d: 시작 위치 (%.1f, %.1f), 각도 %.1f로 설정 완료.\n"
        , PlayerID, startX, startZ, startAngle);
}

void Server_movement(int PlayerID) {
    // 참조 변수 사용
    PlayerKey& keys = g_clients[PlayerID].LastReceivedKeys;
    PlayerData& data = g_clients[PlayerID].playerData;
    PlayerGameStats& stats = g_clients[PlayerID].playerStats;
    float& car_speed = data.car_speed;

    // 기어 변속 처리를 위한 이전 프레임 키 상태 참조
    bool& q_prev = g_clients[PlayerID].Q_PrevServerState;
    bool& e_prev = g_clients[PlayerID].E_PrevServerState;

    // 핸들 값을 실제 바퀴 각도로 변환
    data.front_wheels_rotateY = (keys.handle_rotate_z / HANDLE_MAX_ROTATION) * WHEEL_MAX_ROTATION;

    // 기어 P 혹은 중립이면 움직이지 않음
    if (data.currentGear == PARK || data.currentGear == NEUTRAL) {
        car_speed = 0.0f;
    }

    // 기어 R 상태로 W => 뒤로 가속
    if (data.currentGear == REVERSE && keys.W_Pressed) {
        car_speed -= ACCELERATION;
        if (car_speed < -MAX_SPEED) car_speed = -MAX_SPEED;
    }

    // 기어 D 상태로 W => 앞으로 가속 
    if (data.currentGear == DRIVE && keys.W_Pressed) {
        car_speed += ACCELERATION;
        if (car_speed > MAX_SPEED) car_speed = MAX_SPEED;
    }

    // 브레이크(SPACE) 처리
    if (keys.SPACE_Pressed) {
        if (car_speed > 0.0f) {
            car_speed -= DECELERATION;
            if (car_speed < 0.0f) car_speed = 0.0f;
        }
        else if (car_speed < 0.0f) {
            car_speed += DECELERATION;
            if (car_speed > 0.0f) car_speed = 0.0f;
        }
    }

    // 마찰력 처리(가만히 있으면 서서히 멈춤)
    if (!keys.W_Pressed && !keys.SPACE_Pressed) {
        if (car_speed > 0.0f) {
            car_speed -= FRICTION;
            if (car_speed < 0.0f) car_speed = 0.0f;
        }
        else if (car_speed < 0.0f) {
            car_speed += FRICTION;
            if (car_speed > 0.0f) car_speed = 0.0f;
        }
    }

    // 기어 변속 로직
    // 키가 눌린 순간만을 감지하여 기어를 한 단계씩 바꿈
    // 지속적으로 눌리고 있어도 기어가 계속 바뀌지 않게 rising_edge 감지
    bool q_rising_edge = (keys.Q_Pressed && !q_prev);
    bool e_rising_edge = (keys.E_Pressed && !e_prev);

    if (q_rising_edge) {
        if (data.currentGear > PARK) {
            data.currentGear = static_cast<GearState>(data.currentGear - 1);
        }
    }
    if (e_rising_edge) {
        if (data.currentGear < DRIVE) {
            data.currentGear = static_cast<GearState>(data.currentGear + 1);
        }
    }

    q_prev = keys.Q_Pressed;
    e_prev = keys.E_Pressed;

    // 최종 위치 및 회전 업데이트
    if (car_speed != 0.0f) {
        float radians = (float)(data.car_rotateY * (3.141592 / 180.0));
        float new_dx = data.car_dx + car_speed * sin(radians);
        float new_dz = data.car_dz + car_speed * cos(radians);

        float n = (MAX_SPEED * 2) / MAX_SPEED;
        data.car_rotateY += data.front_wheels_rotateY * n * car_speed;

        data.car_dx = new_dx;
        data.car_dz = new_dz;

        data.wheel_rect_rotateX += car_speed * WHEEL_SPIN_MULTIPLIER;
    }
}

void Server_CheckAllCollisions() {

}

bool Server_CheckGameOver() {
    EnterCriticalSection(&g_cs);

    // 아직 모든 플레이어가 좁속하지 않았으면 게임 오버 아님
    if (g_connectedClients < MAX_PLAYERS) {
        LeaveCriticalSection(&g_cs);
        return false;
    }

    bool allParked = true;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        // 접속 중인 플레이어 중 한 명이라도 주차를 안 했으면 게임 오버 아님
        if (g_clients[i].IsConnected && !g_clients[i].playerStats.IsParked) {
            allParked = false;
            break;
        }
    }

    LeaveCriticalSection(&g_cs);

    return allParked;
}