#include "../main.h"
#include <cmath>
#include "../shared/screenshot.h"

const float period = 4;
const float halfPeriod = period / 2;

void Init() {
	LoadFullOrPreviewScreenshot();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
}

void Draw(HDC hdc, float timeElapsed) {
	float frame = fmod(timeElapsed, period);
	frame = (frame < halfPeriod ? frame : period - frame) / halfPeriod;

	glClear(GL_COLOR_BUFFER_BIT);

	float xs[] = { 0, 1, 1, 0 };
	float ys[] = { 0, 0, 1, 1 };

	glColor4f(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 4; i++) {
		glTexCoord2f(xs[i], ys[i]);
		glVertex2f(xs[i] * (float)screenWidth, ys[i] * (float)screenHeight);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glColor4f(frame, frame, frame, frame);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 4; i++)
		glVertex2f(xs[i] * (float)screenWidth, ys[i] * (float)screenHeight);
	glEnd();
}
