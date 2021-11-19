#include "screenshot.h"
#include "../main.h"

void LoadFullOrPreviewScreenshot() {
	HDC screendc = GetDC(nullptr);
	int screenshotWidth = GetDeviceCaps(screendc, HORZRES);
	int screenshotHeight = GetDeviceCaps(screendc, VERTRES);
	if (screenWidth > screenshotWidth || screenHeight > screenshotHeight) {
		screenshotWidth = screenWidth;
		screenshotHeight = screenHeight;
	}
	void* screenshotBits;
	BITMAPINFO info = {};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = screenshotWidth;
	info.bmiHeader.biHeight = -screenshotHeight;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 24;
	info.bmiHeader.biCompression = BI_RGB;
	HBITMAP screenshot = CreateDIBSection(screendc, &info, DIB_RGB_COLORS, &screenshotBits, nullptr, 0);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	if (screenshot != nullptr) {
		HDC memorydc = CreateCompatibleDC(screendc);
		SelectObject(memorydc, screenshot);
		BitBlt(memorydc, 0, 0, screenshotWidth, screenshotHeight, screendc, 0, 0, SRCCOPY);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB, screenshotWidth, screenshotHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, screenshotBits);
		DeleteDC(memorydc);
		DeleteObject(screenshot);
	} else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, "\0\0\0");
	ReleaseDC(nullptr, screendc);
}
