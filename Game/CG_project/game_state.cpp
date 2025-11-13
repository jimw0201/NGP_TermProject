#include "game_state.h"
#include "car.h"
#include "input_handle.h"
#include "environment.h"
#include <iostream>     
#include <gl/freeglut.h>
#include <ctime>

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