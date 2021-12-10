#include "../main.h"
#include <cmath>
#include "resource.h"
#include "../shared/background.h"
#include "../shared/glshaders.h"

const float period = 6;
const float halfPeriod = period / 2;

GLuint frameUniform;

void Init() {
	InitGlShaderFunctions();

	LoadDesktopBackground();

	HRSRC fragmentShaderInfo = FindResource(nullptr, MAKEINTRESOURCE(IDR_FLASHING_SHADER), RT_RCDATA);
	if (fragmentShaderInfo == nullptr)
		return;
	HGLOBAL fragmentShaderData = LoadResource(nullptr, fragmentShaderInfo);
	if (fragmentShaderData == nullptr)
		return;
	int fragmentShaderLength = SizeofResource(nullptr, fragmentShaderInfo);
	const char* fragmentShaderSource = (const char*)LockResource(fragmentShaderData);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, &fragmentShaderLength);
	glCompileShader(fragmentShader);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	frameUniform = glGetUniformLocation(shaderProgram, "frame");
}

void Draw(HDC hdc, float timeElapsed) {
	float frame = fmod(timeElapsed, period) / halfPeriod;

	float xs[] = { 0, 1, 1, 0 };
	float ys[] = { 0, 0, 1, 1 };

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 4; i++) {
		glTexCoord2f(xs[i], ys[i]);
		glVertex2f(xs[i] * (float)screenWidth, ys[i] * (float)screenHeight);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glUniform1f(frameUniform, frame);
}
