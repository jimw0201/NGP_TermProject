#include "mesh.h"
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <cmath>

// #define TRI_COUNT 12
const int TRI_COUNT = 12;

// #define HANDLE_SIZE 0.7f
// #define HAND_RECT_SIZE HANDLE_SIZE / 4.0f
const float HANDLE_SIZE = 0.7f;
const float HAND_RECT_SIZE = HANDLE_SIZE / 4.0f;

// #define CAR_SIZE 0.5f
const float CAR_SIZE = 0.5f;

// #define OBSTACLE_WIDTH CAR_SIZE * 0.7f
// #define OBSTACLE_HEIGHT CAR_SIZE * 1.1f
const float OBSTACLE_WIDTH = CAR_SIZE * 0.7f;
const float OBSTACLE_HEIGHT = CAR_SIZE * 1.1f;

// #define WHEEL_SIZE CAR_SIZE / 4.0f
// #define WHEEL_RECT_SIZE WHEEL_SIZE / 8.0f
const float WHEEL_SIZE = CAR_SIZE / 4.0f;
static const float WHEEL_RECT_SIZE = WHEEL_SIZE / 8.0f;

// #define FINISH_SIZE 1.0f //(바깥쪽 사각형 크기)
// #define FINISH_SIZE_2 0.9f
const float FINISH_SIZE = 1.0f;
static const float FINISH_SIZE_2 = 0.9f;

// #define GROUND_SIZE 5.0f
// #define WALL_HEIGHT 0.5f
// #define WALL_THICKNESS 0.1f
const float GROUND_SIZE = 8.0f;
static const float WALL_HEIGHT = 0.5f;
const float WALL_THICKNESS = 0.2f;

const float fheight = 0.75f;
const float fy = 0.0001f;
static const float fy2 = 0.00015f;



GLfloat handle_rect[] = {
	-HAND_RECT_SIZE, 0.0f, -HAND_RECT_SIZE, 0.0f, 0.0f, 0.0f,
	HAND_RECT_SIZE, 0.0f, -HAND_RECT_SIZE,  0.0f, 0.0f, 0.0f,
	-HAND_RECT_SIZE, 0.0f, HAND_RECT_SIZE,	 0.0f, 0.0f, 0.0f,
	-HAND_RECT_SIZE, 0.0f, HAND_RECT_SIZE,  0.0f, 0.0f, 0.0f,
	HAND_RECT_SIZE, 0.0f, -HAND_RECT_SIZE,  0.0f, 0.0f, 0.0f,
	 HAND_RECT_SIZE, 0.0f, HAND_RECT_SIZE,	 0.0f, 0.0f, 0.0f,
};

GLfloat gear_rect[] = {
	-0.3f, 0, -1.0f, 0.0f, 0.0f, 0.0f,
	0.3f, 0, -1.0f,  0.0f, 0.0f, 0.0f,
	-0.3f, 0, 1.0f,	 0.0f, 0.0f, 0.0f,
	-0.3f, 0, 1.0f,  0.0f, 0.0f, 0.0f,
	0.3f, 0, -1.0f,  0.0f, 0.0f, 0.0f,
	0.3f, 0, 1.0f,	 0.0f, 0.0f, 0.0f,
};
GLfloat gear_stick_rect[] = {
	-0.1f, 0, -0.1f, 0.0f, 0.0f, 0.0f,
	0.1f, 0, -0.1f,  0.0f, 0.0f, 0.0f,
	-0.1f, 0, 0.1f,	 0.0f, 0.0f, 0.0f,
	-0.1f, 0, 0.1f,  0.0f, 0.0f, 0.0f,
	0.1f, 0, -0.1f,  0.0f, 0.0f, 0.0f,
	0.1f, 0, 0.1f, 	 0.0f, 0.0f, 0.0f,
};

GLfloat Block[4][TRI_COUNT * 3][6];

GLfloat obstacle_car[TRI_COUNT * 3][6];

GLfloat wheel_rect[4][TRI_COUNT * 3][6];

