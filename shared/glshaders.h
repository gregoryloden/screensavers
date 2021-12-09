#include "glextensions.h"

declareGlFunction(GLuint, (GLenum), glCreateShader);
declareGlFunction(void, (GLuint, GLsizei, const char**, const GLint*), glShaderSource);
declareGlFunction(void, (GLuint), glCompileShader);
declareGlFunction(GLuint, (), glCreateProgram);
declareGlFunction(void, (GLuint, GLuint), glAttachShader);
declareGlFunction(void, (GLuint), glLinkProgram);
declareGlFunction(void, (GLuint), glUseProgram);
declareGlFunction(GLuint, (GLuint, const char*), glGetUniformLocation);
declareGlFunction(void, (GLuint, GLfloat), glUniform1f);

#define GL_FRAGMENT_SHADER 0x8B30

void InitGlShaderFunctions();
