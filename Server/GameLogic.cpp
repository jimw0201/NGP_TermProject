#include "GameLogic.h"

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

std::vector<std::pair<float, float>> getRotatedCarCorners(float x, float z, float angle)
{
    float halfWidth = CAR_SIZE / 2;
    float halfHeight = CAR_SIZE;

    // 차량 꼭짓점
    std::vector<std::pair<float, float>> corners = {
        {-halfWidth, -halfHeight},
        {halfWidth, -halfHeight},
        {-halfWidth, halfHeight},
        {halfWidth, halfHeight}
    };

    // 회전
    float rad_neg = (float)(-angle * (M_PI / 180.0));

    std::vector<std::pair<float, float>> rotatedCorners;
    for (const auto& corner : corners) {
        // 회전 행렬 공식
        float rotatedX = corner.first * cos(rad_neg) - corner.second * sin(rad_neg);
        float rotatedZ = corner.first * sin(rad_neg) + corner.second * cos(rad_neg);
        
        // 월드 좌표로 변환
        rotatedCorners.emplace_back(x + rotatedX, z + rotatedZ);
    }

    return rotatedCorners;
}

static bool isPointInsidePolygon(const std::vector<std::pair<float, float>>& polygon, float x, float z)
{
    int intersections = 0;
    int n = (int)polygon.size();
    
    for (int i = 0; i < n; ++i) {
        
        auto p1 = polygon[i];
        auto p2 = polygon[(i + 1) % n];
        
        // 반직선 교차 판정
        if ((p1.second > z) != (p2.second > z)) {
            float intersectionX = p1.first + (z - p1.second) * (p2.first - p1.first) / (p2.second - p1.second);
            if (intersectionX > x) intersections++;
        }
    }

    // 홀수면 내부
    return intersections % 2 == 1;
}

static bool doLinesIntersect(float x1, float z1, float x2, float z2, float x3, float z3, float x4, float z4)
{
    auto cross = [](float ax, float ay, float bx, float by) { return ax * by - ay * bx; };

    float d1 = cross(x3 - x1, z3 - z1, x4 - x1, z4 - z1);
    float d2 = cross(x3 - x2, z3 - z2, x4 - x2, z4 - z2);
    float d3 = cross(x1 - x3, z1 - z3, x2 - x3, z2 - z3);
    float d4 = cross(x1 - x4, z1 - z4, x2 - x4, z2 - z4);

    // 두 선분 교차 판정
    return (d1 * d2 < 0 && d3 * d4 < 0);
}

static bool checkCollisionWalls(const std::vector<std::pair<float, float>>& carCorners)
{
    // 맵의 경계 계산
    float wallMin = -GROUND_SIZE + WALL_THICKNESS;
    float wallMax = GROUND_SIZE - WALL_THICKNESS;

    // 꼭짓점 중 하나라도 맵 밖으로 나가면 충돌
    for (const auto& corner : carCorners) {
        if (corner.first < wallMin || corner.first > wallMax ||
            corner.second < wallMin || corner.second > wallMax) {
            return true;
        }
    }
    return false;
}

static bool checkCollisionCars (const std::vector<std::pair<float, float>>& carCornersA, 
    const std::vector<std::pair<float, float>>& carCornersB)
{
    // 1) A의 꼭짓점이 B 안에?
    for (const auto& corner : carCornersA) {
        if (isPointInsidePolygon(carCornersB, corner.first, corner.second)) return true;
    }
    
    // 2) B의 꼭짓점이 A 안에?
    for (const auto& corner : carCornersB) {
        if (isPointInsidePolygon(carCornersA, corner.first, corner.second)) return true;
    }
    
    // 3) 변끼리 교차?
    int sizeA = (int)carCornersA.size();
    int sizeB = (int)carCornersB.size();
    
    for (int i = 0; i < sizeA; ++i) {
        for (int j = 0; j < sizeB; ++j) {
            if (doLinesIntersect(
                carCornersA[i].first, carCornersA[i].second,
                carCornersA[(i + 1) % sizeA].first, carCornersA[(i + 1) % sizeA].second,
                carCornersB[j].first, carCornersB[j].second,
                carCornersB[(j + 1) % sizeB].first, carCornersB[(j + 1) % sizeB].second)) {
                
                return true;
            }
        }
    }

    return false;
}

void Server_CheckAllCollisions() {
    // 모든 접속한 플레이어에 대해 충돌 검사
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!g_clients[i].IsConnected) continue;

        PlayerData& player = g_clients[i].playerData;
        PlayerGameStats& stats = g_clients[i].playerStats;

        // 1. 현재 위치 기반으로 4개 꼭짓점 계산
        auto currentCorners = getRotatedCarCorners(player.car_dx, player.car_dz, player.car_rotateY);

        bool isColliding = false;

        // 2. 벽 충돌 검사
        if (checkCollisionWalls(currentCorners)) {
            isColliding = true;
        }

        // 장애물 충돌은 맵 데이터가 없으므로 제외함

        // 3. 다른 플레이어와의 충돌 검사
        if (!isColliding) {
            for (int j = 0; j < MAX_PLAYERS; j++) {
                // 나 자신이나 접속 안 한 플레이어는 패스
                if (i == j || !g_clients[j].IsConnected) continue;

                // 상대방 꼭짓점 계산
                auto otherCorners = getRotatedCarCorners(g_clients[j].playerData.car_dx,
                    g_clients[j].playerData.car_dz,
                    g_clients[j].playerData.car_rotateY);

                // 충돌 검사
                if (checkCollisionCars(currentCorners, otherCorners)) {
                    isColliding = true;
                    // 상대방도 멈추게 처리
                    g_clients[j].playerData.car_speed = 0.0f;
                    break;
                }
            }
        }
    }
}

bool Server_CheckGameOver() {
    EnterCriticalSection(&g_cs);

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

