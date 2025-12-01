#include "game_state.h"
#include <gl/glew.h>
#include "car.h"
#include "Protocol.h"
#include "network_client.h"
#include "input_handle.h"
#include "collision.h"
#include "environment.h"
#include "mesh.h"
#include <iostream>     
#include <gl/freeglut.h>
#include <ctime>
#include <cmath>

// --- Private 전역 변수 ---
static time_t startTime;
static time_t pauseTime;
static time_t tempTime;
static int elapsedSeconds = 0;
static bool crushed = false;
static bool invincible = false;
static GearState currentGear = DRIVE;
static bool isParked = false;
static int current_stage = 1;
static bool pause_mode = false;
static bool isClear = false;

// 각 차량의 현재 입력 상태 (멀티 대비용)
static CarInput g_carInputs[kCarCount];

static void GameState_SendInputToServer();

// --- 함수 구현 ---
void GameState_Init()
{
    startTime = time(nullptr);
    pauseTime = startTime - time(nullptr);
    elapsedSeconds = 0;
    crushed = false;
    invincible = false;
    currentGear = DRIVE;
    isParked = false;
    current_stage = 1;
    pause_mode = false;
    isClear = false;
}

void GameState_NextStage()
{
	// 1. 각도 및 입력 초기화
    for (int i = 0; i < Car_Count(); ++i)
    {
        Car_SetRotationY(i, 0.0f);
        Car_SetFrontWheelRotationY(0.0f);
        Car_SetWheelRotationX(i, 0.0f);
    }

	Input_ResetHandle();

	// 2. 기어 및 시간 초기화
	GameState_SetCurrentGear(DRIVE);
	GameState_UpdateStartTime(time(nullptr));
	GameState_UpdatePauseTime(GameState_GetStartTime() - time(nullptr));

	// 3. 상태 초기화
	GameState_SetCrushed(false);
	GameState_SetClear(false);
	GameState_SetParked(false);
	GameState_SetPaused(false);

	// 4. 스테이지별 로직
	if (current_stage == 1)
	{
		current_stage++;

		// 환경(맵) 설정
		Environment_SetupStage(2);
	}
	else if (current_stage == 2)
	{
		current_stage++;

		Environment_SetupStage(3);
	}
	else if (current_stage == 3)
	{
		std::cout << "--Clear!!!--\n";
		glutLeaveMainLoop(); // 게임 종료
	}
}

// 현재 로컬(단일) 입력 상태를 CarInput 배열에 복사
static void GameState_FillInputsFromSingleLocal()
{
    // 1) 전체 초기화
    for (int i = 0; i < Car_Count(); ++i)
    {
        g_carInputs[i].accelForward = false;
        g_carInputs[i].accelBackward = false;
        g_carInputs[i].brake = false;
        g_carInputs[i].steering = 0.0f;
        g_carInputs[i].gear = DRIVE;
    }

    // 2) 네트워크 안 붙었으면 그냥 0번을 내 차로 취급
    int myId = 0;
    if (Network_IsConnected())
    {
        int id = Network_GetMyPlayerID();
        if (id >= 0 && id < Car_Count())
            myId = id;
    }

    // 3) 내 차(myId)에만 실제 키 입력을 반영
    bool  accelF = Car_IsAcceleratingForward();
    bool  accelB = Car_IsAcceleratingBackward();
    bool  brake = Car_IsBraking();
    float steer = Car_GetFrontWheelRotationY();
    GearState gear = GameState_GetCurrentGear();

    g_carInputs[myId].accelForward = accelF;
    g_carInputs[myId].accelBackward = accelB;
    g_carInputs[myId].brake = brake;
    g_carInputs[myId].steering = steer;
    g_carInputs[myId].gear = gear;
}


