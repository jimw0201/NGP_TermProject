#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <ctime>

// 차량 기어 상태를 나타내는 열거형
enum GearState
{
	PARK,
	REVERSE,
	NEUTRAL,
	DRIVE
};

// 초기화
void GameState_Init();

// 다음 스테이지로 넘어가는 로직
void GameState_NextStage();
// 일정 시간 간격으로 호출되어 게임 로직 업데이트 하는 타이머 콜백
void GameState_TimerLoop(int value);

// --- Getters ---
bool GameState_IsPaused();				
bool GameState_IsClear();				
bool GameState_IsParked();				
bool GameState_IsInvincible();			
bool GameState_IsCrushed();				
int  GameState_GetCurrentStage();		
int  GameState_GetElapsedSeconds();		
GearState GameState_GetCurrentGear();	
time_t GameState_GetStartTime();		
time_t GameState_GetPauseTime();
time_t GameState_GetTempTime();

// --- Setters ---
void GameState_SetPaused(bool paused);
void GameState_SetClear(bool clear);
void GameState_SetParked(bool parked);
void GameState_SetInvincible(bool invincible);
void GameState_SetCrushed(bool crushed);
void GameState_SetCurrentStage(int stage);
void GameState_SetElapsedSeconds(int seconds);
void GameState_SetCurrentGear(GearState gear);
void GameState_UpdateStartTime(time_t time);
void GameState_UpdatePauseTime(time_t time);
void GameState_UpdateTempTime(time_t time);


#endif