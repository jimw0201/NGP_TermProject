#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <ctime>

enum GearState
{
	PARK,
	REVERSE,
	NEUTRAL,
	DRIVE
};

// --- √ ±‚»≠ ---
void GameState_Init();

void GameState_NextStage();

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