static void GameState_ApplyServerState(const S2C_GameStateUpdatePacket& pkt)
{
    GameState_SetElapsedSeconds(pkt.srvElapsedSec);

    int carCount = Car_Count();
    for (int i = 0; i < carCount && i < MAX_PLAYERS; ++i)
    {
        const PlayerData& p = pkt.playerData[i];

        Car_SetPosition(i, p.car_dx, p.car_dz);
        Car_SetRotationY(i, p.car_rotateY);
        Car_SetWheelRotationX(i, p.wheel_rect_rotateX);
        Car_SetFrontWheelRotationY(i, p.front_wheels_rotateY);
    }

    int myId = Network_GetMyPlayerID();
    if (myId >= 0 && myId < MAX_PLAYERS)
    {
        const PlayerGameStats& stats = pkt.PlayerStats[myId];
        GameState_SetParked(stats.IsParked);
    }
}



void GameState_TimerLoop(int value)
{
    time_t currentTime = time(nullptr);

    // 일시정지가 아닐 때만 경과 시간 갱신
    if (!GameState_IsPaused())
    {
        GameState_SetElapsedSeconds(
            static_cast<int>(currentTime - GameState_GetPauseTime() - GameState_GetStartTime()));
    }

    // 네트워크 연결되어 있으면 서버로 내 입력 전송
    if (Network_IsConnected())
    {
        GameState_SendInputToServer();
    }

    // 현재 키 입력 상태를 CarInput 배열(g_carInputs)에 반영
    GameState_FillInputsFromSingleLocal();

    // =========================
    // 1) 싱글플레이(로컬 물리 계산)
    // =========================
    if (!Network_IsConnected())
    {
        // (1) 현재 기어 상태
        GearState gear = GameState_GetCurrentGear();

        // (2) 각 차량 속도 업데이트
        for (int i = 0; i < Car_Count(); ++i)
        {
            const CarInput& input = GameState_GetCarInput(i);
            Car_UpdateSpeed(input, i);
        }

        // (3) 0번 차량의 주차 영역 판정
        {
            auto carCorners0 = Car_GetRotatedCorners(0);
            Environment_UpdateParkingStatus(carCorners0);
        }

        // (4) 차량 이동 + 충돌 처리(벽, 장애물, 다른 차량)
        for (int i = 0; i < Car_Count(); ++i)
        {
            if (Car_GetSpeed(i) == 0.0f) continue;

            float radians = glm::radians(Car_GetRotationY(i));
            float new_dx = Car_GetDX(i) + Car_GetSpeed(i) * sin(radians);
            float new_dz = Car_GetDZ(i) + Car_GetSpeed(i) * cos(radians);

            const float n = 2.0f;

            const CarInput& input = GameState_GetCarInput(i);
            float steering = input.steering;

            float newAngle =
                Car_GetRotationY(i) + steering * n * Car_GetSpeed(i);

            // i번 차를 new_dx/new_dz/newAngle 만큼 옮겼을 때의 모서리 좌표
            auto futureCarCorners = Car_GetRotatedCorners(new_dx, new_dz, newAngle);

            bool isColliding = false;

            if (!GameState_IsInvincible())
            {
                // 1) 경기장 네 벽과의 충돌 검사
                for (int w = 0; w < 4 && !isColliding; ++w)
                {
                    float wallX = (w % 2 == 0) ? 0.0f : (w == 1 ? GROUND_SIZE : -GROUND_SIZE);
                    float wallZ = (w % 2 == 1) ? 0.0f : (w == 2 ? GROUND_SIZE : -GROUND_SIZE);
                    float wallWidth = (w % 2 == 0) ? GROUND_SIZE * 2 : WALL_THICKNESS;
                    float wallHeight = (w % 2 == 1) ? GROUND_SIZE * 2 : WALL_THICKNESS;

                    if (checkCollisionWalls(futureCarCorners, wallX, wallZ, wallWidth, wallHeight))
                    {
                        isColliding = true;
                    }
                }

                // 2) 장애물과의 충돌 검사
                if (!isColliding && checkCollisionObstacle(futureCarCorners))
                {
                    isColliding = true;
                }

                // 3) 다른 차량과의 충돌 검사
                if (!isColliding)
                {
                    for (int j = 0; j < Car_Count(); ++j)
                    {
                        if (j == i) continue;

                        auto otherCarCorners = Car_GetRotatedCorners(j);

                        if (checkCollisionCars(futureCarCorners, otherCarCorners))
                        {
                            isColliding = true;

                            Car_SetSpeed(j, 0.0f);

                            break;
                        }
                    }
                }
            }

            if (!isColliding)
            {
                // 실제로 이동/회전 및 바퀴 회전 적용
                Car_SetRotationY(i, newAngle);
                Car_SetPosition(i, new_dx, new_dz);

                float newWheelAngle =
                    Car_GetWheelRotationX(i) + Car_GetSpeed(i) * 200.0f;
                Car_SetWheelRotationX(i, newWheelAngle);
            }
            else
            {
                // 0번 플레이어(내 차)가 부딪히면 crushed 처리
                if (i == 0)
                {
                    GameState_SetCrushed(true);
                }
                Car_SetSpeed(i, 0.0f);
            }
        }
    }

    // =========================
    // 2) 멀티플레이(서버 권위 상태 적용)
    // =========================
    if (Network_IsConnected())
    {
        S2C_GameStateUpdatePacket pkt;
        if (Network_TryGetLatestGameState(pkt))
        {
            GameState_ApplyServerState(pkt);
        }
    }

    Input_UpdateHandleReturn();

    glutPostRedisplay();
    glutTimerFunc(16, GameState_TimerLoop, 1);
}


