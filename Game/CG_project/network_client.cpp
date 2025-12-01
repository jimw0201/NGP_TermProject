// network_client.cpp
#include "network_client.h"
#include "Protocol.h"
#include <ws2tcpip.h>
#include <stdio.h>
#include <process.h>   // _beginthreadex

#pragma comment(lib, "ws2_32.lib")

static SOCKET g_tcpSocket = INVALID_SOCKET;
static bool   g_connected = false;
static int    g_myPlayerID = -1;   // 내가 배정받은 ID

// S2C 수신용
static HANDLE g_recvThreadHandle = NULL;
static S2C_GameStateUpdatePacket g_latestState;
static volatile bool g_hasLatestState = false;

unsigned __stdcall Network_RecvThread(void* arg);


bool Network_Init(const char* serverIp, unsigned short port)
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
    addr.sin_port = htons(port);

    // 문자열 IP를 네트워크 바이트 순서로 변환
    if (inet_pton(AF_INET, serverIp, &addr.sin_addr) != 1)
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

    printf("서버에 TCP 연결 성공! (%s:%d)\n", serverIp, port);
    g_connected = true;

    unsigned int threadId;
    g_recvThreadHandle = (HANDLE)_beginthreadex(
        nullptr, 0, Network_RecvThread, nullptr, 0, &threadId);

    if (!g_recvThreadHandle) {
        printf("수신 스레드 생성 실패\n");
        g_connected = false;
        return false;
    }

    // ---- 여기부터: PlayerID 응답 패킷 받기 ----
    S2C_PlayerIdResponsePacket idPkt{};
    int bytesNeeded = sizeof(idPkt);
    int bytesReceived = 0;

    while (bytesReceived < bytesNeeded)
    {
        int ret = recv(g_tcpSocket,
            reinterpret_cast<char*>(&idPkt) + bytesReceived,
            bytesNeeded - bytesReceived,
            0);
        if (ret <= 0)
        {
            printf("PlayerID 패킷 수신 실패\n");
            Network_Shutdown();
            return false;
        }
        bytesReceived += ret;
    }

    if (idPkt.type != S2C_PlayerIdResponse)
    {
        printf("예상과 다른 패킷 타입 수신: %d\n", idPkt.type);
        Network_Shutdown();
        return false;
    }

    g_myPlayerID = idPkt.PlayerID;
    printf("[Client] 서버로부터 PlayerID %d 할당 받음\n", g_myPlayerID);

    return true;
}

void Network_Shutdown()
{
    if (g_tcpSocket != INVALID_SOCKET)
    {
        closesocket(g_tcpSocket);
        g_tcpSocket = INVALID_SOCKET;
    }

    WSACleanup();
    g_connected = false;

    printf("네트워크 종료\n");
}

SOCKET Network_GetTCPSocket()
{
    return g_tcpSocket;
}

bool Network_IsConnected()
{
    return g_connected;
}

int Network_GetMyPlayerID()
{
    return g_myPlayerID;
}

void Network_SendPlayerInput(const PlayerKey& key)
{
    if (!g_connected) return;
    if (g_myPlayerID < 0) return; // 아직 ID 못 받았으면 패킷 안 보냄

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

unsigned __stdcall Network_RecvThread(void* arg)
{
    while (g_connected) {
        // 1) 먼저 패킷 타입 1바이트 읽기
        PacketType type;
        char* pType = reinterpret_cast<char*>(&type);
        int   recvd = 0;
        int   need = sizeof(PacketType);

        while (recvd < need) {
            int ret = recv(g_tcpSocket, pType + recvd, need - recvd, 0);
            if (ret <= 0) {
                // 서버 끊김
                g_connected = false;
                return 0;
            }
            recvd += ret;
        }

        // 2) 패킷 타입에 따라 나머지 본문 읽기
        if (type == S2C_GameStart) {
            // 이 패킷은 type 말고 추가 데이터 없음
            // 필요하면 여기서 "라운드 시작" 플래그 세팅 가능
            continue;
        }
        else if (type == S2C_GameStateUpdate) {
            S2C_GameStateUpdatePacket pkt;
            pkt.type = type;

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
            g_latestState = pkt;
            g_hasLatestState = true;
        }
        else {
            // (추가 패킷 타입 생기면 여기서 처리)
            // 지금은 무시
        }
    }
    return 0;
}

bool Network_TryGetLatestGameState(S2C_GameStateUpdatePacket& outPkt)
{
    if (!g_connected) return false;
    if (!g_hasLatestState) return false;

    outPkt = g_latestState;
    g_hasLatestState = false;    // 한 번 읽으면 플래그 내리기 (원하면 유지해도 됨)
    return true;
}
