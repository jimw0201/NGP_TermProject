#ifndef MESH_H
#define MESH_H

#include <gl/glew.h>

extern GLuint vao[20], vbo[20];

extern const float fheight;
extern const float fy;

extern const float HANDLE_SIZE;
extern const int TRI_COUNT;
extern const float CAR_SIZE;
extern const float OBSTACLE_WIDTH;
extern const float OBSTACLE_HEIGHT;
extern const float WHEEL_SIZE;
extern const float FINISH_SIZE;
extern const float GROUND_SIZE;
extern const float WALL_THICKNESS;

void Mesh_InitAll();

#endif