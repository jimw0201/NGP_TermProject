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

    const float INITIAL_CAR_Y = 0.125f;

    float startX = g_round1_StartPos[PlayerID][0];
    float startZ = g_round1_StartPos[PlayerID][1];
    float startAngle = g_round1_StartPos[PlayerID][2];

    EnterCriticalSection(&g_cs);
    {
        ClientInfo& client = g_clients[PlayerID];

        client.IsConnected = true;
        client.PlayerID = PlayerID;
        client.TCPSocket = newTCP_Socket;
        client.UDPAddrInitialized = false;
        g_connectedClients++;

        memset(&client.LastReceivedKeys, 0, sizeof(PlayerKey));

        client.playerData.playerID = PlayerID;
        client.playerData.car_dx = startX;
        client.playerData.car_dy = INITIAL_CAR_Y;
        client.playerData.car_dz = startZ;
        client.playerData.car_rotateY = startAngle;
        client.playerData.front_wheels_rotateY = 0.0f;
        client.playerData.currentGear = GearState::DRIVE;
        client.playerData.car_speed = 0.0f;

        client.playerStats.PlayerID = PlayerID;
        client.playerStats.CollisionCount = 0;
        client.playerStats.ParkingSec = 0.0f;
        client.playerStats.IsParked = false;
        client.playerStats.IsEnterParking = false;

        client.Q_PrevServerState = false;
        client.E_PrevServerState = false;
    }
    LeaveCriticalSection(&g_cs);

    printf("[TCP] 클라이언트 %d: 시작 위치 (%.1f, %.1f), 각도 %.1f로 설정 완료.\n"
        , PlayerID, startX, startZ, startAngle);
}

void Server_movement(int PlayerID) {
    PlayerKey& keys = g_clients[PlayerID].LastReceivedKeys;
    PlayerData& data = g_clients[PlayerID].playerData;
    PlayerGameStats& stats = g_clients[PlayerID].playerStats;
    float& car_speed = data.car_speed;

    bool& q_prev = g_clients[PlayerID].Q_PrevServerState;
    bool& e_prev = g_clients[PlayerID].E_PrevServerState;

    data.front_wheels_rotateY = (keys.handle_rotate_z / HANDLE_MAX_ROTATION) * WHEEL_MAX_ROTATION;

    if (data.currentGear == PARK || data.currentGear == NEUTRAL) {
        car_speed = 0.0f;
    }

    if (data.currentGear == REVERSE && keys.W_Pressed) {
        car_speed -= ACCELERATION;
        if (car_speed < -MAX_SPEED) car_speed = -MAX_SPEED;
    }

    if (data.currentGear == DRIVE && keys.W_Pressed) {
        car_speed += ACCELERATION;
        if (car_speed > MAX_SPEED) car_speed = MAX_SPEED;
    }

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

    if (g_connectedClients < MAX_PLAYERS) {
        LeaveCriticalSection(&g_cs);
        return false;
    }

    bool allParked = true;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (g_clients[i].IsConnected && !g_clients[i].playerStats.IsParked) {
            allParked = false;
            break;
        }
    }

    LeaveCriticalSection(&g_cs);

    return allParked;
}