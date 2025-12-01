#define _CRT_SECURE_NO_WARNINGS
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

#include "network_client.h"

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

	// 2. 네트워크 초기화
	//    - 서버 IP/포트는 일단 로컬 테스트 기준
	//    - 나중에 설정 파일이나 인자에서 받아도 됨
	if (Network_Init("127.0.0.1", 9000))
	{
		printf("[Client] 서버 연결 성공!\n");
	}
	else
	{
		printf("[Client] 서버 연결 실패. 오프라인 모드로 실행합니다.\n");
		// 여기서 실패했다고 프로그램을 바로 종료할지,
		// 싱글 모드로 계속 돌릴지는 네가 선택
	}

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

	Network_Shutdown();
	return 0;
}
