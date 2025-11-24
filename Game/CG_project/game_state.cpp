#include "game_state.h"
#include <gl/glew.h>
#include "car.h"
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
	Car_SetRotationY(0.0f);
	Car_SetFrontWheelRotationY(0.0f);
	Car_SetWheelRotationX(0.0f);

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

		// 차 위치 설정
		Car_SetPosition(2.0f, -4.0f);
	}
	else if (current_stage == 2)
	{
		current_stage++;

		Environment_SetupStage(3);

		Car_SetPosition(-4.0f, -4.0f);
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
    // car.cpp의 전역 입력 상태 읽기
    bool  accelF = Car_IsAcceleratingForward();
    bool  accelB = Car_IsAcceleratingBackward();
    bool  brake = Car_IsBraking();
    float steer = Car_GetFrontWheelRotationY();
    GearState gear = GameState_GetCurrentGear();

    // 일단은 4대 모두 같은 입력으로 채움
    for (int i = 0; i < Car_Count(); ++i)
    {
        g_carInputs[i].accelForward = accelF;
        g_carInputs[i].accelBackward = accelB;
        g_carInputs[i].brake = brake;
        g_carInputs[i].steering = steer;
        g_carInputs[i].gear = gear;
    }
}


void GameState_TimerLoop(int value)
{
    time_t currentTime = time(nullptr);

    if (!GameState_IsPaused())
    {
        GameState_SetElapsedSeconds(
            static_cast<int>(currentTime - GameState_GetPauseTime() - GameState_GetStartTime()));
    }

    // 이번 프레임의 입력 상태를 CarInput 배열에 정리
    GameState_FillInputsFromSingleLocal();

    // 1) 현재 기어 상태 가져오기
    GearState gear = GameState_GetCurrentGear();

    // 2) 모든 차량 속도 업데이트 (입력은 공유)
    for (int i = 0; i < Car_Count(); ++i)
    {
        Car_UpdateSpeed(gear, i);
    }

    // 3) 주차 상태는 0번 차 기준으로만 체크 (카메라도 0번 기준이니까)
    {
        auto carCorners0 = Car_GetRotatedCorners(0);
        Environment_UpdateParkingStatus(carCorners0);
    }

    // 4) 각 차량별 이동 + 충돌 처리
    for (int i = 0; i < Car_Count(); ++i)
    {
        if (Car_GetSpeed(i) == 0.0f) continue;

        float radians = glm::radians(Car_GetRotationY(i));
        float new_dx = Car_GetDX(i) + Car_GetSpeed(i) * sin(radians);
        float new_dz = Car_GetDZ(i) + Car_GetSpeed(i) * cos(radians);

        const float n = 2.0f;
        float newAngle =
            Car_GetRotationY(i) + Car_GetFrontWheelRotationY() * n * Car_GetSpeed(i);

        // i번 차의 "미래 위치" 꼭짓점
        auto futureCarCorners = Car_GetRotatedCorners(new_dx, new_dz, newAngle);

        bool isColliding = false;

        if (!GameState_IsInvincible())
        {
            // 1) 벽과의 충돌 검사
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

            // 3) 다른 차들과의 충돌 검사
            if (!isColliding)
            {
                for (int j = 0; j < Car_Count(); ++j)
                {
                    if (j == i) continue;

                    // j번 차의 "현재 위치" 꼭짓점
                    auto otherCarCorners = Car_GetRotatedCorners(j);

                    if (checkCollisionCars(futureCarCorners, otherCarCorners))
                    {
                        isColliding = true;

                        // 둘 다 멈추게 하고 싶으면 j번 차 속도도 0으로
                        Car_SetSpeed(j, 0.0f);

                        break;
                    }
                }
            }
        }

        if (!isColliding)
        {
            Car_SetRotationY(i, newAngle);
            Car_SetPosition(i, new_dx, new_dz);

            float newWheelAngle =
                Car_GetWheelRotationX(i) + Car_GetSpeed(i) * 200.0f;
            Car_SetWheelRotationX(i, newWheelAngle);
        }
        else
        {
            if (i == 0)
            {
                GameState_SetCrushed(true);
            }
            Car_SetSpeed(i, 0.0f);
        }
    }


    // 핸들 복원은 한 번만
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