GLfloat finish_rect[] = {
	//바깥쪽 (z길이가 x길이의 두배로 설정)
	-FINISH_SIZE / 2, fy, -FINISH_SIZE * fheight, 0.0f,	1.0f,	0.0f,
	 FINISH_SIZE / 2, fy, -FINISH_SIZE * fheight, 0.0f,	1.0f,	0.0f,
	-FINISH_SIZE / 2, fy,  FINISH_SIZE * fheight, 0.0f,	1.0f,	0.0f,
	-FINISH_SIZE / 2, fy,  FINISH_SIZE * fheight, 0.0f,	1.0f,	0.0f,
	 FINISH_SIZE / 2, fy, -FINISH_SIZE * fheight, 0.0f,	1.0f,	0.0f,
	 FINISH_SIZE / 2, fy,  FINISH_SIZE * fheight, 0.0f,	1.0f,	0.0f,

	 //안쪽
	 -FINISH_SIZE_2 / 2, fy2, -FINISH_SIZE_2 * fheight,	 0.0f,	1.0f,	0.0f,
	  FINISH_SIZE_2 / 2, fy2, -FINISH_SIZE_2 * fheight,	 0.0f,	1.0f,	0.0f,
	 -FINISH_SIZE_2 / 2, fy2,  FINISH_SIZE_2 * fheight,	 0.0f,	1.0f,	0.0f,
	 -FINISH_SIZE_2 / 2, fy2,  FINISH_SIZE_2 * fheight,	 0.0f,	1.0f,	0.0f,
	  FINISH_SIZE_2 / 2, fy2, -FINISH_SIZE_2 * fheight,	 0.0f,	1.0f,	0.0f,
	  FINISH_SIZE_2 / 2, fy2,  FINISH_SIZE_2 * fheight,	 0.0f,	1.0f,	0.0f
};

GLfloat ground[] = {
	-GROUND_SIZE, 0.0f, -GROUND_SIZE,  0.0f,	1.0f,	0.0f,
	GROUND_SIZE, 0.0f, -GROUND_SIZE,   0.0f,	1.0f,	0.0f,
	-GROUND_SIZE, 0.0f, GROUND_SIZE,   0.0f,	1.0f,	0.0f,
	-GROUND_SIZE, 0.0f, GROUND_SIZE,   0.0f,	1.0f,	0.0f,
	GROUND_SIZE, 0.0f, -GROUND_SIZE,   0.0f,	1.0f,	0.0f,
	GROUND_SIZE, 0.0f, GROUND_SIZE,	   0.0f,	1.0f,	0.0f,
};

GLfloat walls[] = {
	// Front Wall
   -GROUND_SIZE, 0.0f, -GROUND_SIZE, 				  0.0f,	1.0f,	0.0f,
   GROUND_SIZE, 0.0f, -GROUND_SIZE, 				  0.0f,	1.0f,	0.0f,
   -GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE,		  0.0f,	1.0f,	0.0f,
   -GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE, 		  0.0f,	1.0f,	0.0f,
   GROUND_SIZE, 0.0f, -GROUND_SIZE, 				  0.0f,	1.0f,	0.0f,
   GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE,			  0.0f,	1.0f,	0.0f,
   // Back Wall
   -GROUND_SIZE, 0.0f, GROUND_SIZE, 				  0.0f,	1.0f,	0.0f,
   GROUND_SIZE, 0.0f, GROUND_SIZE, 				  0.0f,	1.0f,	0.0f,
   -GROUND_SIZE, WALL_HEIGHT, GROUND_SIZE,			  0.0f,	1.0f,	0.0f,
   -GROUND_SIZE, WALL_HEIGHT, GROUND_SIZE, 		  0.0f,	1.0f,	0.0f,
   GROUND_SIZE, 0.0f, GROUND_SIZE, 				  0.0f,	1.0f,	0.0f,
   GROUND_SIZE, WALL_HEIGHT, GROUND_SIZE,			  0.0f,	1.0f,	0.0f,
   // Left Wall
   -GROUND_SIZE, 0.0f, -GROUND_SIZE , 				  0.0f,	1.0f,	0.0f,
   -GROUND_SIZE, 0.0f, GROUND_SIZE , 				  0.0f,	1.0f,	0.0f,
   -GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE ,		  0.0f,	1.0f,	0.0f,
   -GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE , 		  0.0f,	1.0f,	0.0f,
   -GROUND_SIZE, 0.0f, GROUND_SIZE , 				  0.0f,	1.0f,	0.0f,
   -GROUND_SIZE, WALL_HEIGHT, GROUND_SIZE ,		  0.0f,	1.0f,	0.0f,
   // Right Wall
   GROUND_SIZE, 0.0f, -GROUND_SIZE , 				  0.0f,	1.0f,	0.0f,
   GROUND_SIZE, 0.0f, GROUND_SIZE, 				  0.0f,	1.0f,	0.0f,
   GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE,			  0.0f,	1.0f,	0.0f,
   GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE, 		  0.0f,	1.0f,	0.0f,
   GROUND_SIZE, 0.0f, GROUND_SIZE, 				  0.0f,	1.0f,	0.0f,
   GROUND_SIZE, WALL_HEIGHT, GROUND_SIZE,			  0.0f,	1.0f,	0.0f,
};





