#include "../main.h"
#include <cmath>
#include "resource.h"
#include "../shared/background.h"
#include "../shared/glshaders.h"
#include "../shared/registry.h"

void initDialogControls(HWND hDlg);
void loadRegistrySettings();
void saveRegistrySettings();

const float period = 6;
const float halfPeriod = period / 2;
const char registryAscendingName[] = "Ascending";
const DWORD registryAscending = 0;
const DWORD registryDescending = 1;

DWORD background = backgroundTypeWallpaper;
bool ascending = true;
GLuint frameUniform;

void Init() {
	InitGlShaderFunctions();

	loadRegistrySettings();
	LoadAppropriateBackground(background);

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
	if (!ascending)
		frame = period - frame;

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

BOOL DialogInit(HWND hDlg) {
	loadRegistrySettings();
	initDialogControls(hDlg);
	return TRUE;
}

BOOL DialogCommand(HWND hDlg, WORD command) {
	switch (command) {
		case IDC_WALLPAPER: background = backgroundTypeWallpaper; return TRUE;
		case IDC_COLORS: background = backgroundTypeColors; return TRUE;
		case IDC_SCREENSHOT: background = backgroundTypeScreenshot; return TRUE;
		case IDC_ASCENDING: ascending = true; return TRUE;
		case IDC_DESCENDING: ascending = false; return TRUE;
		case IDC_DEFAULTS_BUTTON:
			background = backgroundTypeWallpaper;
			ascending = true;
			initDialogControls(hDlg);
			return TRUE;
		case IDOK:
			saveRegistrySettings();
			return TRUE;
	}
	return FALSE;
}

void initDialogControls(HWND hDlg) {
	const int backgroundRadioButtons[] = { IDC_WALLPAPER, IDC_COLORS, IDC_SCREENSHOT };
	CheckRadioButton(hDlg, IDC_WALLPAPER, IDC_SCREENSHOT, backgroundRadioButtons[background]);
	CheckRadioButton(hDlg, IDC_ASCENDING, IDC_DESCENDING, ascending ? IDC_ASCENDING : IDC_DESCENDING);
}

void loadRegistrySettings() {
	background = LoadRegistryBackground();
	ascending = LoadRegistryDword(registryAscendingName) != registryDescending;
}

void saveRegistrySettings() {
	DWORD registryAscendingValue = ascending ? registryAscending : registryDescending;
	bool anySettingsInRegistry = SaveRegistryBackground(background);
	anySettingsInRegistry = SaveRegistryDword(registryAscendingName, registryAscendingValue) || anySettingsInRegistry;
	if (!anySettingsInRegistry)
		DeleteRegistryFolders();
}
