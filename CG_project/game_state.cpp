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

void GameState_TimerLoop(int value)
{
    time_t currentTime = time(nullptr); // 로컬 변수로 선언

    if (!GameState_IsPaused())
    {
        GameState_SetElapsedSeconds(static_cast<int>(currentTime - GameState_GetPauseTime() - GameState_GetStartTime()));
    }

    Car_UpdateSpeed(GameState_GetCurrentGear());

    // 차량의 꼭짓점 계산
    auto carCorners = Car_GetRotatedCorners();
    // 주차 상태 업데이트
    Environment_UpdateParkingStatus(carCorners);

    if (Car_GetSpeed() != 0.0f)
    {
        float radians = glm::radians(Car_GetRotationY());
        float new_dx = Car_GetDX() + Car_GetSpeed() * sin(radians);
        float new_dz = Car_GetDZ() + Car_GetSpeed() * cos(radians);

        const float n = 2.0f;
        float newAngle = Car_GetRotationY() + Car_GetFrontWheelRotationY() * n * Car_GetSpeed();

        auto futureCarCorners = Car_GetRotatedCorners(new_dx, new_dz, newAngle);

        bool isColliding = false;
        if (!GameState_IsInvincible())
        {
            // 벽과의 충돌 여부 확인
            for (int i = 0; i < 4; ++i)
            {
                // 벽 데이터는 environment.h나 mesh.h의 상수를 씁니다.
                // GROUND_SIZE, WALL_THICKNESS 사용을 위해 mesh.h 포함 필요할 수 있음 (Car_Init 등에서 이미 쓰임)
                float wallX = (i % 2 == 0) ? 0.0f : (i == 1 ? GROUND_SIZE : -GROUND_SIZE);
                float wallZ = (i % 2 == 1) ? 0.0f : (i == 2 ? GROUND_SIZE : -GROUND_SIZE);
                float wallWidth = (i % 2 == 0) ? GROUND_SIZE * 2 : WALL_THICKNESS;
                float wallHeight = (i % 2 == 1) ? GROUND_SIZE * 2 : WALL_THICKNESS;

                if (checkCollisionWalls(futureCarCorners, wallX, wallZ, wallWidth, wallHeight))
                {
                    isColliding = true;
                    break;
                }
            }

            if (checkCollisionObstacle(futureCarCorners))
            {
                isColliding = true;
            }
        }

        if (!isColliding)
        {
            Car_SetRotationY(newAngle);
            Car_SetPosition(new_dx, new_dz);
            float newWheelAngle = Car_GetWheelRotationX() + Car_GetSpeed() * 200.0f;
            Car_SetWheelRotationX(newWheelAngle);
        }
        else
        {
            GameState_SetCrushed(true);
            Car_SetSpeed(0.0f);
        }

        // 핸들과 바퀴 복원 로직
        Input_UpdateHandleReturn();
    }

    // 화면 갱신 요청 및 타이머 재설정
    glutPostRedisplay();
    // 재귀 호출 시 자기 자신(GameState_TimerLoop)을 호출
    glutTimerFunc(16, GameState_TimerLoop, 1); // TIMER_VELOCITY 대신 16 직접 사용하거나 헤더에 정의
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