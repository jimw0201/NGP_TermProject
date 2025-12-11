// network_client.cpp
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "network_client.h"
#include "game_state.h"
#include "Protocol.h"
#include <stdio.h>
// #include <process.h>   // _beginthreadex


static SOCKET g_tcpSocket = INVALID_SOCKET;
bool   g_connected = false;

SCREENState GameScreen = STATE_IP_INPUT;
int PlayerId = -1; // 아직 안 받은 상태
std::string SERVERIP = "";
CRITICAL_SECTION cs;
int   recvd = 0;

// 네트워크 송수신용
#define SERVERPORT 9000
#define BUFSIZE    512

// S2C 수신용
static HANDLE g_recvThreadHandle = NULL;
S2C_GameStateUpdatePacket g_latestState;
// StageScore g_stageScores[MAX_PLAYERS];
EndScore g_endScores[MAX_PLAYERS];
static volatile bool g_hasLatestState = false;

unsigned __stdcall Network_RecvThread(void* arg);


DWORD WINAPI Network_Init(LPVOID lpParam)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup 실패\n");
        return false;
    }

    g_tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_tcpSocket == INVALID_SOCKET)
    {
        printf("socket 생성 실패\n");
        WSACleanup();
        return false;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVERPORT);

    // 문자열 IP를 네트워크 바이트 순서로 변환
    if (inet_pton(AF_INET, SERVERIP.c_str(), &addr.sin_addr) != 1)
    {
        printf("inet_pton 실패 (IP 주소 확인)\n");
        closesocket(g_tcpSocket);
        WSACleanup();
        return false;
    }

    if (connect(g_tcpSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        printf("서버에 연결 실패 (connect 에러)\n");
        closesocket(g_tcpSocket);
        WSACleanup();
        return false;
    }

    printf("서버에 TCP 연결 성공!\n");
    g_connected = true;

    while (g_connected) {
        // 패킷 타입 먼저 수신
        PacketType type;
        recvd = 0;
        while (recvd < sizeof(PacketType)) {
            int ret = recv(g_tcpSocket, (char*)&type + recvd, sizeof(PacketType) - recvd, 0);
            if (ret <= 0) {
                g_connected = false;
                return 0;
            }
            recvd += ret;
        }
        char buf[BUFSIZE];
        // 타입별로 처리
        switch (type) {
        case S2C_GameStart: {// S2C_GameStart 라고 가정
            GameScreen = STATE_GAME_PLAY;   // 게임 화면으로 전환
            break;
        }
        case S2C_PlayerIdResponse: {
            S2C_PlayerIdResponsePacket idP{};

            //  구조체의 나머지 부분(패딩 포함)을 전부 읽기
            int bodySize = sizeof(S2C_PlayerIdResponsePacket) - sizeof(PacketType);

            char* pBody = reinterpret_cast<char*>(&idP) + sizeof(PacketType);

            int recvd = 0;
            while (recvd < bodySize)
            {
                int ret = recv(g_tcpSocket, pBody + recvd, bodySize - recvd, 0);
                if (ret <= 0) { g_connected = false; return 0; }
                recvd += ret;
            }

            PlayerId = idP.PlayerID;
            printf("[Client] 서버로부터 PlayerID %d 할당 받음\n", PlayerId);

            break;
        }
        case S2C_StageClear: {
            // 스테이지 클리어 패킷 수신 시 UI 표시 설정
            S2C_StageClearPacket pkt{};
            GameState_SetShowClearUI(true);
            break;
        }
        case S2C_GameStateUpdate: {
            S2C_GameStateUpdatePacket pkt{};

            int bodySize = sizeof(S2C_GameStateUpdatePacket) - sizeof(PacketType);
            char* pBody = reinterpret_cast<char*>(&pkt) + sizeof(PacketType);
            recvd = 0;

            while (recvd < bodySize) {
                int ret = recv(g_tcpSocket, pBody + recvd, bodySize - recvd, 0);
                if (ret <= 0) {
                    g_connected = false;
                    return 0;
                }
                recvd += ret;
            }

            // 최신 상태 갱신
            EnterCriticalSection(&cs);
            g_latestState = pkt;
            g_hasLatestState = true;

            if (pkt.currentStage != GameState_GetCurrentStage()) {
                // 만약 클리어 UI가 켜져있었다면 끄기 (다음 스테이지 시작)
                GameState_SetShowClearUI(false);
            }
            LeaveCriticalSection(&cs);
            break;
        }
        case S2C_GameOver: {
            S2C_GameOverPacket pkt{};

            int bodySize = sizeof(S2C_GameOverPacket) - sizeof(PacketType);
            char* pBody = reinterpret_cast<char*>(&pkt) + sizeof(PacketType);
            recvd = 0;

            while (recvd < bodySize) {
                int ret = recv(g_tcpSocket, pBody + recvd, bodySize - recvd, 0);
                if (ret <= 0) {
                    g_connected = false;
                    return 0;
                }
                recvd += ret;
            }

            for (int i = 0; i < MAX_PLAYERS; ++i) {
                g_endScores[i] = pkt.playerEnd[i];
            }

            GameScreen = STATE_END;
            break;
        }
        }
    }

    DeleteCriticalSection(&cs);
    closesocket(g_tcpSocket);
    WSACleanup();
    printf("[NET] disconnected\n");

    return 0;
}

// 클라->서버 패킷 전송 함수
void Network_SendPlayerInput(const PlayerKey& key)
{
    if (!g_connected) return;
    if (PlayerId < 0) return; // 아직 ID 못 받았으면 패킷 안 보냄

    C2S_PlayerUpdatePacket pkt{};
    pkt.type = C2S_PlayerUpdate;
    pkt.myData = key;

    int bytesToSend = sizeof(pkt);
    int bytesSent = 0;

    while (bytesSent < bytesToSend)
    {
        int ret = send(g_tcpSocket,
            reinterpret_cast<const char*>(&pkt) + bytesSent,
            bytesToSend - bytesSent,
            0);
        if (ret <= 0)
        {
            printf("입력 패킷 전송 중 오류 발생\n");
            g_connected = false;
            break;
        }
        bytesSent += ret;
    }
}

// 위치 실시간 업데이트용 최신 게임 상태 패킷 메모리에서 가져오기
bool Network_TryGetLatestGameState(S2C_GameStateUpdatePacket& outPkt)
{
    if (!g_connected) return false;
    EnterCriticalSection(&cs);
    if (!g_hasLatestState) {
        LeaveCriticalSection(&cs);
        return false;
    }

    outPkt = g_latestState;
    g_hasLatestState = false; // 한 번 읽으면 플래그 내리기 (원하면 유지해도 됨)
    LeaveCriticalSection(&cs);
    return true;
}
