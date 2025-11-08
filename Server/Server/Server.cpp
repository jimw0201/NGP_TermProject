#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>

#define MAX_PLAYERS 4
#define TCP_PORT 9000
#define UDP_PORT 9001
#define TIMER_VELOCITY 16


enum GearState {PARK, REVERSE, NEUTRAL, DRIVE};

struct PlayerKey {
	bool W_Pressed;
	bool SPACE_Pressed;
	bool Q_Pressed;
	bool E_Pressed;

	float handle_rotate_z;
};

struct PlayerData {
	int playerID;
	
	float car_dx;
	float car_dy;
	float car_dz;
	float car_rotateY;

	float front_wheels_rotateY;
	float wheel_rect_rotateX;

	GearState currentGear;
};

struct PlayerGameStats {
	int PlayerID;
	int CollisionCount;
	float ParkingSec;
	bool IsParked;
	bool IsEnterParking;
};

enum PacketType : uint8_t {
	C2S_PlayerUpdate = 0,
	S2C_GameStart,
	S2C_GameStateUpdate,
	S2C_GameOver,
	C2S_ReportParked,
	C2S_GameReady,
};

struct C2S_PlayerUpdatePacket {
	PacketType type = C2S_PlayerUpdate;
	int playerID;
	PlayerKey myData;
};

struct S2C_GameStateUpdatePacket {
	PacketType type = S2C_GameStateUpdate;
	int srvElapsedSec;
	PlayerData PlayerStates[MAX_PLAYERS];
	PlayerGameStats PlayerStats[MAX_PLAYERS];
};

struct C2S_ReportParkedPacket {
	PacketType type = C2S_ReportParked;
	int PlayerID;
};

struct ClientInfo {
	bool IsConnected;
	int PlayerID;

	SOCKET TCPSocket;
	HANDLE hTCPThread;

	sockaddr_in UDPaddr;
	bool UDPAddrInitialized;

	PlayerKey LastReceivedKeys;

	PlayerData playerData;
	PlayerGameStats playerStats;
};

CRITICAL_SECTION g_cs;
ClientInfo g_clients[MAX_PLAYERS];
int g_connectedClients = 0;

SOCKET g_UDPSocket;
bool g_IsGameRunning;
DWORD g_GameStartTime;

void Server_HandleNC(SOCKET newTCP_Socket, int PlayerID) {

}

void Server_movement(int PlayerID) {

}

void Server_CheckAllCollisions() {

}

bool Server_CheckGameOver() {

}

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

			Server_CheckGameOver();
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

    while (true) {
        if (!g_IsGameRunning) {

            Sleep(100);
            continue;
        }

        DWORD frameStartTime = GetTickCount();

        EnterCriticalSection(&g_cs);
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (g_clients[i].IsConnected) {
                Server_movement(i);
            }
        }
        LeaveCriticalSection(&g_cs);


        EnterCriticalSection(&g_cs);
        Server_CheckAllCollisions();
        LeaveCriticalSection(&g_cs);


        S2C_GameStateUpdatePacket updatePkt;
        updatePkt.srvElapsedSec = (GetTickCount() - g_GameStartTime) / 1000;

        EnterCriticalSection(&g_cs);
        {
            // 전역 상태를 패킷에 복사
            for (int i = 0; i < MAX_PLAYERS; i++) {
                updatePkt.PlayerStates[i] = g_clients[i].playerData;
                updatePkt.PlayerStats[i] = g_clients[i].playerStats;
            }

            // 모든 "접속+UDP주소등록 완료"된 클라이언트에게 전송 (Unicast) 
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (g_clients[i].IsConnected && g_clients[i].UDPAddrInitialized) {
                    sendto(g_UDPSocket, (char*)&updatePkt, sizeof(updatePkt), 0,
                        (sockaddr*)&g_clients[i].UDPaddr, sizeof(g_clients[i].UDPaddr));
                }
            }
        }
        LeaveCriticalSection(&g_cs);

        DWORD frameTime = GetTickCount() - frameStartTime;
        if (frameTime < TIMER_VELOCITY) {
            Sleep(TIMER_VELOCITY - frameTime);
        }
    }
    return 0;
}


int main(int argc, char* argv[]) {
    InitializeCriticalSection(&g_cs);
    memset(g_clients, 0, sizeof(g_clients));

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

    SOCKET TCP_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in tcpAddr;
    memset(&tcpAddr, 0, sizeof(tcpAddr));
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpAddr.sin_port = htons(TCP_PORT);
    if (bind(TCP_ListenSock, (SOCKADDR*)&tcpAddr, sizeof(tcpAddr)) == SOCKET_ERROR) err_quit("TCP bind()");
    if (listen(TCP_ListenSock, SOMAXCONN) == SOCKET_ERROR) err_quit("TCP listen()");

    g_UDPSocket = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in udpAddr;
    memset(&udpAddr, 0, sizeof(udpAddr));
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    udpAddr.sin_port = htons(UDP_PORT);
    if (bind(g_UDPSocket, (SOCKADDR*)&udpAddr, sizeof(udpAddr)) == SOCKET_ERROR) err_quit("UDP bind()");

    _beginthreadex(NULL, 0, Server_UDP, NULL, 0, NULL);

    _beginthreadex(NULL, 0, Server_GameLoop, NULL, 0, NULL);

    printf(">> 주차의 달인 게임 서버 실행 [TCP Port: %d, UDP Port: %d]\n", TCP_PORT, UDP_PORT);
    printf(">> 클라이언트 접속 대기 중...\n");

    while (true) {
        SOCKET clientTcpSocket = accept(TCP_ListenSock, NULL, NULL);
        if (clientTcpSocket == INVALID_SOCKET) {
            printf("[오류] TCP accept() 실패\n");
            continue;
        }

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
            Server_HandleNC(clientTcpSocket, newPlayerID);

            g_clients[newPlayerID].hTCPThread = (HANDLE)_beginthreadex(NULL, 0, TCP_HandleClient, (LPVOID)newPlayerID, 0, NULL);
        }
        else {
            printf("[접속 거부] 서버가 꽉 찼습니다.\n");
            closesocket(clientTcpSocket);
        }
    }

    closesocket(TCP_ListenSock);
    closesocket(g_UDPSocket);
    DeleteCriticalSection(&g_cs);
    WSACleanup();
    return 0;
}