GLuint vao[20], vbo[20];

// 함수: 두 벡터의 외적을 계산
static void calculateNormal(const GLfloat* v1, const GLfloat* v2, const GLfloat* v3, GLfloat* normal, bool reverse = false)
{
	GLfloat u[3] = { v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2] };
	GLfloat v[3] = { v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2] };
	normal[0] = u[1] * v[2] - u[2] * v[1];
	normal[1] = u[2] * v[0] - u[0] * v[2];
	normal[2] = u[0] * v[1] - u[1] * v[0];

	if (reverse)
	{ // 뒷면 노말 방향 반전
		normal[0] = -normal[0];
		normal[1] = -normal[1];
		normal[2] = -normal[2];
	}

	GLfloat magnitude = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	if (magnitude != 0.0f)
	{
		normal[0] /= magnitude;
		normal[1] /= magnitude;
		normal[2] /= magnitude;
	}
}

// 자동차 초기화
static void initCar()
{
	//큐브 데이터 초기화
	GLuint CubeIndices[] = {
		// Front face
		0, 1, 2, 0, 2, 3,
		// Back face
		4, 5, 6, 4, 6, 7,
		// Right face
		1, 5, 6, 1, 6, 2,
		// Left face
		0, 4, 7, 0, 7, 3,
		// Top face
		3, 2, 6, 3, 6, 7,
		// Bottom face
		0, 1, 5, 0, 5, 4
	};
	//아래 몸체
	if (true)
	{
		GLfloat vertices[8][3] = {
			{-CAR_SIZE / 2,	0.0f,			-CAR_SIZE},	// Vertex 0
			{ CAR_SIZE / 2,	0.0f,			-CAR_SIZE},	// Vertex 1
			{ CAR_SIZE / 2,	CAR_SIZE / 2,	-CAR_SIZE},   // Vertex 2
			{-CAR_SIZE / 2,	CAR_SIZE / 2, -CAR_SIZE },  // Vertex 3
			{-CAR_SIZE / 2,	0.0f,			 CAR_SIZE },  // Vertex 4
			{ CAR_SIZE / 2,	0.0f,			 CAR_SIZE },  // Vertex 5
			{ CAR_SIZE / 2,	CAR_SIZE / 2,  CAR_SIZE },  // Vertex 6
			{-CAR_SIZE / 2,	CAR_SIZE / 2,  CAR_SIZE }   // Vertex 7
		};
		GLfloat CubeFigureWithNormals[TRI_COUNT * 3][6];
		// 정점 데이터와 법선 데이터 초기화
		for (int i = 0; i < TRI_COUNT * 3; i += 3)
		{
			GLfloat normal[3];
			bool reverse = (i / 6) % 2 == 0; // i가 6개씩 그룹화되었을 때 홀수 그룹은 뒷면
			calculateNormal(vertices[CubeIndices[i]], vertices[CubeIndices[i + 1]], vertices[CubeIndices[i + 2]], normal, reverse);
			for (int j = 0; j < 3; ++j)
			{ // 각 삼각형의 정점
				int idx = CubeIndices[i + j];
				CubeFigureWithNormals[i + j][0] = vertices[idx][0]; // x
				CubeFigureWithNormals[i + j][1] = vertices[idx][1]; // y
				CubeFigureWithNormals[i + j][2] = vertices[idx][2]; // z
				CubeFigureWithNormals[i + j][3] = normal[0];        // nx
				CubeFigureWithNormals[i + j][4] = normal[1];        // ny
				CubeFigureWithNormals[i + j][5] = normal[2];        // nz
			}
		}
		//아래 몸통 
		for (int j = 0; j < TRI_COUNT * 3; j++)
		{
			for (int k = 0; k < 6; k++)
			{
				Block[0][j][k] = CubeFigureWithNormals[j][k];
			}
		}
	}

	//윗 몸체(창문)
	if (true)
	{
		GLfloat vertices[8][3] = {
			{-CAR_SIZE / 3,	CAR_SIZE / 2,						-CAR_SIZE / 3 * 2},	// Vertex 0
			{ CAR_SIZE / 3,	CAR_SIZE / 2,						-CAR_SIZE / 3 * 2},	// Vertex 1
			{ CAR_SIZE / 3,	CAR_SIZE / 2 + CAR_SIZE / 3,	-CAR_SIZE / 3 * 2},   // Vertex 2
			{-CAR_SIZE / 3,	CAR_SIZE / 2 + CAR_SIZE / 3,	-CAR_SIZE / 3 * 2 },  // Vertex 3
			{-CAR_SIZE / 3,	CAR_SIZE / 2,						CAR_SIZE / 3 * 2 },  // Vertex 4
			{ CAR_SIZE / 3,	CAR_SIZE / 2,						CAR_SIZE / 3 * 2 },  // Vertex 5
			{ CAR_SIZE / 3,	CAR_SIZE / 2 + CAR_SIZE / 3,	CAR_SIZE / 3 * 2 },  // Vertex 6
			{-CAR_SIZE / 3,	CAR_SIZE / 2 + CAR_SIZE / 3,	CAR_SIZE / 3 * 2 }   // Vertex 7
		};
		GLfloat CubeFigureWithNormals[TRI_COUNT * 3][6];
		// 정점 데이터와 법선 데이터 초기화
		for (int i = 0; i < TRI_COUNT * 3; i += 3)
		{
			GLfloat normal[3];
			bool reverse = (i / 6) % 2 == 0; // i가 6개씩 그룹화되었을 때 홀수 그룹은 뒷면
			calculateNormal(vertices[CubeIndices[i]], vertices[CubeIndices[i + 1]], vertices[CubeIndices[i + 2]], normal, reverse);
			for (int j = 0; j < 3; ++j)
			{ // 각 삼각형의 정점
				int idx = CubeIndices[i + j];
				CubeFigureWithNormals[i + j][0] = vertices[idx][0]; // x
				CubeFigureWithNormals[i + j][1] = vertices[idx][1]; // y
				CubeFigureWithNormals[i + j][2] = vertices[idx][2]; // z
				CubeFigureWithNormals[i + j][3] = normal[0];        // nx
				CubeFigureWithNormals[i + j][4] = normal[1];        // ny
				CubeFigureWithNormals[i + j][5] = normal[2];        // nz
			}
		}
		for (int j = 0; j < TRI_COUNT * 3; j++)
		{
			for (int k = 0; k < 6; k++)
			{
				Block[1][j][k] = CubeFigureWithNormals[j][k];
			}
		}
	}

	//헤드라이트 1, 2 (정육면체)
	if (true)
	{
		GLfloat vertices[8][3] = {
			{-CAR_SIZE / 8,	-CAR_SIZE / 8, -CAR_SIZE / 8},	// Vertex 0
			{ CAR_SIZE / 8,	-CAR_SIZE / 8, -CAR_SIZE / 8},	// Vertex 1
			{ CAR_SIZE / 8,	CAR_SIZE / 8, -CAR_SIZE / 8},  // Vertex 2
			{-CAR_SIZE / 8,	CAR_SIZE / 8, -CAR_SIZE / 8},  // Vertex 3
			{-CAR_SIZE / 8,	-CAR_SIZE / 8, CAR_SIZE / 8},	// Vertex 4
			{ CAR_SIZE / 8,	-CAR_SIZE / 8, CAR_SIZE / 8},	// Vertex 5
			{ CAR_SIZE / 8,	CAR_SIZE / 8, CAR_SIZE / 8},	// Vertex 6
			{-CAR_SIZE / 8,	CAR_SIZE / 8, CAR_SIZE / 8}	// Vertex 7
		};
		GLfloat CubeFigureWithNormals[TRI_COUNT * 3][6];
		// 정점 데이터와 법선 데이터 초기화
		for (int i = 0; i < TRI_COUNT * 3; i += 3)
		{
			GLfloat normal[3];
			bool reverse = (i / 6) % 2 == 0; // i가 6개씩 그룹화되었을 때 홀수 그룹은 뒷면
			calculateNormal(vertices[CubeIndices[i]], vertices[CubeIndices[i + 1]], vertices[CubeIndices[i + 2]], normal, reverse);
			for (int j = 0; j < 3; ++j)
			{ // 각 삼각형의 정점
				int idx = CubeIndices[i + j];
				CubeFigureWithNormals[i + j][0] = vertices[idx][0]; // x
				CubeFigureWithNormals[i + j][1] = vertices[idx][1]; // y
				CubeFigureWithNormals[i + j][2] = vertices[idx][2]; // z
				CubeFigureWithNormals[i + j][3] = normal[0];        // nx
				CubeFigureWithNormals[i + j][4] = normal[1];        // ny
				CubeFigureWithNormals[i + j][5] = normal[2];        // nz
			}
		}
		for (int j = 0; j < TRI_COUNT * 3; j++)
		{
			for (int k = 0; k < 6; k++)
			{
				Block[2][j][k] = CubeFigureWithNormals[j][k];

				Block[3][j][k] = CubeFigureWithNormals[j][k];
			}
		}
	}

	//바퀴 사각형
	if (true)
	{
		GLfloat vertices[8][3] = {
				{-WHEEL_SIZE / 4,	-WHEEL_SIZE / 8,  -WHEEL_SIZE },		// Vertex 0
				{ WHEEL_SIZE / 4,	-WHEEL_SIZE / 8,  -WHEEL_SIZE },		// Vertex 1
				{ WHEEL_SIZE / 4,	 WHEEL_SIZE / 8, 	-WHEEL_SIZE },		// Vertex 2
				{-WHEEL_SIZE / 4,	 WHEEL_SIZE / 8, 	-WHEEL_SIZE },		// Vertex 3
				{-WHEEL_SIZE / 4,	-WHEEL_SIZE / 8,   WHEEL_SIZE },	// Vertex 4
				{ WHEEL_SIZE / 4,	-WHEEL_SIZE / 8,   WHEEL_SIZE },	// Vertex 5
				{ WHEEL_SIZE / 4,	 WHEEL_SIZE / 8, 	 WHEEL_SIZE },		// Vertex 6
				{-WHEEL_SIZE / 4,	 WHEEL_SIZE / 8, 	 WHEEL_SIZE }		// Vertex 7
		};
		GLfloat CubeFigureWithNormals[TRI_COUNT * 3][6];
		// 정점 데이터와 법선 데이터 초기화
		for (int i = 0; i < TRI_COUNT * 3; i += 3)
		{
			GLfloat normal[3];
			bool reverse = (i / 6) % 2 == 0; // i가 6개씩 그룹화되었을 때 홀수 그룹은 뒷면
			calculateNormal(vertices[CubeIndices[i]], vertices[CubeIndices[i + 1]], vertices[CubeIndices[i + 2]], normal, reverse);
			for (int j = 0; j < 3; ++j)
			{ // 각 삼각형의 정점
				int idx = CubeIndices[i + j];
				CubeFigureWithNormals[i + j][0] = vertices[idx][0]; // x
				CubeFigureWithNormals[i + j][1] = vertices[idx][1]; // y
				CubeFigureWithNormals[i + j][2] = vertices[idx][2]; // z
				CubeFigureWithNormals[i + j][3] = normal[0];        // nx
				CubeFigureWithNormals[i + j][4] = normal[1];        // ny
				CubeFigureWithNormals[i + j][5] = normal[2];        // nz
			}
		}
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < TRI_COUNT * 3; j++)
			{
				for (int k = 0; k < 6; k++)
				{
					wheel_rect[i][j][k] = CubeFigureWithNormals[j][k];
				}
			}
		}
	}
}

