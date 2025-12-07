// network_client.h
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Protocol.h"

DWORD WINAPI Network_Init(LPVOID lpParam);

extern bool g_connected;

void Network_SendPlayerInput(const PlayerKey& key);

// 서버가 보내는 게임 상태 패킷 가져오기
bool Network_TryGetLatestGameState(S2C_GameStateUpdatePacket& outPkt);
