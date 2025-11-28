#include "renderer.h"

// 다른 모듈의 정보를 가져와서 그려야 하므로 모두 포함
#include "game_state.h"
#include "mesh.h"
#include "shader.h"
#include "car.h"
#include "input_handle.h"
#include "environment.h"

#include <iostream>
#include <string>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

static int width = 900;
static int height = 600;

// 배경색
static GLfloat rColor = 0.8f;
static GLfloat gColor = 1.0f;
static GLfloat bColor = 1.0f;

// 투영, 은면
bool isProspect = true;
bool isCull = false;

// 조명 변수
static GLfloat lightX = 0.0f;
static GLfloat lightY = 5.0f;
static GLfloat lightZ = 1.0f;
static float light = 0.8f;

// 플레이어별 차 몸체 색 (1P~4P)
static const glm::vec3 g_playerBodyColors[kCarCount] = {
	glm::vec3(1.0f, 0.0f, 0.0f),	// 1P: 빨강
	glm::vec3(1.0f, 0.0f, 1.0f),	// 2P: 핑크
	glm::vec3(0.55f, 0.27f, 0.07f), // 3P: 갈색
	glm::vec3(0.0f, 0.0f, 1.0f)		// 4P: 파랑
};

// 텍스트 렌더링
static void RenderBitmapString(float x, float y, void* font, const char* string)
{
	glRasterPos2f(x, y);
	while (*string)
	{
		glutBitmapCharacter(font, *string);
		string++;
	}
}

// 행렬 계산 헬퍼들
static glm::mat4 Handle()
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Rx = glm::mat4(1.0f);
	glm::mat4 Rz = glm::mat4(1.0f);

	T = glm::translate(T, glm::vec3(0.0, HANDLE_SIZE - HANDLE_SIZE / 4, 0.1));
	Rx = glm::rotate(Rx, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
	Rz = glm::rotate(Rz, glm::radians(Input_GetHandleRotation()), glm::vec3(0.0, 0.0, 1.0));

	return Rz * T * Rx;
}

static glm::mat4 Gear()
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Rx = glm::mat4(1.0f);

	T = glm::translate(T, glm::vec3(0.5, -0.2, 0.1));
	Rx = glm::rotate(Rx, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));

	return T * Rx;
}

static glm::mat4 PointMode()
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Rx = glm::mat4(1.0f);
	glm::mat4 S = glm::mat4(1.0f);

	S = glm::scale(S, glm::vec3(5.0, 5.0, 5.0));
	Rx = glm::rotate(Rx, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));

	return T * Rx * S;
}

static glm::mat4 Gear_Stick()
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Rx = glm::mat4(1.0f);

	float gearYOffset = 0.0f;
	switch (GameState_GetCurrentGear())
	{
	case PARK: gearYOffset = 0.55f; break;
	case REVERSE: gearYOffset = 0.1f; break;
	case NEUTRAL: gearYOffset = -0.35f; break;
	case DRIVE: gearYOffset = -0.75f; break;
	}

	T = glm::translate(T, glm::vec3(0.6, gearYOffset, 0.1));
	Rx = glm::rotate(Rx, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));

	return T * Rx;
}

static glm::mat4 RearCameraView()
{
	glm::vec3 carPosition(Car_GetDX(), Car_GetDY(), Car_GetDZ());
	float radians = glm::radians(Car_GetRotationY());
	glm::vec3 carDirection(-sin(radians), 0.0f, -cos(radians));

	glm::vec3 cameraPosition = carPosition + carDirection * 0.45f + glm::vec3(0.0f, 0.15f, 0.0f);
	glm::vec3 lookAtTarget = carPosition + carDirection * 1.0f;
	glm::vec3 upVector(0.0f, 1.0f, 0.0f);

	return glm::lookAt(cameraPosition, lookAtTarget, upVector);
}


