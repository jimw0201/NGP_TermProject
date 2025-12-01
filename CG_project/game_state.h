#ifndef GAME_STATE_H
#define GAME_STATE_H
#include <Windows.h>
#include <ctime>
#include <string>

enum SCREENState {
    STATE_IP_INPUT,  // 내부적으로 0
    STATE_CONNECTING,
	STATE_GAME_PLAY
};

extern int PlayerId;

// 서버 IP 주소 전역 변수
extern std::string SERVERIP;

extern CRITICAL_SECTION cs;

extern int width;
extern int height;

DWORD WINAPI Network(LPVOID lpParam);

extern SCREENState GameScreen;

// 차량 기어 상태를 나타내는 열거형
enum GearState
{
	PARK,
	REVERSE,
	NEUTRAL,
	DRIVE
};

// 멀티 대비용: 한 차량의 입력 상태
struct CarInput
{
    bool accelForward;    // 가속 (전진)
    bool accelBackward;   // 가속 (후진)
    bool brake;           // 브레이크
    float steering;       // 조향각 (front_wheels_rotateY)
    GearState gear;       // 현재 기어 상태
};

// 초기화
void GameState_Init();

// 다음 스테이지로 넘어가는 로직
void GameState_NextStage();
// 일정 시간 간격으로 호출되어 게임 로직 업데이트 하는 타이머 콜백
void GameState_TimerLoop(int value);

// --- 게임 접속 UI ---
void RenderIpInputScreen();

// --- 오류 메시지 출력 ---
void err_display(const char* msg);
void err_quit(const char* msg);

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

// CarInput 배열 접근용 (나중에 서버/멀티플레이에서 사용)
CarInput* GameState_GetCarInputs();       // 전체 배열 포인터
const CarInput& GameState_GetCarInput(int idx); // 특정 차량 입력 참조


#endif