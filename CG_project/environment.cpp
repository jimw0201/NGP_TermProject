#include "environment.h"
#include "mesh.h"      
#include "game_state.h"

glm::vec3 obstacle_scale[4][5];

// 자동차 4대의 주차 도착지 배열
float FINISH_OFFSET_X[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float FINISH_OFFSET_Z[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

// 장애물 초기화
float obstacle_xz[4][5][2] = { 0.0f };

// 장애물 각각의 Y축 회전 각도
float obstacle_ry[4][5] = { 0.0f };

// 주차 공간 선언
float PARKING_X_MIN[4];
float PARKING_X_MAX[4];
float PARKING_Z_MIN[4];
float PARKING_Z_MAX[4];


void Environment_Init()
{
	Environment_SetupStage(1);
}

void Environment_SetupStage(int stage)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            obstacle_scale[i][j] = glm::vec3(1.0f, 1.0f, 1.0f);
        }
    }

    // 1스테이지
    if (stage == 1)
    {
        FINISH_OFFSET_X[0] = -1.05 * 1.5f;
        FINISH_OFFSET_Z[0] = 4.5;

        FINISH_OFFSET_X[1] = -4.5;
        FINISH_OFFSET_Z[1] = -1.05 * 1.5f;

        FINISH_OFFSET_X[2] = 4.5;
        FINISH_OFFSET_Z[2] = 1.05 * 1.5f;

        FINISH_OFFSET_X[3] = 1.05 * 1.5f;
        FINISH_OFFSET_Z[3] = -4.5;

        for (int i = 0; i < 4; i++) {
            if (i == 0 || i == 3)
            {
                PARKING_X_MIN[i] = -FINISH_SIZE / 2.0f + FINISH_OFFSET_X[i];
                PARKING_X_MAX[i] = FINISH_SIZE / 2.0f + FINISH_OFFSET_X[i];
                PARKING_Z_MIN[i] = -FINISH_SIZE * fheight + FINISH_OFFSET_Z[i];
                PARKING_Z_MAX[i] = FINISH_SIZE * fheight + FINISH_OFFSET_Z[i];
            }
            else
            {
                PARKING_X_MIN[i] = -FINISH_SIZE * fheight + FINISH_OFFSET_X[i];
                PARKING_X_MAX[i] = FINISH_SIZE * fheight + FINISH_OFFSET_X[i];
                PARKING_Z_MIN[i] = -FINISH_SIZE / 2.0f + FINISH_OFFSET_Z[i];
                PARKING_Z_MAX[i] = FINISH_SIZE / 2.0f + FINISH_OFFSET_Z[i];
            }
        }

        // P1 장애물
        obstacle_xz[0][0][0] = FINISH_OFFSET_X[0] - 1.05; 
        obstacle_xz[0][0][1] = FINISH_OFFSET_Z[0];

        obstacle_xz[0][1][0] = FINISH_OFFSET_X[0] + 1.05;
        obstacle_xz[0][1][1] = FINISH_OFFSET_Z[0];
        obstacle_xz[0][2][0] = FINISH_OFFSET_X[0] + 1.05 * 2;
        obstacle_xz[0][2][1] = FINISH_OFFSET_Z[0];
        obstacle_xz[0][3][0] = FINISH_OFFSET_X[0] + 1.05 * 3;
        obstacle_xz[0][3][1] = FINISH_OFFSET_Z[0];
        obstacle_xz[0][4][0] = FINISH_OFFSET_X[0] + 1.05 * 4; 
        obstacle_xz[0][4][1] = FINISH_OFFSET_Z[0];


        // P2 장애물
        obstacle_xz[1][0][0] = FINISH_OFFSET_X[1];
        obstacle_xz[1][0][1] = FINISH_OFFSET_Z[1] - 1.05;
        obstacle_ry[1][0] = 90.0f;
        obstacle_xz[1][1][0] = FINISH_OFFSET_X[1];
        obstacle_xz[1][1][1] = FINISH_OFFSET_Z[1] + 1.05;
        obstacle_ry[1][1] = 90.0f;
        obstacle_xz[1][2][0] = FINISH_OFFSET_X[1];
        obstacle_xz[1][2][1] = FINISH_OFFSET_Z[1] + 1.05 * 2; 
        obstacle_ry[1][2] = 90.0f;
        obstacle_xz[1][3][0] = FINISH_OFFSET_X[1];
        obstacle_xz[1][3][1] = FINISH_OFFSET_Z[1] + 1.05 * 3;
        obstacle_ry[1][3] = 90.0f;
        obstacle_xz[1][4][0] = FINISH_OFFSET_X[1];
        obstacle_xz[1][4][1] = FINISH_OFFSET_Z[1] + 1.05 * 4;
        obstacle_ry[1][4] = 90.0f;


        // P3 장애물
        obstacle_xz[2][0][0] = FINISH_OFFSET_X[2];
        obstacle_xz[2][0][1] = FINISH_OFFSET_Z[2] - 1.05 * 4; 
        obstacle_ry[2][0] = 90.0f;

        obstacle_xz[2][1][0] = FINISH_OFFSET_X[2];
        obstacle_xz[2][1][1] = FINISH_OFFSET_Z[2] - 1.05 * 3;
        obstacle_ry[2][1] = 90.0f;

        obstacle_xz[2][2][0] = FINISH_OFFSET_X[2];
        obstacle_xz[2][2][1] = FINISH_OFFSET_Z[2] - 1.05 * 2; 
        obstacle_ry[2][2] = 90.0f;

        obstacle_xz[2][3][0] = FINISH_OFFSET_X[2];
        obstacle_xz[2][3][1] = FINISH_OFFSET_Z[2] - 1.05;
        obstacle_ry[2][3] = 90.0f;

        obstacle_xz[2][4][0] = FINISH_OFFSET_X[2];
        obstacle_xz[2][4][1] = FINISH_OFFSET_Z[2] + 1.05;
        obstacle_ry[2][4] = 90.0f;

        // P4 장애물
        obstacle_xz[3][0][0] = FINISH_OFFSET_X[3] - 1.05 * 4; 
        obstacle_xz[3][0][1] = FINISH_OFFSET_Z[3];

        obstacle_xz[3][1][0] = FINISH_OFFSET_X[3] - 1.05 * 3; 
        obstacle_xz[3][1][1] = FINISH_OFFSET_Z[3];

        obstacle_xz[3][2][0] = FINISH_OFFSET_X[3] - 1.05 * 2; 
        obstacle_xz[3][2][1] = FINISH_OFFSET_Z[3];

        obstacle_xz[3][3][0] = FINISH_OFFSET_X[3] - 1.05;
        obstacle_xz[3][3][1] = FINISH_OFFSET_Z[3];

        obstacle_xz[3][4][0] = FINISH_OFFSET_X[3] + 1.05;
        obstacle_xz[3][4][1] = FINISH_OFFSET_Z[3];
    }
    // 2스테이지
    else if (stage == 2)
    {
        // 주차 공간
        // 차 1번 주차장
        FINISH_OFFSET_X[0] = 1.55 * 1.5f;
        FINISH_OFFSET_Z[0] = 4.5f;

        // 차 2번 주차장
        FINISH_OFFSET_X[1] = 1.55 * 1.5f;
        FINISH_OFFSET_Z[1] = -4.5f;

        // 차 3번 주차장
        FINISH_OFFSET_X[2] = -1.55 * 1.5f;
        FINISH_OFFSET_Z[2] = 4.5f;

        // 차 4번 주차장
        FINISH_OFFSET_X[3] = -1.55 * 1.5f;
        FINISH_OFFSET_Z[3] = -4.5f;


        for (int i = 0; i < 4; i++) {
            PARKING_X_MIN[i] = -FINISH_SIZE * fheight + FINISH_OFFSET_X[i];
            PARKING_X_MAX[i] = FINISH_SIZE * fheight + FINISH_OFFSET_X[i];
            PARKING_Z_MIN[i] = -FINISH_SIZE / 2.0f + FINISH_OFFSET_Z[i];
            PARKING_Z_MAX[i] = FINISH_SIZE / 2.0f + FINISH_OFFSET_Z[i];
        }

        obstacle_xz[0][0][0] = FINISH_OFFSET_X[0] - 1.55f;
        obstacle_xz[0][0][1] = FINISH_OFFSET_Z[0];
        obstacle_ry[0][0] = 90.0f;

        obstacle_xz[0][1][0] = FINISH_OFFSET_X[0] + 1.55f;
        obstacle_xz[0][1][1] = FINISH_OFFSET_Z[0];
        obstacle_ry[0][1] = 90.0f;

        for (int k = 2; k < 5; k++) { obstacle_xz[0][k][0] = 100.0f; obstacle_xz[0][k][1] = 100.0f; }

        obstacle_xz[1][0][0] = FINISH_OFFSET_X[1] - 1.55f;
        obstacle_xz[1][0][1] = FINISH_OFFSET_Z[1];
        obstacle_ry[1][0] = 90.0f;

        obstacle_xz[1][1][0] = FINISH_OFFSET_X[1] + 1.55f;
        obstacle_xz[1][1][1] = FINISH_OFFSET_Z[1];
        obstacle_ry[1][1] = 90.0f;

        for (int k = 2; k < 5; k++) { obstacle_xz[1][k][0] = 100.0f; obstacle_xz[1][k][1] = 100.0f; }

        obstacle_xz[2][0][0] = FINISH_OFFSET_X[2] - 1.55f;
        obstacle_xz[2][0][1] = FINISH_OFFSET_Z[2];
        obstacle_ry[2][0] = 90.0f;

        obstacle_xz[2][1][0] = FINISH_OFFSET_X[2] + 1.55f; 
        obstacle_xz[2][1][1] = FINISH_OFFSET_Z[2];
        obstacle_ry[2][1] = 90.0f;

        for (int k = 2; k < 5; k++) { obstacle_xz[2][k][0] = 100.0f; obstacle_xz[2][k][1] = 100.0f; }

        obstacle_xz[3][0][0] = FINISH_OFFSET_X[3] - 1.55f;
        obstacle_xz[3][0][1] = FINISH_OFFSET_Z[3];
        obstacle_ry[3][0] = 90.0f;

        obstacle_xz[3][1][0] = FINISH_OFFSET_X[3] + 1.55f;
        obstacle_xz[3][1][1] = FINISH_OFFSET_Z[3];
		obstacle_ry[3][1] = 90.0f;

        for (int k = 2; k < 5; k++) { obstacle_xz[3][k][0] = 100.0f; obstacle_xz[3][k][1] = 100.0f; }
    }

    // 3스테이지
    else if (stage == 3)
    {
        // 1. 주차 공간 배치
        // P1
        FINISH_OFFSET_X[0] = -4.5f;
        FINISH_OFFSET_Z[0] = -4.5f;

        // P2
        FINISH_OFFSET_X[1] = -4.5f;
        FINISH_OFFSET_Z[1] = 4.5f;

        // P3
        FINISH_OFFSET_X[2] = 4.5f;
        FINISH_OFFSET_Z[2] = -4.5f;

        // P4
        FINISH_OFFSET_X[3] = 4.5f;
        FINISH_OFFSET_Z[3] = 4.5f;

        // 주차 판정 범위
        for (int i = 0; i < 4; i++) {
            // 가로형 공식 적용
            PARKING_X_MIN[i] = -FINISH_SIZE / 2.0f + FINISH_OFFSET_X[i];
            PARKING_X_MAX[i] = FINISH_SIZE / 2.0f + FINISH_OFFSET_X[i];
            PARKING_Z_MIN[i] = -FINISH_SIZE * fheight + FINISH_OFFSET_Z[i];
            PARKING_Z_MAX[i] = FINISH_SIZE * fheight + FINISH_OFFSET_Z[i];
        }

        // 2. 장애물(긴 벽) 위치 설정
        obstacle_xz[0][0][0] = 0.0f;
        obstacle_xz[0][0][1] = -4.5f;
        obstacle_scale[0][0] = glm::vec3(2.0f, 2.0f, 6.0f);

        obstacle_xz[0][1][0] = 0.0f;
        obstacle_xz[0][1][1] = 4.5f;
        obstacle_scale[0][1] = glm::vec3(2.0f, 2.0f, 6.0f);

        // 나머지 모든 장애물은 (100, 100)으로 치워버리기
        obstacle_xz[0][2][0] = 100.0f; obstacle_xz[0][2][1] = 100.0f;
        obstacle_xz[0][3][0] = 100.0f; obstacle_xz[0][3][1] = 100.0f;
        obstacle_xz[0][4][0] = 100.0f; obstacle_xz[0][4][1] = 100.0f;

        // 나머지 장애물 치우기
        for (int i = 1; i < 4; i++) {
            for (int j = 0; j < 5; j++) {
                obstacle_xz[i][j][0] = 100.0f;
                obstacle_xz[i][j][1] = 100.0f;
            }
        }
    }
}


