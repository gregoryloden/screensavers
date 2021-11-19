#include "../main.h"
#include <cmath>

const float period = 4.0f;
const float halfPeriod = period / 2;

void Init() {
	void* screenshotBits;
	BITMAPINFO info = {};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = screenWidth;
	info.bmiHeader.biHeight = -screenHeight;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 24;
	info.bmiHeader.biCompression = BI_RGB;
	HDC screendc = GetDC(nullptr);
	HBITMAP screenshot = CreateDIBSection(screendc, &info, DIB_RGB_COLORS, &screenshotBits, nullptr, 0);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	if (screenshot != nullptr) {
		HDC memorydc = CreateCompatibleDC(screendc);
		SelectObject(memorydc, screenshot);
		BitBlt(memorydc, 0, 0, screenWidth, screenHeight, screendc, 0, 0, SRCCOPY);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, screenshotBits);
		DeleteDC(memorydc);
		DeleteObject(screenshot);
	} else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, "\0\0\0");
	ReleaseDC(nullptr, screendc);
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