// 그리기 함수들
static void headLight(int modelLoc)
{
	glm::mat4 headLightModelLeft = Headlights(0);
	glm::vec3 headLightWorldPosLeft = glm::vec3(headLightModelLeft[3]);

	glm::mat4 headLightModelRight = Headlights(1);
	glm::vec3 headLightWorldPosRight = glm::vec3(headLightModelRight[3]);

	glm::vec3 headLightPos = (headLightWorldPosLeft + headLightWorldPosRight) * 0.5f;

	float radians = glm::radians(Car_GetRotationY());
	glm::vec3 carForwardDir(-sin(radians), 0.0f, -cos(radians));
	glm::vec3 headLightDir = glm::normalize(-carForwardDir);

	GLuint headLightPosLoc = glGetUniformLocation(shaderProgramID, "headLightPos");
	GLuint headLightDirLoc = glGetUniformLocation(shaderProgramID, "headLightDir");
	GLuint headLightColorLoc = glGetUniformLocation(shaderProgramID, "headLightColor");

	glUniform3f(headLightPosLoc, headLightPos.x, headLightPos.y, headLightPos.z);
	glUniform3f(headLightDirLoc, headLightDir.x, headLightDir.y, headLightDir.z);
	glUniform3f(headLightColorLoc, 1.0f, 1.0f, 0.8f);

	glUniform1f(glGetUniformLocation(shaderProgramID, "headLightCutOff"), cos(glm::radians(15.0f)));
	glUniform1f(glGetUniformLocation(shaderProgramID, "headLightOuterCutOff"), cos(glm::radians(20.0f)));
}

static void illuminate(int modelLoc)
{
	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
	glUniform3f(lightPosLocation, lightX, lightY, lightZ);

	if (GameState_IsPaused())
	{
		glUseProgram(shaderProgramID);
		int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
		glUniform3f(lightColorLocation, 0.1f, 0.1f, 0.1f);
	}
	else
	{
		glUseProgram(shaderProgramID);
		int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
		glUniform3f(lightColorLocation, 0.8f, 0.8f, 0.8f);
	}

	unsigned int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 1.0f, 0.5f, 0.6f);

	unsigned int ablColorLocation = glGetUniformLocation(shaderProgramID, "ambientLight");
	glUniform3f(ablColorLocation, light, light, light);
}

static void draw_handle(int modelLoc, int num)
{
	int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 0.25f, 0.25f, 0.25f);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	GLUquadricObj* qobj1;
	qobj1 = gluNewQuadric();
	gluDisk(qobj1, HANDLE_SIZE - HANDLE_SIZE / 2, HANDLE_SIZE, 20, 8);
	gluDeleteQuadric(qobj1);

	objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);
	glBindVertexArray(vao[2]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Handle()));
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void draw_gear(int modelLoc, int num)
{
	int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 0.5f, 0.5f, 0.5f);

	glBindVertexArray(vao[6]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Gear()));
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void draw_gear_stick(int modelLoc, int num)
{
	int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 0.25f, 0.25f, 0.25f);
	glBindVertexArray(vao[7]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Gear_Stick()));
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void draw_pointMode(int modelLoc, int num)
{
	int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 0.5f, 0.5f, 0.5f);

	glBindVertexArray(vao[6]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(PointMode()));
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void draw_wheels(int modelLoc, int num, int carIndex)
{
	int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 0.25f, 0.25f, 0.25f);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_on_000(num, 0, carIndex)));
	GLUquadricObj* qobj1 = gluNewQuadric();
	gluCylinder(qobj1, WHEEL_SIZE, WHEEL_SIZE, WHEEL_SIZE / 2, 20, 8);
	gluDeleteQuadric(qobj1);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_on_000(num, 1, carIndex)));
	GLUquadricObj* qobj2 = gluNewQuadric();
	gluDisk(qobj2, 0.0f, WHEEL_SIZE, 20, 8);
	gluDeleteQuadric(qobj2);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_on_000(num, 2, carIndex)));
	GLUquadricObj* qobj3 = gluNewQuadric();
	gluDisk(qobj3, 0.0f, WHEEL_SIZE, 20, 8);
	gluDeleteQuadric(qobj3);
}

