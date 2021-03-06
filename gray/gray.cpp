#include "../main.h"

const float period = 3;
const float quarterPeriod = period / 4;

void Draw(HDC hdc, float timeElapsed) {
	int frame = (int)(timeElapsed / quarterPeriod) % 4;

	float bgColor = (frame < 2) ? (frame + 1) * 0.25f : (5 - frame) * 0.25f;
	glClearColor(bgColor, bgColor, bgColor, 1);

	if (frame % 2 == 0) {
		float color = (3 - frame) * 0.25f;
		glColor3f(color, color, color);

		float xs[] = { 0, (float)screenWidth, (float)screenWidth, 0 };
		float ys[] = { 0, 0, (float)(screenHeight / 2), (float)(screenHeight / 2) };
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 4; i++)
			glVertex2f(xs[i], ys[i]);
		glEnd();
	}
}

void Init() {}
BOOL DialogInit(HWND hDlg) { return FALSE; }
BOOL DialogCommand(HWND hDlg, WORD command) { return FALSE; }
