#define _CRT_SECURE_NO_WARNINGS
#include "shader.h"

#include <iostream>
#include <cstdlib> 
#include <cstdio>

GLuint shaderProgramID;

static GLchar* vertexSource, * fragmentSource;

static char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb");			// Open file for reading 
	if (!fptr)							// Return NULL on failure 
		return NULL;
	fseek(fptr, 0, SEEK_END);			// Seek to the end of the file 
	length = ftell(fptr);				// Find out how many bytes into the file we are 
	buf = (char*)malloc(length + 1);	// Allocate a buffer for the entire length of the file and a null terminator 
	fseek(fptr, 0, SEEK_SET);			// Go back to the beginning of the file 
	fread(buf, length, 1, fptr);		// Read the contents of the file in to the buffer 
	fclose(fptr);						// Close the file 
	buf[length] = 0;					// Null terminator 
	return buf;							// Return the buffer 
}

//static void make_vertexShaders()
//{
//	vertexSource = filetobuf("vertex.glsl");
//
//	//--- 버텍스 세이더 객체 만들기
//	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//
//	//--- 세이더 코드를 세이더 객체에 넣기
//	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
//
//	//--- 버텍스 세이더 컴파일하기
//	glCompileShader(vertexShader);
//
//	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
//	GLint result;
//	GLchar errorLog[512];
//	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
//	if (!result)
//	{
//		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
//		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
//		return;
//	}
//}
//
//static void make_fragmentShaders()
//{
//	fragmentSource = filetobuf("fragment.glsl");
//
//	//--- 프래그먼트 세이더 객체 만들기
//	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//
//	//--- 세이더 코드를 세이더 객체에 넣기
//	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
//
//	//--- 프래그먼트 세이더 컴파일
//	glCompileShader(fragmentShader);
//
//	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
//	GLint result;
//	GLchar errorLog[512];
//	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
//	if (!result)
//	{
//		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
//		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
//		return;
//	}
//}

void Shader_InitAll()
{
	// --- Vertex Shader ---
	vertexSource = filetobuf("vertex.glsl");
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
	glCompileShader(vertexShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}

	// --- Fragment Shader ---
	fragmentSource = filetobuf("fragment.glsl");
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}

	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	free(vertexSource);
	free(fragmentSource);

	glUseProgram(shaderProgramID);
}