// 장애물 초기화
static void initObstacleCar()
{
	//큐브 데이터 초기화
	GLuint CubeIndices[] = {
		// Front face
		0, 1, 2, 0, 2, 3,
		// Back face
		4, 5, 6, 4, 6, 7,
		// Right face
		1, 5, 6, 1, 6, 2,
		// Left face
		0, 4, 7, 0, 7, 3,
		// Top face
		3, 2, 6, 3, 6, 7,
		// Bottom face
		0, 1, 5, 0, 5, 4
	};
	if (true)
	{
		GLfloat vertices[8][3] = {
			{-OBSTACLE_WIDTH,	0.0f,			-OBSTACLE_HEIGHT },	// Vertex 0
			{ OBSTACLE_WIDTH,	0.0f,			-OBSTACLE_HEIGHT },	// Vertex 1
			{ OBSTACLE_WIDTH,	CAR_SIZE,		-OBSTACLE_HEIGHT },   // Vertex 2
			{-OBSTACLE_WIDTH,	CAR_SIZE,		-OBSTACLE_HEIGHT },  // Vertex 3
			{-OBSTACLE_WIDTH,	0.0f,			 OBSTACLE_HEIGHT },  // Vertex 4
			{ OBSTACLE_WIDTH,	0.0f,			 OBSTACLE_HEIGHT },  // Vertex 5
			{ OBSTACLE_WIDTH,	CAR_SIZE,		 OBSTACLE_HEIGHT },  // Vertex 6
			{-OBSTACLE_WIDTH,	CAR_SIZE,		 OBSTACLE_HEIGHT }   // Vertex 7
		};
		GLfloat CubeFigureWithNormals[TRI_COUNT * 3][6];
		// 정점 데이터와 법선 데이터 초기화
		for (int i = 0; i < TRI_COUNT * 3; i += 3)
		{
			GLfloat normal[3];
			bool reverse = (i / 6) % 2 == 0; // i가 6개씩 그룹화되었을 때 홀수 그룹은 뒷면
			calculateNormal(vertices[CubeIndices[i]], vertices[CubeIndices[i + 1]], vertices[CubeIndices[i + 2]], normal, reverse);
			for (int j = 0; j < 3; ++j)
			{ // 각 삼각형의 정점
				int idx = CubeIndices[i + j];
				CubeFigureWithNormals[i + j][0] = vertices[idx][0]; // x
				CubeFigureWithNormals[i + j][1] = vertices[idx][1]; // y
				CubeFigureWithNormals[i + j][2] = vertices[idx][2]; // z
				CubeFigureWithNormals[i + j][3] = normal[0];        // nx
				CubeFigureWithNormals[i + j][4] = normal[1];        // ny
				CubeFigureWithNormals[i + j][5] = normal[2];        // nz
			}
		}
		for (int j = 0; j < TRI_COUNT * 3; j++)
		{
			for (int k = 0; k < 6; k++)
			{
				obstacle_car[j][k] = CubeFigureWithNormals[j][k];
			}
		}
	}
}






