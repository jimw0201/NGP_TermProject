#ifndef INPUT_HANDLE_H
#define INPUT_HANDLE_H

#include <gl/freeglut.h>


void Input_Init();
void Input_ResetHandle();

float Input_GetCameraDX();
float Input_GetCameraDY();
float Input_GetCameraDZ();
float Input_GetCameraRotateY();
float Input_GetHandleRotation();
bool  Input_IsMouseOnHandle();

void Input_UpdateHandleReturn();

void Keyboard(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);
void MouseButton(int button, int state, int x, int y);
void MouseMotion(int x, int y);

#endif