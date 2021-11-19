#include "../main.h"
#include <cmath>
#include "../shared/screenshot.h"

const float period = 4.0f;
const float halfPeriod = period / 2;

void Init() {
	LoadFullOrPreviewScreenshot();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
}

void Draw(HDC hdc, float timeElapsed) {
	float frame = fmod(timeElapsed, period);

	glClear(GL_COLOR_BUFFER_BIT);

	float xs[] = { 0.0f, 1.0f, 1.0f, 0.0f };
	float ys[] = { 0.0f, 0.0f, 1.0f, 1.0f };

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 4; i++) {
		glTexCoord2f(xs[i], ys[i]);
		glVertex2f(xs[i] * (float)screenWidth, ys[i] * (float)screenHeight);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);

	float val = (frame < halfPeriod ? frame : period - frame) / halfPeriod;
	glColor4f(val, val, val, val);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 4; i++)
		glVertex2f(xs[i] * (float)screenWidth, ys[i] * (float)screenHeight);
	glEnd();

	glFlush();
	SwapBuffers(hdc);
}