static void Mesh_LoadBuffers()
{
	glGenVertexArrays(10, vao);
	glGenBuffers(10, vbo);

	// 땅
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ground), ground, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);						// 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));		// 노말 속성
	glEnableVertexAttribArray(1);

	// 블록(차체)
	glBindVertexArray(vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Block), Block, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);						// 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));		// 노말 속성
	glEnableVertexAttribArray(1);

	// 핸들
	glBindVertexArray(vao[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(handle_rect), handle_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);						// 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));		// 노말 속성
	glEnableVertexAttribArray(1);

	// 벽
	glBindVertexArray(vao[3]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(walls), walls, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);						// 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));		// 노말 속성
	glEnableVertexAttribArray(1);

	// 바퀴
	glBindVertexArray(vao[4]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wheel_rect), wheel_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);						// 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));		// 노말 속성
	glEnableVertexAttribArray(1);

	// 도착지점 (주차 공간)
	glBindVertexArray(vao[5]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(finish_rect), finish_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);						// 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));		// 노말 속성
	glEnableVertexAttribArray(1);

	// 기어 데이터 초기화
	glBindVertexArray(vao[6]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gear_rect), gear_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);						// 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));		// 노말 속성
	glEnableVertexAttribArray(1);

	// 기어봉 데이터 초기화
	glBindVertexArray(vao[7]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gear_stick_rect), gear_stick_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);						// 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));		// 노말 속성
	glEnableVertexAttribArray(1);

	//장식용 주차공간
	glBindVertexArray(vao[8]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(finish_rect), finish_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);						// 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));		// 노말 속성
	glEnableVertexAttribArray(1);

	// 장애물 차
	glBindVertexArray(vao[9]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(obstacle_car), obstacle_car, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);						// 위치 속성
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));		// 노말 속성
	glEnableVertexAttribArray(1);

	// unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
	// glUniform3f(lightPosLocation, lightX, lightY, lightZ);
	// 
	// unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
	// glUniform3f(lightColorLocation, 1.0f, 1.0f, 1.0f);
	// 
	// unsigned int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	// glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);
	// 
	// unsigned int ablColorLocation = glGetUniformLocation(shaderProgramID, "ambientLight");
	// glUniform3f(ablColorLocation, light, light, light);
	// 
	// unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos");	//--- viewPos 값 전달: 카메라위치
	// glUniform3f(viewPosLocation, c_dx, c_dy, 0.0f);
}





void Mesh_InitAll()
{
	// 1. 메모리에 정점 데이터 배열 채우기
	initCar();
	initObstacleCar();

	// 2. GPU에 데이터 업로드 (VAO/VBO 생성)
	Mesh_LoadBuffers();
}