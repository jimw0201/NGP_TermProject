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
#include "collision.h"
#include "renderer.h"

// 클라이언트
#define clientWidth 900
#define clientHeight 600
GLfloat rColor = 0.8f;
GLfloat gColor = 1.0f;
GLfloat bColor = 1.0f;

// 타이머 관련
#define TIMER_VELOCITY 16

// 자동차 이동 및 회전 애니메이션
time_t currentTime;
void TimerFunction_UpdateMove(int value)
{

	currentTime = time(nullptr);
	if (!GameState_IsPaused())
	{
		GameState_SetElapsedSeconds(static_cast<int>(currentTime - GameState_GetPauseTime() - GameState_GetStartTime()));
	}

	Car_UpdateSpeed(GameState_GetCurrentGear());

	// 차량의 꼭짓점 계산
	auto carCorners = Car_GetRotatedCorners();
	// 주차 상태 업데이트
	Environment_UpdateParkingStatus(carCorners);

	if (Car_GetSpeed() != 0.0f)
	{
		float radians = glm::radians(Car_GetRotationY());
		float new_dx = Car_GetDX() + Car_GetSpeed() * sin(radians);
		float new_dz = Car_GetDZ() + Car_GetSpeed() * cos(radians);

		const float n = 2.0f;
		float newAngle = Car_GetRotationY() + Car_GetFrontWheelRotationY() * n * Car_GetSpeed();

		auto futureCarCorners = Car_GetRotatedCorners(new_dx, new_dz, newAngle);

		bool isColliding = false;
		if (!GameState_IsInvincible())
		{
			// 벽과의 충돌 여부 확인
			for (int i = 0; i < 4; ++i)
			{
				float wallX = (i % 2 == 0) ? 0.0f : (i == 1 ? GROUND_SIZE : -GROUND_SIZE);
				float wallZ = (i % 2 == 1) ? 0.0f : (i == 2 ? GROUND_SIZE : -GROUND_SIZE);
				float wallWidth = (i % 2 == 0) ? GROUND_SIZE * 2 : WALL_THICKNESS;
				float wallHeight = (i % 2 == 1) ? GROUND_SIZE * 2 : WALL_THICKNESS;

				if (checkCollisionWalls(futureCarCorners, wallX, wallZ, wallWidth, wallHeight))
				{
					isColliding = true;
					break;
				}
			}

			if (checkCollisionObstacle(futureCarCorners))
			{
				isColliding = true;
			}
		}

		if (!isColliding)
		{
			Car_SetRotationY(newAngle);
			Car_SetPosition(new_dx, new_dz);
			float newWheelAngle = Car_GetWheelRotationX() + Car_GetSpeed() * 200.0f;
			Car_SetWheelRotationX(newWheelAngle);
		}
		else
		{
			GameState_SetCrushed(true);
			Car_SetSpeed(0.0f);
		}
	
		// 핸들과 바퀴 복원 로직
		Input_UpdateHandleReturn();
	}

	// 화면 갱신 요청 및 타이머 재설정
	glutPostRedisplay();
	glutTimerFunc(TIMER_VELOCITY, TimerFunction_UpdateMove, 1);
}

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

	// 시간 초기화
	GameState_UpdateStartTime(time(nullptr));
	GameState_UpdatePauseTime(GameState_GetStartTime() - time(nullptr));

	Car_Init();

	Input_Init();

	glEnable(GL_DEPTH_TEST);
	// 자동체 액셀 브레이크 감지 - 이동 애니메이션
	glutTimerFunc(TIMER_VELOCITY, TimerFunction_UpdateMove, 1);

	Mesh_InitAll();

	Shader_InitAll();

	glutDisplayFunc(drawScene);					//--- 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);					//--- 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard);					// 키보드 입력
	glutKeyboardUpFunc(KeyboardUp);					// 키보드 입력
	//glutSpecialFunc(SpecialKeyboard);			// 키보드 입력(방향키 등 스페셜)
	glutMouseFunc(MouseButton);					// 마우스 버튼 콜백 등록
	glutMotionFunc(MouseMotion);				// 마우스 드래그 콜백 등록
	glutMainLoop();								//--- 이벤트 처리 시작
}
