#ifndef RENDERER_H
#define RENDERER_H

#include <gl/glew.h>
#include <gl/freeglut.h>

extern bool isProspect; 
extern bool isCull;     

void drawScene();
void Reshape(int w, int h);

#endif