static void drawCar(int modelLoc, int carIndex)
{
	int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");

	// 차체
	glm::vec3 bodyColor = g_playerBodyColors[carIndex];
	glUniform3f(objColorLocation, bodyColor.r, bodyColor.g, bodyColor.b);

	glBindVertexArray(vao[1]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Car_Body(carIndex)));
	glDrawArrays(GL_TRIANGLES, 0, 6 * 6);

	// 유리 부분
	objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 0.0f, 0.9f, 0.9f);
	glDrawArrays(GL_TRIANGLES, 36, 6 * 6);

	// 헤드라이트 메쉬
	objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 1.0f, 1.0f, 0.8f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Headlights(0, carIndex)));
	glDrawArrays(GL_TRIANGLES, 72, 6 * 6);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Headlights(1, carIndex)));
	glDrawArrays(GL_TRIANGLES, 108, 6 * 6);

	// 바퀴(원통)
	draw_wheels(modelLoc, 1, carIndex);
	draw_wheels(modelLoc, 2, carIndex);
	draw_wheels(modelLoc, 3, carIndex);
	draw_wheels(modelLoc, 4, carIndex);

	// 바퀴 박스
	objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 0.5f, 0.5f, 0.5f);

	glBindVertexArray(vao[4]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_rects(1, carIndex)));
	glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_rects(2, carIndex)));
	glDrawArrays(GL_TRIANGLES, 36, 6 * 6);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_rects(3, carIndex)));
	glDrawArrays(GL_TRIANGLES, 72, 6 * 6);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_rects(4, carIndex)));
	glDrawArrays(GL_TRIANGLES, 108, 6 * 6);
}


static void drawWalls(int modelLoc)
{
	int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 0.3f, 0.3f, 0.3f);
	glBindVertexArray(vao[3]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	glDrawArrays(GL_TRIANGLES, 0, 24);
}

static void drawGround(int modelLoc)
{
	int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 0.5f, 0.5f, 0.5f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	glBindVertexArray(vao[0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void drawFinishRect(int modelLoc)
{
	int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");

	glBindVertexArray(vao[5]);

	// 4개의 주차장 그리기
	for (int i = 0; i < 4; i++)
	{
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Environment_GetFinishRectMatrix(i)));

		// 주차장 색상 지정
		if (GameState_IsParked())
		{
			// 주차 성공 시 초록색으로 표시
			glUniform3f(objColorLocation, 0.0f, 1.0f, 0.0f);
		}
		else
		{
			switch (i)
			{
			case 0:
				glUniform3f(objColorLocation, 0.0f, 0.0f, 1.0f);
				break;
			case 1:
				glUniform3f(objColorLocation, 1.0f, 0.0f, 0.0f);
				break;
			case 2: // 갈색으로 수정
				glUniform3f(objColorLocation, 0.55f, 0.27f, 0.07f);
				break;
			case 3:
				glUniform3f(objColorLocation, 1.0f, 0.0f, 1.0f);
				break;
			}
		}

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glUniform3f(objColorLocation, 0.5f, 0.5f, 0.5f);
		glDrawArrays(GL_TRIANGLES, 6, 6);
	}
}

static void drawObstacleCars(int modelLoc)
{
	int currentStage = GameState_GetCurrentStage();

	// 3스테이지(벽)가 아닐 때만 주차선 그리기
	if (currentStage != 3)
	{
		glBindVertexArray(vao[8]);
		int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				glm::mat4 obsMat = Environment_GetObstacleMatrix(i, j);

				if (obsMat[3][0] > 50.0f) continue;

				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obsMat));

				// 바깥쪽 선 (흰색)
				glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				// 안쪽 채움 (회색)
				glUniform3f(objColorLocation, 0.5f, 0.5f, 0.5f);
				glDrawArrays(GL_TRIANGLES, 6, 6);
			}
		}
	}

	int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glBindVertexArray(vao[9]); // 큐브 모델 바인딩


	// 3라운드 : 긴 벽 2개 그리기
	if (currentStage == 3)
	{
		glUniform3f(objColorLocation, 0.0f, 0.0f, 0.0f); // 검은색

		// 벽 2개 그리기
		for (int j = 0; j < 2; j++)
		{
			glm::mat4 obsMat = Environment_GetObstacleMatrix(0, j);

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obsMat));
			glDrawArrays(GL_TRIANGLES, 0, TRI_COUNT * 3);
		}
	}
	// 1,2 라운드 작은 박스들 그리기
	else
	{
		glUniform3f(objColorLocation, 0.25f, 0.25f, 0.25f);

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 5; j++) // 5개의 장애물 그리기
			{
				glm::mat4 obsMat = Environment_GetObstacleMatrix(i, j);
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obsMat));
				glDrawArrays(GL_TRIANGLES, 0, TRI_COUNT * 3);
			}
		}
	}
}

