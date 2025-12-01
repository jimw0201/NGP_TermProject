#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0600

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

#include <gl/glew.h>			
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#include "game_state.h"
#include "mesh.h"
#include "shader.h"
#include "car.h"
#include "input_handle.h"
#include "environment.h"
#include "renderer.h"
#include "protocol.h"

SCREENState GameScreen = STATE_IP_INPUT;
int PlayerId = -1; // 아직 안 받은 상태

std::string SERVERIP = "";
CRITICAL_SECTION cs;

int width = 900;
int height = 600;

// 네트워크 송수신용
#define SERVERPORT 9000
#define BUFSIZE    512

// 클라이언트
#define clientWidth 900
#define clientHeight 600

// 타이머 관련
#define TIMER_VELOCITY 16

// 메인 함수
int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//srand(time(0));

	//--- 윈도우 생성하기
	glutInit(&argc, argv);												//--- glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);			//--- 디스플레이 모드 설정 (+ 깊이 검사 추가)
	glutInitWindowPosition(100, 100);									//--- 윈도우의 위치 지정
	glutInitWindowSize(clientWidth, clientHeight);						//--- 윈도우의 크기 지정
	glutCreateWindow("parking_master");									//--- 윈도우 생성(윈도우 이름)

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) //--- glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	GameState_Init(); 
	Car_Init();       
	Input_Init();     
	Environment_Init();
	Mesh_InitAll();   
	Shader_InitAll(); 

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(KeyboardUp);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);

    glutTimerFunc(TIMER_VELOCITY, GameState_TimerLoop, 1);

	glutMainLoop();
}

DWORD WINAPI Network(LPVOID lpParam) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    sockaddr_in serveraddr{};
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP.c_str(), &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);

    int retval = connect(sock, (sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect() failed");

    printf("[NET] connected to server\n");

    while (1) {
        char buf[256];
        int n = recv(sock, buf, sizeof(buf), 0);
        if (n <= 0) {
            printf("[NET] disconnected\n");
            break;
        }

        unsigned char type = (unsigned char)buf[0];

		switch (type) {
		case S2C_GameStart: // S2C_GameStart 라고 가정 (enum으로 정의해두면 더 좋음)
			GameScreen = STATE_GAME_PLAY;   // 게임 화면으로 전환
			break;
		case S2C_PlayerIdResponse:
			S2C_PlayerIdResponsePacket* p = (S2C_PlayerIdResponsePacket*)buf;
			PlayerId = p->PlayerID;
			break;
		// case S2C_GameStateUpdate:

		// case S2C_GameOver:

		}
    }

    closesocket(sock);
    WSACleanup();
    printf("[NET] disconnected\n");
    return 0;
}