// 인자 2개(주차장번호, 장애물번호) 몇 번 주차장의 몇 번째 장애물인지 정보
glm::mat4 Environment_GetObstacleMatrix(int parkingIdx, int obsIdx)
{
    glm::mat4 T = glm::mat4(1.0f);

    // 위치 이동
    glm::vec3 position(obstacle_xz[parkingIdx][obsIdx][0], fy, obstacle_xz[parkingIdx][obsIdx][1]);
    T = glm::translate(T, position);

    // 설정된 각도만큼 회전
    float angle = obstacle_ry[parkingIdx][obsIdx];
    if (angle != 0.0f) 
    {
        T = glm::rotate(T, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    T = glm::scale(T, obstacle_scale[parkingIdx][obsIdx]);

    return T;
}

// 인자 1개(주차장 번호)
glm::mat4 Environment_GetFinishRectMatrix(int index)
{
    glm::mat4 T = glm::mat4(1.0f);

    // 1. 위치 이동
    T = glm::translate(T, glm::vec3(FINISH_OFFSET_X[index], fy, FINISH_OFFSET_Z[index]));

    // 2. 가로/세로 길이를 비교하여 회전 결정
    float widthX = PARKING_X_MAX[index] - PARKING_X_MIN[index];
    float lengthZ = PARKING_Z_MAX[index] - PARKING_Z_MIN[index];

    // 현재 스테이지 정보 가져오기
    int currentStage = GameState_GetCurrentStage();

    // 가로가 세로보다 길면 90도 회전(3스테이지 장애물 제외)
    if (widthX > lengthZ)
    {
        T = glm::rotate(T, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    return T;
}

void Environment_UpdateParkingStatus(const std::vector<std::pair<float, float>>& carCorners)
{
    bool newIsParked = false;
    int checkCount = 0;

    for (const auto& corner : carCorners)
    {
        float cornerX = corner.first;
        float cornerZ = corner.second;

        // 4개의 주차 구역 중 하나라도 포함되는지 확인
        bool insideAny = false;
        for (int i = 0; i < 4; i++)
        {
            if (PARKING_X_MIN[i] <= cornerX && cornerX <= PARKING_X_MAX[i] &&
                PARKING_Z_MIN[i] <= cornerZ && cornerZ <= PARKING_Z_MAX[i])
            {
                insideAny = true;
                break; // 하나라도 들어가면 통과
            }
        }

        if (insideAny) checkCount++;
    }

    if (checkCount >= 4)
    {
        newIsParked = true;
    }

    if (newIsParked != GameState_IsParked())
    {
        GameState_SetParked(newIsParked);
    }
}