static void drawCarCorners(int modelLoc)
{
	int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(objColorLocation, 1.0f, 0.0f, 0.0f);

	for (int i = 0; i < Car_Count(); ++i)
	{
		auto carCorners = Car_GetRotatedCorners(i);
		for (const auto& corner : carCorners)
		{
			float cornerX = corner.first;
			float cornerZ = corner.second;

			glm::mat4 T = glm::mat4(1.0f);
			T = glm::translate(T, glm::vec3(cornerX, CAR_SIZE * 0.75, cornerZ));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(T));

			GLUquadricObj* qobj1 = gluNewQuadric();
			gluSphere(qobj1, WHEEL_SIZE / 3, 10, 10);
			gluDeleteQuadric(qobj1);
		}
	}
}


// 메인 그리기 함수
void drawScene()
{
	glViewport(0, 0, width, height);

	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	int modelLoc = glGetUniformLocation(shaderProgramID, "modelTransform");
	int viewLoc = glGetUniformLocation(shaderProgramID, "viewTransform");
	int projLoc = glGetUniformLocation(shaderProgramID, "projectionTransform");

	// 3인칭 뷰
	{
		if (true)
		{
			if (isCull)
			{
				glDisable(GL_DEPTH_TEST);
			}
			else
			{
				glEnable(GL_DEPTH_TEST);
			}

			// 차체 중심을 공전 중심으로 설정
			glm::vec3 orbitCenter = glm::vec3(Car_GetDX(), Car_GetDY(), Car_GetDZ());

			// 카메라 위치 계산
			float cameraDistance = Input_GetCameraDZ();
			glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
			glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

			glm::mat4 cameraRotateMat = glm::rotate(glm::mat4(1.0f), glm::radians(Input_GetCameraRotateY()), glm::vec3(0.0, 1.0, 0.0));
			glm::vec3 cameraOffset = glm::vec3(cameraRotateMat * glm::vec4(0.0f, 1.9f, cameraDistance, 1.0f)); // Y축으로 살짝 올림
			glm::vec3 cameraPos = orbitCenter + cameraOffset;

			// 카메라 방향 업데이트 (살짝 아래로 보기)
			glm::vec3 lookTarget = orbitCenter + glm::vec3(0.0f, -0.2f, 0.0f); // 아래로 약간 이동
			cameraDirection = glm::normalize(lookTarget - cameraPos);

			// 뷰 행렬 설정
			glm::mat4 vTransform = glm::lookAt(cameraPos, lookTarget, cameraUp);
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

			unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos");
			glUniform3f(viewPosLocation, cameraPos.x, cameraPos.y, cameraPos.z);

			// 투영변환
			glm::mat4 pTransform = glm::mat4(1.0f);
			if (!isProspect)
			{
				pTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 100.0f);
			}
			else
			{
				pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 50.0f);
			}
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);
		}

		// 헤드라이트
		headLight(modelLoc);

		// 조명 설정
		illuminate(modelLoc);

		// 바닥 그리기
		drawGround(modelLoc);

		// 모델 그리기
		for (int i = 0; i < Car_Count(); ++i)
		{
			drawCar(modelLoc, i);
		}

		// 장애물 차 그리기
		drawObstacleCars(modelLoc);

		// 벽 그리기
		drawWalls(modelLoc);

		// 도착지점 그리기
		drawFinishRect(modelLoc);

		// 차 꼭지점 (좌표에 따라) 그리기 (디버깅, 무적)
		if (GameState_IsInvincible())
		{
			drawCarCorners(modelLoc);
		}
	}
	// 후방 카메라 뷰
	if (GameState_GetCurrentGear() == REVERSE)
	{
		// 후방 카메라 뷰포트 설정
		int rearViewWidth = width / 3;
		int rearViewHeight = height / 4;
		int rearViewX = (width - rearViewWidth) / 2; // 화면 중앙 상단
		int rearViewY = height - rearViewHeight - 10;

		glViewport(rearViewX, rearViewY, rearViewWidth, rearViewHeight);

		// 후방 카메라 뷰 행렬 설정
		glm::mat4 rearViewTransform = RearCameraView();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(rearViewTransform));

		// 동일한 투영 행렬 사용
		glm::mat4 rearProjTransform = glm::perspective(glm::radians(45.0f), (float)rearViewWidth / (float)rearViewHeight, 0.1f, 50.0f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(rearProjTransform));

		// 자동차, 바닥, 벽 등 모든 객체를 다시 그리기
		drawGround(modelLoc);
		for (int i = 0; i < Car_Count(); ++i)
		{
			drawCar(modelLoc, i);
		}
		drawObstacleCars(modelLoc);
		drawWalls(modelLoc);
		drawFinishRect(modelLoc);
	}
	glDisable(GL_DEPTH_TEST);
	// 핸들 - 뷰포트 설정으로 그리기
	if (true)
	{
		int miniMapWidth = 900 / 3;
		int miniMapHeight = 900 / 3;
		int miniMapX = 900 - miniMapWidth;
		int miniMapY = 900 - miniMapHeight;
		glViewport(miniMapX, 0, miniMapWidth, miniMapHeight);

		// 정면 뷰용 카메라 설정
		glm::mat4 topViewTransform = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 1.0f),	// 카메라 위치
			glm::vec3(0.0f, 0.0f, 0.0f),	// 어디를 바라볼 것인가
			glm::vec3(0.0f, 1.0f, 0.0f)		// 업 벡터
		);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &topViewTransform[0][0]);

		// 투영 변환 (직교 투영)
		glm::mat4 orthoTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 1.5f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, &orthoTransform[0][0]);

		// 핸들 그리기
		draw_handle(modelLoc, 0);
	}
	// 기어 그리기
	if (true)
	{
		int miniMapWidth = width / 3;
		int miniMapHeight = height / 3;
		int miniMapX = width - miniMapWidth;
		int miniMapY = height - miniMapHeight;
		glViewport(miniMapX, miniMapY, miniMapWidth, miniMapHeight);

		glm::mat4 topViewTransform = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 1.0f), // 카메라 위치
			glm::vec3(0.0f, 0.0f, 0.0f), // 바라보는 위치
			glm::vec3(0.0f, 1.0f, 0.0f)  // 업 벡터
		);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(topViewTransform));

		glm::mat4 orthoTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 1.5f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(orthoTransform));

		draw_gear(modelLoc, 0);

		// 텍스트 그리기
		glUseProgram(0);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, miniMapWidth, 0, miniMapHeight);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// 텍스트 위치 계산 (픽셀 단위)
		float textScale = 1.0f; // 텍스트 크기 조절
		float p_x = miniMapWidth * 0.65f;
		float r_x = miniMapWidth * 0.65f;
		float n_x = miniMapWidth * 0.65f;
		float d_x = miniMapWidth * 0.65f;
		float y = miniMapHeight * 0.5f;

		int star_count = 1;
		if (GameState_GetElapsedSeconds() < 30)
			glColor3f(1.0f, 1.0f, 1.0f); // 흰색
		else if (GameState_GetElapsedSeconds() < 60)
			glColor3f(1.0f, 1.0f, 0.0f); // 노란색
		else if (GameState_GetElapsedSeconds() >= 60)
			glColor3f(1.0f, 0.0f, 0.0f); // 빨간색
		std::string timeString = std::to_string(GameState_GetElapsedSeconds()) + "s";

		glPushMatrix();
		glTranslatef(p_x + 25, y + 84, 0.0f);
		glScalef(textScale, textScale, textScale);
		RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, timeString.c_str());
		glPopMatrix();

		glColor3f(1.0f, 1.0f, 1.0f); // 흰색
		glPushMatrix();
		glTranslatef(p_x, y + 50, 0.0f);
		glScalef(textScale, textScale, textScale);
		RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, "P");
		glPopMatrix();

		glPushMatrix();
		glTranslatef(r_x, y + 5, 0.0f);
		glScalef(textScale, textScale, textScale);
		RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, "R");
		glPopMatrix();

		glPushMatrix();
		glTranslatef(n_x, y - 40, 0.0f);
		glScalef(textScale, textScale, textScale);
		RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, "N");
		glPopMatrix();

		glPushMatrix();
		glTranslatef(d_x, y - 80, 0.0f);
		glScalef(textScale, textScale, textScale);
		RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, "D");
		glPopMatrix();

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glUseProgram(shaderProgramID);
	}
	// 기어 스틱 그리기
	if (true)
	{
		int miniMapWidth = width / 3;
		int miniMapHeight = height / 3;
		int miniMapX = width - miniMapWidth;
		int miniMapY = height - miniMapHeight;
		glViewport(miniMapX, miniMapY, miniMapWidth, miniMapHeight);

		glm::mat4 topViewTransform = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 1.0f), // 카메라 위치
			glm::vec3(0.0f, 0.0f, 0.0f), // 바라보는 위치
			glm::vec3(0.0f, 1.0f, 0.0f)  // 업 벡터
		);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(topViewTransform));

		glm::mat4 orthoTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 1.5f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(orthoTransform));

		draw_gear_stick(modelLoc, 0);
	}
	glEnable(GL_DEPTH_TEST);

	if (GameState_IsPaused())
	{
		int miniMapWidth = width / 2;
		int miniMapHeight = height / 2;
		int miniMapX = width - miniMapWidth * 3 / 2;
		int miniMapY = height - miniMapHeight * 3 / 2;
		glViewport(miniMapX, miniMapY, miniMapWidth, miniMapHeight);

		glm::mat4 topViewTransform = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 1.0f), // 카메라 위치
			glm::vec3(0.0f, 0.0f, 0.0f), // 바라보는 위치
			glm::vec3(0.0f, 1.0f, 0.0f)  // 업 벡터
		);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(topViewTransform));

		glm::mat4 orthoTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 1.5f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(orthoTransform));

		draw_pointMode(modelLoc, 0);

		// 텍스트 그리기
		glUseProgram(0);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, miniMapWidth, 0, miniMapHeight);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// 텍스트 위치 계산 (픽셀 단위)
		float textScale = 1.0f; // 텍스트 크기 조절
		float mx = miniMapWidth * 0.5f;
		float my = miniMapHeight * 0.5f;

		if (!GameState_IsClear()) // 정지 모드
		{
			glColor3f(1.0f, 1.0f, 1.0f); // 흰색
			std::string String = "PAUSE";
			glPushMatrix();
			glTranslatef(mx - 20, my + 50, 0.0f);
			glScalef(textScale, textScale, textScale);
			RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
			glPopMatrix();

			String = "Press 'ESC' to resume";
			glPushMatrix();
			glTranslatef(mx - 80, my - 50, 0.0f);
			glScalef(textScale, textScale, textScale);
			RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
			glPopMatrix();
		}
		else // 클리어 표시
		{
			glColor3f(1.0f, 1.0f, 1.0f); // 흰색
			std::string String = "stage " + std::to_string(GameState_GetCurrentStage()) + " clear!!";

			glPushMatrix();
			glTranslatef(mx - 50, my + 50, 0.0f);
			glScalef(textScale, textScale, textScale);
			RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
			glPopMatrix();

			glColor3f(1.0f, 1.0f, 0.0f); // 노란색
			int star_count = 1;
			if (GameState_GetElapsedSeconds() <= 60)
			{
				star_count++;
			}
			if (!GameState_IsCrushed())
			{
				star_count++;
			}

			String = "your star count : " + std::to_string(star_count);
			glPushMatrix();
			glTranslatef(mx - 65, my, 0.0f);
			glScalef(textScale, textScale, textScale);
			RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
			glPopMatrix();

			glColor3f(1.0f, 1.0f, 1.0f); // 흰색
			if (GameState_GetCurrentStage() <= 2)
			{
				String = "Press 'n' to next stage";
				glPushMatrix();
				glTranslatef(mx - 80, my - 50, 0.0f);
				glScalef(textScale, textScale, textScale);
				RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
				glPopMatrix();
			}
			else
			{
				String = "Press 'n' to quit game";
				glPushMatrix();
				glTranslatef(mx - 80, my - 50, 0.0f);
				glScalef(textScale, textScale, textScale);
				RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
				glPopMatrix();
			}
		}

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glUseProgram(shaderProgramID);
	}

	glutSwapBuffers();
}

void Reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}