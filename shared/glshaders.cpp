#include "glshaders.h"

declareGlFunctionSource(glCreateShader);
declareGlFunctionSource(glShaderSource);
declareGlFunctionSource(glCompileShader);
declareGlFunctionSource(glCreateProgram);
declareGlFunctionSource(glAttachShader);
declareGlFunctionSource(glLinkProgram);
declareGlFunctionSource(glUseProgram);
declareGlFunctionSource(glGetUniformLocation);
declareGlFunctionSource(glUniform1f);

void InitGlShaderFunctions() {
	assignGlFunction(glCreateShader);
	assignGlFunction(glShaderSource);
	assignGlFunction(glCompileShader);
	assignGlFunction(glCreateProgram);
	assignGlFunction(glAttachShader);
	assignGlFunction(glLinkProgram);
	assignGlFunction(glUseProgram);
	assignGlFunction(glGetUniformLocation);
	assignGlFunction(glUniform1f);
}
