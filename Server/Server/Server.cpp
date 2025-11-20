#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <vector>
#include "Protocol.h"
#include "GameLogic.h"

#define TCP_PORT 9000
#define UDP_PORT 9001
#define TIMER_VELOCITY 16

const float g_round1_StartPos[MAX_PLAYERS][3] = {
    {0.0f, -4.0f, 0.0f},
    {0.0f, 4.0f, 180.0f},
    {4.0f, 0.0f, -90.0f},
    {-4.0f, 0.0f, 90.0f}
};

CRITICAL_SECTION g_cs;
ClientInfo g_clients[MAX_PLAYERS];
int g_connectedClients = 0;

SOCKET g_UDPSocket;
bool g_IsGameRunning;
DWORD g_GameStartTime;

void err_quit(const char* msg)
{
    LPVOID IpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&IpMsgBuf, 0, NULL);
    MessageBoxA(NULL, (const char*)IpMsgBuf, msg, MB_ICONERROR);
    LocalFree(IpMsgBuf);
    exit(1);
}

unsigned int WINAPI TCP_HandleClient(LPVOID arg) {
	int playerID = (int)arg;
	SOCKET clientSock = g_clients[playerID].TCPSocket;

	char recvBuf[1024];
	while (true) {
		int retval = recv(clientSock, recvBuf, sizeof(recvBuf), 0);
		if (retval == SOCKET_ERROR || retval == 0) {
			break; // 접속 종료
		}

		PacketType* pType = (PacketType*)recvBuf;
		if (*pType == C2S_ReportParked) {
			C2S_ReportParkedPacket* pkt = (C2S_ReportParkedPacket*)recvBuf;
			printf("[TCP] 클라이언트 %d: 주차 완료 보고 (C2S_ReportParked)\n", pkt->PlayerID);

			EnterCriticalSection(&g_cs);
			g_clients[pkt->PlayerID].playerStats.IsParked = true;
			g_clients[pkt->PlayerID].playerStats.ParkingSec = (float)(GetTickCount() - g_GameStartTime) / 1000.0f;
			LeaveCriticalSection(&g_cs);

            bool isGameOver = Server_CheckGameOver();

            if (isGameOver) {
                printf("[Game] 모든 플레이어 주차 완료! 라운드 점수 계산을 시작합니다....\n");

                // 여기 EndMatch()관련 내용 넣으면 됨
            }
		}
	}

	printf("[TCP] 클라이언트 %d 접속 종료.\n", playerID);
	EnterCriticalSection(&g_cs);
	{
		g_clients[playerID].IsConnected = false;
		g_connectedClients--;
	}
	LeaveCriticalSection(&g_cs);

	closesocket(clientSock);
	return 0;
}

unsigned int WINAPI Server_UDP(LPVOID arg) {
    char recvBuffer[2048];
    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);

    printf("[UDP] Server_UDP 스레드 시작 (Port: %d)...\n", UDP_PORT);

    while (true) {
        int recvLen = recvfrom(g_UDPSocket, recvBuffer, sizeof(recvBuffer), 0, (sockaddr*)&clientAddr, &addrLen);
        if (recvLen == SOCKET_ERROR) {
            continue;
        }

        PacketType* pType = (PacketType*)recvBuffer;
        if (*pType == C2S_PlayerUpdate) {
            C2S_PlayerUpdatePacket* pkt = (C2S_PlayerUpdatePacket*)recvBuffer;
            int id = pkt->playerID;

            if (id < 0 || id >= MAX_PLAYERS) continue; // Invalid ID

            EnterCriticalSection(&g_cs);
            {
                if (g_clients[id].IsConnected && !g_clients[id].UDPAddrInitialized) {
                    g_clients[id].UDPaddr = clientAddr;
                    g_clients[id].UDPAddrInitialized = true;
                    printf("[UDP] 클라이언트 %d의 UDP 주소 등록 완료\n", id);
                }

                g_clients[id].LastReceivedKeys = pkt->myData;
            }
            LeaveCriticalSection(&g_cs);
        }
    }
    return 0;
}

