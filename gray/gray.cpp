#include "../main.h"

void Init() {}

void Draw(HDC hdc, float timeElapsed) {
	int frame = (int)(timeElapsed / 0.75f) % 4;

	float bgColor = (frame < 2) ? (frame + 1) * 0.25f : (5 - frame) * 0.25f;
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(bgColor, bgColor, bgColor, 1.0f);

	if (frame % 2 == 0) {
		float color = (3 - frame) * 0.25f;
		glColor3f(color, color, color);

		float xs[] = { 0.0f, (float)screenWidth, (float)screenWidth, 0.0f };
		float ys[] = { 0.0f, 0.0f, (float)(screenHeight / 2), (float)(screenHeight / 2) };
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 4; i++)
			glVertex2f(xs[i], ys[i]);
		glEnd();
	}

	glFlush();
	SwapBuffers(hdc);
}
