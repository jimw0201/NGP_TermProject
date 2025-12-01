// network_client.h
#pragma once

#include <winsock2.h>
#include "Protocol.h"

// 네트워크 초기화: 서버 IP/포트로 TCP 연결 시도
bool Network_Init(const char* serverIp, unsigned short port);

// 네트워크 종료: 소켓 닫고 WSACleanup
void Network_Shutdown();

// 현재 TCP 소켓 반환 (send/recv할 때 사용)
SOCKET Network_GetTCPSocket();

// 연결되어 있는지 여부
bool Network_IsConnected();

int  Network_GetMyPlayerID();

void Network_SendPlayerInput(const PlayerKey& key);

// 서버가 보내는 게임 상태 패킷 가져오기
bool Network_TryGetLatestGameState(S2C_GameStateUpdatePacket& outPkt);
