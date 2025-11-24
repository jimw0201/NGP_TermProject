#include "environment.h"
#include "mesh.h"      
#include "game_state.h"

// 자동차 4대의 주차 도착지 배열
float FINISH_OFFSET_X[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float FINISH_OFFSET_Z[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

// 장애물 초기화
float obstacle_xz[4][5][2] = { 0.0f };

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
    // 1스테이지
    if (stage == 1)
    {
        // -------------------------------------------------------
        // 1. 주차 목표 지점 (색깔 박스) 위치 설정
        // -------------------------------------------------------

        // P1 (파랑): 아래쪽 벽, 왼쪽에서 2번째 (-1.5칸 위치)
        FINISH_OFFSET_X[0] = -1.05 * 1.5f;
        FINISH_OFFSET_Z[0] = 6.5;

        // P2 (빨강): 왼쪽 벽, 위에서 2번째 (-1.5칸 위치)
        FINISH_OFFSET_X[1] = -6.5;
        FINISH_OFFSET_Z[1] = -1.05 * 1.5f;

        // P3 (초록): 오른쪽 벽, 아래에서 2번째 (+1.5칸 위치)
        // (그림상 5번째 칸이므로 +1.5f 위치가 맞습니다)
        FINISH_OFFSET_X[2] = 6.5;
        FINISH_OFFSET_Z[2] = 1.05 * 1.5f;

        // P4 (보라): 위쪽 벽, 오른쪽에서 2번째 (+1.5칸 위치)
        FINISH_OFFSET_X[3] = 1.05 * 1.5f;
        FINISH_OFFSET_Z[3] = -6.5;


        // -------------------------------------------------------
        // 2. 주차 판정 범위 설정 (가로/세로 구분)
        // -------------------------------------------------------
        for (int i = 0; i < 4; i++) {
            // P1(0), P4(3) -> 세로형 주차장 (위아래 벽)
            if (i == 0 || i == 3)
            {
                PARKING_X_MIN[i] = -FINISH_SIZE / 2.0f + FINISH_OFFSET_X[i];
                PARKING_X_MAX[i] = FINISH_SIZE / 2.0f + FINISH_OFFSET_X[i];
                PARKING_Z_MIN[i] = -FINISH_SIZE * fheight + FINISH_OFFSET_Z[i];
                PARKING_Z_MAX[i] = FINISH_SIZE * fheight + FINISH_OFFSET_Z[i];
            }
            // P2(1), P3(2) -> 가로형 주차장 (좌우 벽)
            else
            {
                // 가로로 길어야 하므로 fheight를 X축에 곱함
                PARKING_X_MIN[i] = -FINISH_SIZE * fheight + FINISH_OFFSET_X[i];
                PARKING_X_MAX[i] = FINISH_SIZE * fheight + FINISH_OFFSET_X[i];
                PARKING_Z_MIN[i] = -FINISH_SIZE / 2.0f + FINISH_OFFSET_Z[i];
                PARKING_Z_MAX[i] = FINISH_SIZE / 2.0f + FINISH_OFFSET_Z[i];
            }
        }


        // [P1 장애물] 주차장(0번) 기준: 왼쪽 1개, 오른쪽 4개
        obstacle_xz[0][0][0] = FINISH_OFFSET_X[0] - 1.05;     // 왼쪽 1
        obstacle_xz[0][0][1] = FINISH_OFFSET_Z[0];

        obstacle_xz[0][1][0] = FINISH_OFFSET_X[0] + 1.05;     // 오른쪽 1
        obstacle_xz[0][1][1] = FINISH_OFFSET_Z[0];
        obstacle_xz[0][2][0] = FINISH_OFFSET_X[0] + 1.05 * 2; // 오른쪽 2
        obstacle_xz[0][2][1] = FINISH_OFFSET_Z[0];
        obstacle_xz[0][3][0] = FINISH_OFFSET_X[0] + 1.05 * 3; // 오른쪽 3
        obstacle_xz[0][3][1] = FINISH_OFFSET_Z[0];
        obstacle_xz[0][4][0] = FINISH_OFFSET_X[0] + 1.05 * 4; // 오른쪽 4
        obstacle_xz[0][4][1] = FINISH_OFFSET_Z[0];


        // [P2 장애물] 주차장(1번) 기준: 위쪽 1개, 아래쪽 4개
        // ★중요 수정: 인덱스를 [1]로 변경하고, X좌표는 고정, Z좌표를 변경
        obstacle_xz[1][0][0] = FINISH_OFFSET_X[1];
        obstacle_xz[1][0][1] = FINISH_OFFSET_Z[1] - 1.55;     // 위쪽 1

        obstacle_xz[1][1][0] = FINISH_OFFSET_X[1];
        obstacle_xz[1][1][1] = FINISH_OFFSET_Z[1] + 1.55;     // 아래쪽 1
        obstacle_xz[1][2][0] = FINISH_OFFSET_X[1];
        obstacle_xz[1][2][1] = FINISH_OFFSET_Z[1] + 1.55 * 2; // 아래쪽 2
        obstacle_xz[1][3][0] = FINISH_OFFSET_X[1];
        obstacle_xz[1][3][1] = FINISH_OFFSET_Z[1] + 1.55 * 3; // 아래쪽 3
        obstacle_xz[1][4][0] = FINISH_OFFSET_X[1];
        obstacle_xz[1][4][1] = FINISH_OFFSET_Z[1] + 1.55 * 4; // 아래쪽 4


        // [P3 장애물] 주차장(2번) 기준: 위쪽 4개, 아래쪽 1개
        // ★중요 수정: 인덱스를 [2]로 변경
        obstacle_xz[2][0][0] = FINISH_OFFSET_X[2];
        obstacle_xz[2][0][1] = FINISH_OFFSET_Z[2] - 1.55 * 4; // 위쪽 4 (멀리)

        obstacle_xz[2][1][0] = FINISH_OFFSET_X[2];
        obstacle_xz[2][1][1] = FINISH_OFFSET_Z[2] - 1.55 * 3; // 위쪽 3

        obstacle_xz[2][2][0] = FINISH_OFFSET_X[2];
        obstacle_xz[2][2][1] = FINISH_OFFSET_Z[2] - 1.55 * 2; // 위쪽 2

        obstacle_xz[2][3][0] = FINISH_OFFSET_X[2];
        obstacle_xz[2][3][1] = FINISH_OFFSET_Z[2] - 1.55;     // 위쪽 1

        obstacle_xz[2][4][0] = FINISH_OFFSET_X[2];
        obstacle_xz[2][4][1] = FINISH_OFFSET_Z[2] + 1.55;     // 아래쪽 1


        // [P4 장애물] 주차장(3번) 기준: 왼쪽 4개, 오른쪽 1개
        // ★중요 수정: 인덱스를 [3]으로 변경
        obstacle_xz[3][0][0] = FINISH_OFFSET_X[3] - 1.05 * 4; // 왼쪽 4 (멀리)
        obstacle_xz[3][0][1] = FINISH_OFFSET_Z[3];

        obstacle_xz[3][1][0] = FINISH_OFFSET_X[3] - 1.05 * 3; // 왼쪽 3
        obstacle_xz[3][1][1] = FINISH_OFFSET_Z[3];

        obstacle_xz[3][2][0] = FINISH_OFFSET_X[3] - 1.05 * 2; // 왼쪽 2
        obstacle_xz[3][2][1] = FINISH_OFFSET_Z[3];

        obstacle_xz[3][3][0] = FINISH_OFFSET_X[3] - 1.05;     // 왼쪽 1
        obstacle_xz[3][3][1] = FINISH_OFFSET_Z[3];

        obstacle_xz[3][4][0] = FINISH_OFFSET_X[3] + 1.05;     // 오른쪽 1
        obstacle_xz[3][4][1] = FINISH_OFFSET_Z[3];
    }
    // 2스테이지
    else if (stage == 2)
    {
        float gap = 1.05f;   // 주차 칸 간격
        float dist = 4.5f;   // 벽까지의 거리

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

        // FINISH_OFFSET_X = 3.0f;
        // FINISH_OFFSET_Z = 0.0f;

        for (int i = 0; i < 4; i++) {
            PARKING_X_MIN[i] = -FINISH_SIZE * fheight + FINISH_OFFSET_X[i];
            PARKING_X_MAX[i] = FINISH_SIZE * fheight + FINISH_OFFSET_X[i];
            PARKING_Z_MIN[i] = -FINISH_SIZE / 2.0f + FINISH_OFFSET_Z[i];
            PARKING_Z_MAX[i] = FINISH_SIZE / 2.0f + FINISH_OFFSET_Z[i];
        }

        // -------------------------------------------------------
        // 3. 장애물 배치 (주차장 양옆 빈칸 채우기)
        // -------------------------------------------------------
        // 각 주차장(P1~P4)을 기준으로 양옆(Left/Right)에 장애물을 둡니다.
        // 남는 장애물 슬롯은 (100, 100)으로 치워버립니다.

        // [P1 주변] (아래쪽 오른쪽) -> 왼쪽 1개, 오른쪽 1개 필요
        obstacle_xz[0][0][0] = FINISH_OFFSET_X[0] - gap; // 왼쪽 칸 (+0.5위치)
        obstacle_xz[0][0][1] = FINISH_OFFSET_Z[0];

        obstacle_xz[0][1][0] = FINISH_OFFSET_X[0] + gap; // 오른쪽 칸 (+2.5위치)
        obstacle_xz[0][1][1] = FINISH_OFFSET_Z[0];

        // 나머지 3개는 사용 안 함
        for (int k = 2; k < 5; k++) { obstacle_xz[0][k][0] = 100.0f; obstacle_xz[0][k][1] = 100.0f; }


        // [P2 주변] (위쪽 오른쪽) -> 왼쪽 1개, 오른쪽 1개 필요
        obstacle_xz[1][0][0] = FINISH_OFFSET_X[1] - gap; // 왼쪽 칸 (+0.5위치)
        obstacle_xz[1][0][1] = FINISH_OFFSET_Z[1];

        obstacle_xz[1][1][0] = FINISH_OFFSET_X[1] + gap; // 오른쪽 칸 (+2.5위치)
        obstacle_xz[1][1][1] = FINISH_OFFSET_Z[1];

        // 나머지 3개는 사용 안 함
        for (int k = 2; k < 5; k++) { obstacle_xz[1][k][0] = 100.0f; obstacle_xz[1][k][1] = 100.0f; }


        // [P3 주변] (아래쪽 왼쪽) -> 왼쪽 1개, 오른쪽 1개 필요
        obstacle_xz[2][0][0] = FINISH_OFFSET_X[2] - gap; // 왼쪽 칸 (-2.5위치)
        obstacle_xz[2][0][1] = FINISH_OFFSET_Z[2];

        obstacle_xz[2][1][0] = FINISH_OFFSET_X[2] + gap; // 오른쪽 칸 (-0.5위치)
        obstacle_xz[2][1][1] = FINISH_OFFSET_Z[2];

        // 나머지 3개는 사용 안 함
        for (int k = 2; k < 5; k++) { obstacle_xz[2][k][0] = 100.0f; obstacle_xz[2][k][1] = 100.0f; }


        // [P4 주변] (위쪽 왼쪽) -> 왼쪽 1개, 오른쪽 1개 필요
        obstacle_xz[3][0][0] = FINISH_OFFSET_X[3] - gap; // 왼쪽 칸 (-2.5위치)
        obstacle_xz[3][0][1] = FINISH_OFFSET_Z[3];

        obstacle_xz[3][1][0] = FINISH_OFFSET_X[3] + gap; // 오른쪽 칸 (-0.5위치)
        obstacle_xz[3][1][1] = FINISH_OFFSET_Z[3];

        // 나머지 3개는 사용 안 함
        for (int k = 2; k < 5; k++) { obstacle_xz[3][k][0] = 100.0f; obstacle_xz[3][k][1] = 100.0f; }
    }

    // 3스테이지
    else if (stage == 3)
    {
        float dist_x = 6.5f; // 좌우 벽까지 거리
        float dist_z = 6.5f; // 상하 벽까지 거리

        // 1. 주차 공간 배치 (네 귀퉁이)
        // P1 (왼쪽 위) - 그림상 보라색
        FINISH_OFFSET_X[0] = -dist_x;
        FINISH_OFFSET_Z[0] = -dist_z;

        // P2 (왼쪽 아래) - 그림상 초록색
        FINISH_OFFSET_X[1] = -dist_x;
        FINISH_OFFSET_Z[1] = dist_z;

        // P3 (오른쪽 위) - 그림상 빨간색
        FINISH_OFFSET_X[2] = dist_x;
        FINISH_OFFSET_Z[2] = -dist_z;

        // P4 (오른쪽 아래) - 그림상 파란색
        FINISH_OFFSET_X[3] = dist_x;
        FINISH_OFFSET_Z[3] = dist_z;

        // 주차 판정 범위 (가로형 주차장으로 통일하거나 그림에 맞춰 설정)
        // 그림을 보니 모두 "가로"로 누워있는 것 같습니다.
        for (int i = 0; i < 4; i++) {
            // 가로형 공식 적용
            PARKING_X_MIN[i] = -FINISH_SIZE * fheight + FINISH_OFFSET_X[i];
            PARKING_X_MAX[i] = FINISH_SIZE * fheight + FINISH_OFFSET_X[i];
            PARKING_Z_MIN[i] = -FINISH_SIZE / 2.0f + FINISH_OFFSET_Z[i];
            PARKING_Z_MAX[i] = FINISH_SIZE / 2.0f + FINISH_OFFSET_Z[i];
        }

        // 2. 장애물(거대 벽) 위치 설정
        // 배열의 앞쪽 2개만 사용합니다.

        // [벽 1] 왼쪽 중앙에 위치
        obstacle_xz[0][0][0] = -dist_x;  // X좌표 (왼쪽 벽)
        obstacle_xz[0][0][1] = 0.0f;     // Z좌표 (중앙)

        // [벽 2] 오른쪽 중앙에 위치
        obstacle_xz[0][1][0] = dist_x;   // X좌표 (오른쪽 벽)
        obstacle_xz[0][1][1] = 0.0f;     // Z좌표 (중앙)

        // 나머지 모든 장애물은 (100, 100)으로 치워버리기
        // (0,0,2)부터 끝까지 반복문으로 처리
        obstacle_xz[0][2][0] = 100.0f; obstacle_xz[0][2][1] = 100.0f;
        obstacle_xz[0][3][0] = 100.0f; obstacle_xz[0][3][1] = 100.0f;
        obstacle_xz[0][4][0] = 100.0f; obstacle_xz[0][4][1] = 100.0f;

        // 나머지 주차 구역(1,2,3)에 할당된 장애물들도 싹 다 치우기
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
    // [parkingIdx][obsIdx] 두 단계를 거쳐야 좌표가 나옴
    glm::vec3 position(obstacle_xz[parkingIdx][obsIdx][0], fy, obstacle_xz[parkingIdx][obsIdx][1]);
    T = glm::translate(T, position);
    return T;
}

// 인자 1개(주차장 번호)
glm::mat4 Environment_GetFinishRectMatrix(int index)
{
    glm::mat4 T = glm::mat4(1.0f);
    // FINISH_OFFSET_X[index] 로 특정 주차장을 콕 집어야 함
    T = glm::translate(T, glm::vec3(FINISH_OFFSET_X[index], fy, FINISH_OFFSET_Z[index]));
    return T;
}

void Environment_UpdateParkingStatus(const std::vector<std::pair<float, float>>& carCorners)
{
    bool newIsParked = false;

    // 차의 네 모퉁이가 주차장 안에 들어왔는지 검사
    int checkCount = 0;

    // [수정] 어떤 주차장에라도 들어갔는지 확인하기 위해 반복문 필요
    // 하지만 보통 플레이어 차는 하나니까, 가장 가까운 주차장 하나만 검사하거나
    // 혹은 4개 주차장 중 하나라도 들어가면 성공으로 칠 수 있습니다.

    // 일단 간단하게 "4개 주차장 중 어디든 들어가면 OK" 로직으로 수정:
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