CarInput* GameState_GetCarInputs()
{
    return g_carInputs;
}

const CarInput& GameState_GetCarInput(int idx)
{
    return g_carInputs[idx];
}


// 현재 입력 상태를 PlayerKey로 만들어 서버로 보냄
static void GameState_SendInputToServer()
{
    if (!Network_IsConnected()) return;

    PlayerKey key{};
    key.W_Pressed = Input_IsKeyWDown();
    key.SPACE_Pressed = Input_IsKeySpaceDown();
    key.Q_Pressed = Input_IsKeyQDown();
    key.E_Pressed = Input_IsKeyEDown();
    key.handle_rotate_z = Input_GetHandleRotation();

    Network_SendPlayerInput(key);
}



// --- Getters ---
bool GameState_IsPaused() { return pause_mode; }
bool GameState_IsClear() { return isClear; }
bool GameState_IsParked() { return isParked; }
GearState GameState_GetCurrentGear() { return currentGear; }
time_t GameState_GetStartTime() { return startTime; }
time_t GameState_GetPauseTime() { return pauseTime; }
time_t GameState_GetTempTime() { return tempTime; }
int GameState_GetCurrentStage() { return current_stage; }
int GameState_GetElapsedSeconds() { return elapsedSeconds; }
bool GameState_IsInvincible() { return invincible; }
bool GameState_IsCrushed() { return crushed; }


// --- Setters ---
void GameState_SetPaused(bool paused) { pause_mode = paused; }
void GameState_SetClear(bool clear) { isClear = clear; }
void GameState_SetParked(bool parked) { isParked = parked; }
void GameState_SetCurrentGear(GearState gear) { currentGear = gear; }
void GameState_UpdateStartTime(time_t time) { startTime = time; }
void GameState_UpdatePauseTime(time_t time) { pauseTime = time; }
void GameState_UpdateTempTime(time_t time) { tempTime = time; }
void GameState_SetCurrentStage(int stage) { current_stage = stage; }
void GameState_SetElapsedSeconds(int seconds) { elapsedSeconds = seconds; }
void GameState_SetInvincible(bool newStatus) { invincible = newStatus; }
void GameState_SetCrushed(bool newStatus) { crushed = newStatus; }