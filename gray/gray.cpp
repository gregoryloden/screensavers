#include "gray.h"

LONG WINAPI ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_CREATE:
			SetupOpengl(hWnd);
			return 0;
		case WM_DESTROY:
			TeardownOpengl(hWnd);
			break;
		case WM_PAINT:
			Draw();
			return 0;
	}
	return DefScreenSaverProc(hWnd, message, wParam, lParam);
}

//Needed for scrnsave.lib
BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) { return FALSE; }

//Needed for scrnsave.lib
BOOL WINAPI RegisterDialogClasses(HANDLE hInst) { return TRUE; }

void SetupOpengl(HWND hWnd) {
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof pfd);
	pfd.nSize = sizeof pfd;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.cColorBits = 24;

	hdc = GetDC(hWnd);
	SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);

	hglrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hglrc);

	//Enable vsync
	((BOOL(WINAPI*)(int))wglGetProcAddress("wglSwapIntervalEXT"))(1);

	RECT rect;
	GetClientRect(hWnd, &rect);
	screenWidth = rect.right;
	screenHeight = rect.bottom;

	//"Window" pixels
	glViewport(0, 0, screenWidth, screenHeight);
	//"World view" pixels
	glOrtho(0, screenWidth, screenHeight, 0, -1, 1);

	QueryPerformanceFrequency(&ticksPerSecond);
	QueryPerformanceCounter(&startTime);
}

void TeardownOpengl(HWND hWnd) {
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);
	ReleaseDC(hWnd, hdc);
}

void Draw() {
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	float timeElapsed = (float)(currentTime.QuadPart - startTime.QuadPart) / (float)ticksPerSecond.QuadPart;
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