unsigned int WINAPI Server_GameLoop(LPVOID arg) {
    printf("[Game] 게임 루프 스레드 시작 (Tick: %dms)\n", TIMER_VELOCITY);

    S2C_GameStateUpdatePacket updatePkt;

    while (true) {
        if (!g_IsGameRunning) {
            if (g_connectedClients == MAX_PLAYERS) {
                printf("[Game] 4명 접속 완료. 게임을 시작합니다.\n");
                g_GameStartTime = GetTickCount();
                g_IsGameRunning = true;
            }
            Sleep(100);
            continue;
        }

        DWORD frameStartTime = GetTickCount();

        EnterCriticalSection(&g_cs);
        {
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (g_clients[i].IsConnected) {
                    Server_movement(i);     // 물리 연산
                }
            }
            Server_CheckAllCollisions();    // 충돌 판정

            updatePkt.srvElapsedSec = (GetTickCount() - g_GameStartTime) / 1000;
            for (int i = 0; i < MAX_PLAYERS; i++) {
                updatePkt.PlayerStates[i] = g_clients[i].playerData;
                updatePkt.PlayerStats[i] = g_clients[i].playerStats;
            }
        }
        LeaveCriticalSection(&g_cs);

        
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (g_clients[i].IsConnected && g_clients[i].UDPAddrInitialized) {
                sendto(g_UDPSocket, (char*)&updatePkt, sizeof(updatePkt), 0, 
                    (sockaddr*)&g_clients[i].UDPaddr, sizeof(g_clients[i].UDPaddr));
            }
        }

        DWORD frameTime = GetTickCount() - frameStartTime;
        if (frameTime < TIMER_VELOCITY) {
            Sleep(TIMER_VELOCITY - frameTime);
        }
    }
    return 0;
}


int main(int argc, char* argv[]) {
    InitializeCriticalSection(&g_cs);
    // 클라이언트 정보 배열 초기화
    memset(g_clients, 0, sizeof(g_clients));

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

    // TCP 연결 설정
    SOCKET TCP_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in tcpAddr;
    memset(&tcpAddr, 0, sizeof(tcpAddr));
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpAddr.sin_port = htons(TCP_PORT);
    if (bind(TCP_ListenSock, (SOCKADDR*)&tcpAddr, sizeof(tcpAddr)) == SOCKET_ERROR) err_quit("TCP bind()");
    if (listen(TCP_ListenSock, SOMAXCONN) == SOCKET_ERROR) err_quit("TCP listen()");

    // UDP 연결 설정
    g_UDPSocket = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in udpAddr;
    memset(&udpAddr, 0, sizeof(udpAddr));
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    udpAddr.sin_port = htons(UDP_PORT);
    if (bind(g_UDPSocket, (SOCKADDR*)&udpAddr, sizeof(udpAddr)) == SOCKET_ERROR) err_quit("UDP bind()");

    // UDP 패킷 처리하는 스레드 실행
    _beginthreadex(NULL, 0, Server_UDP, NULL, 0, NULL);

    // 물리 연산, 충돌 체크, 클라이언트 상태 업데이트용 스레드 실행
    _beginthreadex(NULL, 0, Server_GameLoop, NULL, 0, NULL);

    printf(">> 주차의 달인 게임 서버 실행 [TCP Port: %d, UDP Port: %d]\n", TCP_PORT, UDP_PORT);
    printf(">> 클라이언트 접속 대기 중...\n");

    while (true) {
        // TCP 연결 수락
        SOCKET clientTcpSocket = accept(TCP_ListenSock, NULL, NULL);
        if (clientTcpSocket == INVALID_SOCKET) {
            printf("[오류] TCP accept() 실패\n");
            continue;
        }

        // Nagle 알고리즘 설정
        BOOL optval = TRUE;
        int optlen = sizeof(optval);
        if (setsockopt(clientTcpSocket, IPPROTO_TCP, TCP_NODELAY,
            (const char*)&optval, optlen) == SOCKET_ERROR) {
            printf("[경고] TCP_NODELAY 설정 실패 (Socket: %llu)\n", clientTcpSocket);
        }

        // 빈 슬롯(PlayerID) 찾기
        // 최대 플레이어 수 내에서 빈 자리를 찾아 ID를 할당
        int newPlayerID = -1;
        EnterCriticalSection(&g_cs);
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!g_clients[i].IsConnected) {
                newPlayerID = i;
                break;
            }
        }
        LeaveCriticalSection(&g_cs);

        if (newPlayerID != -1) {
            // 신규 클라이언트 초기화
            Server_HandleNC(clientTcpSocket, newPlayerID);

            // 해당 클라이언트 전담 TCP 수신 스레드 생성
            // ClientInfo 구조체에 스레드 핸들 저장
            g_clients[newPlayerID].hTCPThread = (HANDLE)_beginthreadex(NULL, 0, TCP_HandleClient, (LPVOID)newPlayerID, 0, NULL);
        }
        else {
            printf("[접속 거부] 서버가 꽉 찼습니다.\n");
            closesocket(clientTcpSocket);
        }

        // 여기 코드 복붙(ID 전송 코드)
    }

    closesocket(TCP_ListenSock);
    closesocket(g_UDPSocket);
    DeleteCriticalSection(&g_cs);
    WSACleanup();
    return 0;
}