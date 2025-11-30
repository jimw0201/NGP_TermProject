#define _CRT_SECURE_NO_WARNINGS
#include <ws2tcpip.h>
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

// 네트워크 송수신용
char* SERVERIP;
#define SERVERPORT 9000
#define BUFSIZE    512

// 클라이언트
#define clientWidth 900
#define clientHeight 600

// 타이머 관련
#define TIMER_VELOCITY 16

// 오류
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
/*
DWORD WINAPI Client_TCP(LPVOID lpParam) {
    // 1. 윈속 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 실패\n";
        return;
    }

    // 2. 소켓 생성
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "소켓 생성 실패\n";
        return;
    }

    // 3. 서버 연결 정보 설정
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000); // 서버 포트 번호 
    serverAddr.sin_addr.s_addr = inet_addr("서버 ip"); // 서버 IP 

    // 4. 서버 연결 시도
    if (connect(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "서버 연결 실패\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
    }
    std::cout << ">>> 서버에 연결되었습니다!\n";

    // 5. 송수신 루프 (무한 반복)
    while (isRunning)
    {
        
    }

    // 6. 종료 처리
    closesocket(serverSocket);
    WSACleanup();
}
*/

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

	// 개발자 정의 함수 게임에 필요한 데이터들 초기화
	// _INIT : 해당 함수와 관련된 데이터를 초기화하겠다. 
	// _INITALL : 해당 함수와 관련된 모든 것을 초기화하겠다.
	// -> 개발자가 미리 작성해 둔 변수 값 채우기 작업이 수행됨
	GameState_Init(); 
	Car_Init();       
	Input_Init();     
	Environment_Init();
	Mesh_InitAll();   
	Shader_InitAll(); 

	// glEnable, glut... 함수들 모두 라이브러리(OpenGL, GLUT)에서 제공하는 내장 기능 함수
	// 내장 함수들이 매개변수(괄호 안)로 받고 있는 것들은 사용자 정의 함수
	// 어떤 상황에 내 함수를 실행시킬지 등록하는 과정
	glEnable(GL_DEPTH_TEST); // 더 앞에 있는 물체가 뒤에 있는 물체를 가리도록 처리(그래픽)

	// glut...Func함수들은 사건, 행동을 연결해주는 접착제 역할
	glutDisplayFunc(drawScene); // 화면을 그려야 할 때가 되면 drawScene함수를 실행하라
	// 작동: 윈도우가 처음 켜지거나, 가려졌다가 다시 나타날 때 이 함수가 호출되어 화면을 그립니다.
	
	//"창 크기가 바뀔 때(Reshape), Reshape 함수를 실행해라."
	// 작동: 사용자가 마우스로 창의 모서리를 잡아당겨 크기를 바꾸면,
	// 이 함수가 실행되어 화면 비율이 찌그러지지 않게 조정합니다.
	glutReshapeFunc(Reshape);

	// 사용자 입력(컨트롤)관련
	// 의미: "키보드를 눌렀을 때, Keyboard 함수를 실행해라."
	//용도: 'W'를 누르면 전진, 'S'를 누르면 후진하도록 설정.
	glutKeyboardFunc(Keyboard);
	// 의미: "키보드에서 손을 뗐을 때, KeyboardUp 함수를 실행해라."
	// 용도: 'W'에서 손을 떼면 자동차가 멈추도록 설정.
	glutKeyboardUpFunc(KeyboardUp);
	//의미: "마우스 버튼을 클릭했을 때, MouseButton 함수를 실행해라."
	glutMouseFunc(MouseButton);
	// 의미: "마우스 버튼을 누른 채로 움직일 때(드래그), MouseMotion 함수를 실행해라."
	// 용도: 주로 게임 화면(카메라)을 돌려볼 때 사용합니다.
	glutMotionFunc(MouseMotion);

	// 타이머
	// 의미: "TIMER_VELOCITY 밀리초(ms) 뒤에, GameState_TimerLoop 함수를 딱 한 번 실행해라."
	// 핵심: 이 함수는 보통 GameState_TimerLoop 내부에서 자기 자신을 또다시 호출하도록 만듭니다.
	// 결과 : 무한히 반복되면서 게임 시간(프레임)을 흐르게 만듭니다.여기서 자동차 이동, 충돌 체크 등을 수행합니다.
	glutTimerFunc(TIMER_VELOCITY, GameState_TimerLoop, 1);

	/*
    // 네트워크 송수신용 스레드 생성
    hThread = CreateThread(NULL, 0, Client_TCP, (LPVOID)i, 0, NULL);

    if (hThread == NULL) {
        EnterCriticalSection(&cs);
        client[i].socket = INVALID_SOCKET;
        clientnum--;
        LeaveCriticalSection(&cs);
        closesocket(client_sock);
    }
    else { CloseHandle(hThread); }
	*/


	// 무한루프(실행시작)
	// 의미: "자, 이제 모든 설정이 끝났으니 프로그램을 가동해라!"
	// 설명: 이 함수가 호출되는 순간, 프로그램은 무한 반복문(While Loop) 상태로 들어갑니다.
	// 이 줄 아래에 어떤 코드를 적어도 실행되지 않습니다.프로그램은 이제 사용자가 창을 닫을 때까지 위에서 등록한 함수들(drawScene, Keyboard 등)을 상황에 맞게 계속 호출하며 살아 움직이게 됩니다.
	glutMainLoop();
}
