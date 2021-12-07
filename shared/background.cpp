#include "background.h"
#include <wincodec.h>
#include <wincodecsdk.h>
#include "threading.cpp"

#undef RGB
#define doOrLoadFallbackImage(thing) if (FAILED(thing)) return LoadFallbackImage();
#define buildOrLoadFallbackImage(type, obj, construction) \
	type obj;\
	doOrLoadFallbackImage(construction)\
	Releaser<type> obj##_Releaser (obj);

void loadGlTexture(const GLvoid* pixels, GLsizei width, GLsizei height, GLenum format);
constexpr float sqrtConst(float f);
float equalDistributionDark(float dark);

struct RGB {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};
template <typename T> class Releaser {
public:
	T val;
	Releaser(T pVal) { val = pVal; }
	~Releaser() { val->Release(); }
};

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

	if (screenshot != nullptr) {
		HDC memorydc = CreateCompatibleDC(screendc);
		SelectObject(memorydc, screenshot);
		BitBlt(memorydc, 0, 0, screenshotWidth, screenshotHeight, screendc, 0, 0, SRCCOPY);
		loadGlTexture(screenshotBits, screenshotWidth, screenshotHeight, GL_BGR_EXT);
		DeleteDC(memorydc);
		DeleteObject(screenshot);
	} else
		loadGlTexture("\0\0\0", 1, 1, GL_RGB);
	ReleaseDC(nullptr, screendc);
}

void LoadDesktopBackground() {
	doOrLoadFallbackImage(CoInitialize(nullptr))

	buildOrLoadFallbackImage(
		IWICImagingFactory*,
		factory,
		CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))

	char path[MAX_PATH] = {};
	SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, path, 0);
	wchar_t wpath[sizeof(path) * 2] = {};
	swprintf_s(wpath, L"%S", path);

	buildOrLoadFallbackImage(
		IWICBitmapDecoder*,
		decoder,
		factory->CreateDecoderFromFilename(wpath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder))

	buildOrLoadFallbackImage(IWICBitmapFrameDecode*, frame, decoder->GetFrame(0, &frame))

	UINT imageWidth;
	UINT imageHeight;
	doOrLoadFallbackImage(frame->GetSize(&imageWidth, &imageHeight))

	IWICBitmapSource* convertedFrame;
	doOrLoadFallbackImage(WICConvertBitmapSource(GUID_WICPixelFormat24bppRGB, frame, &convertedFrame))

	RGB* image = new RGB[imageWidth * imageHeight];
	WICRect rect = { 0, 0, (INT)imageWidth, (INT)imageHeight };
	doOrLoadFallbackImage(
		convertedFrame->CopyPixels(&rect, imageWidth * 3, imageWidth * imageHeight * 3, reinterpret_cast<BYTE*>(image)))
	loadGlTexture(image, imageWidth, imageHeight, GL_RGB);
	CoUninitialize();
}

void LoadFallbackImage() {
	RGB* image = new RGB[screenWidth * screenHeight];

	//Represent hues as ranges, with each range indicating how to compute an individual hue
	//Each RGB component in the range computes its value by multiplying the position between hues by the component and possibly
	// adding 255
	//A hue's dark value is calculated similarly, averaging the three values and converting to be between 0 and 1
	struct HueRange {
		float rAdd;
		float rMul;
		float gAdd;
		float gMul;
		float bAdd;
		float bMul;
		float darkAdd;
		float darkMul;
	};
	HueRange hueTable[] = {
		//Hues start at magenta pointing up and ascend counterclockwise moving towards red
		{ 255, 0, 0, 0, 255, -255 }, //magenta -> red
		{ 255, 0, 0, 255, 0, 0 }, //red -> yellow
		{ 255, -255, 255, 0, 0, 0 }, //yellow -> green
		{ 0, 0, 255, 0, 0, 255 }, //green -> cyan
		{ 0, 0, 255, -255, 255, 0 }, //cyan -> blue
		{ 0, 255, 0, 0, 255, 0 } // blue -> magenta
	};
	for (int i = 0; i < 6; i++) {
		HueRange* hue = &hueTable[i];
		hue->darkAdd = 1 - (hue->rAdd + hue->gAdd + hue->bAdd) / (255 * 3);
		hue->darkMul = -(hue->rMul + hue->gMul + hue->bMul) / (255 * 3);
	}

	float halfWidth = screenWidth * 0.5f - 0.5f;
	float halfHeight = screenHeight * 0.5f - 0.5f;

	//Calculate dark for each x/y value that we'll pass through
	//Regardless of the point on screen, we always pick a dark based only on either the x or the y
	float* normalizedXDark = new float[screenWidth];
	float* normalizedYDark = new float[screenHeight];
	for (int i = 0; i < 2; i++) {
		float* normalizedDark = i == 0 ? normalizedXDark : normalizedYDark;
		int valueCount = i == 0 ? screenWidth : screenHeight;
		float halfValue = i == 0 ? halfWidth : halfHeight;
		for (int j = valueCount / 2; j < valueCount; j++) {
			float dark = equalDistributionDark(j / halfValue - 1);
			normalizedDark[j] = dark;
			normalizedDark[valueCount - j - 1] = dark;
		}
	}

	SplitToMultipleThreads([hueTable, halfWidth, halfHeight, normalizedXDark, normalizedYDark, image](int i, int threadCount) {
		for (int y = i; y < screenHeight; y += threadCount) {
			int pixelI = y * screenWidth;
			for (int x = 0; x < screenWidth; x++) {
				float xDist = x / halfWidth - 1;
				float yDist = y / halfHeight - 1;
				float xDark = fabsf(xDist);
				float yDark = fabsf(yDist);
				//Calculate angle (hue between 0 and 6) based on position around square
				float angle;
				float dark;
				if (yDark > xDark) {
					angle = 0.75f * (yDist > 0 ? 4 + xDist / yDark : xDist < 0 ? -xDist / yDark : 8 - xDist / yDark);
					dark = normalizedYDark[y];
				} else {
					angle = 0.75f * (xDist < 0 ? 2 + yDist / xDark : 6 - yDist / xDark);
					dark = normalizedXDark[x];
				}
				float hueIndex;
				float huePosition = modff(angle, &hueIndex);
				const HueRange* hue = &(hueTable[(int)hueIndex]);
				float hueDark = hue->darkAdd + hue->darkMul * huePosition;
				RGB* pixel = &image[pixelI];
				//Dark colors: scale the colors by the distance to the edge of the screen
				if (dark > hueDark) {
					float bright = (1 - dark) / (1 - hueDark);
					pixel->r = (unsigned char)((hue->rAdd + hue->rMul * huePosition) * bright);
					pixel->g = (unsigned char)((hue->gAdd + hue->gMul * huePosition) * bright);
					pixel->b = (unsigned char)((hue->bAdd + hue->bMul * huePosition) * bright);
				//Bright colors: scale the negative of the color by the distance to the center of the screen, then subtract it
				// from white
				} else {
					float scaledDark = dark / hueDark;
					pixel->r = (unsigned char)(255 - ((255 - hue->rAdd - hue->rMul * huePosition) * scaledDark));
					pixel->g = (unsigned char)(255 - ((255 - hue->gAdd - hue->gMul * huePosition) * scaledDark));
					pixel->b = (unsigned char)(255 - ((255 - hue->bAdd - hue->bMul * huePosition) * scaledDark));
				}
				pixelI++;
			}
		}
	});
	loadGlTexture(image, screenWidth, screenHeight, GL_RGB);
	delete[] image;
	delete[] normalizedYDark;
	delete[] normalizedXDark;
}

void loadGlTexture(const GLvoid* pixels, GLsizei width, GLsizei height, GLenum format) {
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
}

constexpr float sqrtConst(float f) {
	//Newton's method
	float lastGuess = 0;
	float guess = f / 2;
	while (guess != lastGuess) {
		lastGuess = guess;
		guess = (guess + f / guess) / 2;
	}
	return guess;
}

float equalDistributionDark(float dark) {
	//Recalculate dark so that every dark value has equal representation in terms of total pixels (theoretically), based on the
	// distibution of values
	//Begin by converting dark from distance-from-center to the total factor of all colors darker than this darkness that will
	// be displayed
	dark *= dark;
	//For a given darkness value 0 <= x <= 1 which is the sum of three darkness values each between 0 and 1/3, the distribution
	// of those values is:
	//  0 <= x <= 1/3: 27/2 * x^2
	//1/3 <= x <= 2/3: -27x^2 + 27x - 9/2          = 9/4 - 27 * (x - 1/2)^2
	//2/3 <= x <= 1  : 27/2 * x^2 - 27x + 27/2     = 27/2 * (x - 1)^2
	//and the area under the graph is 1.
	//For a given darkness value 0 <= x <= 1, the total factor of all colors brighter than it that will be displayed is:
	//  0 <= x <= 1/3: 9/2 * x^3
	//1/3 <= x <= 2/3: -9x^3 + 27/2 * x^2 - 9/2 * x + 1/2          = -9 * x * (x - 1/2) * (x - 1) + 1/2
	//2/3 <= x <= 1  : 9/2 * x^3 - 27/2 * x^2 + 27/2 * x - 7/2     = 9/2 * (x - 1)^3 + 1
	//To find darkness y given x = the total factor of all colors brighter than it that will be displayed, is:
	//  0 <= x <= 1/6: cbrt(2x/9)
	//1/6 <= x <= 5/6: (inverse of above equation, see below)
	//5/6 <= x <= 1  : 1 - cbrt((2-2x)/9)
	if (dark <= (1.0f / 6.0f))
		return cbrtf(dark * 2 / 9);
	else if (dark >= (5.0f / 6.0f))
		return 1 - cbrtf((2 - dark * 2) / 9);

	//For 1/6 <= x <= 5/6, we need to derive the cubic formula
	// -- ay^3 + by^2 + cy + d = x
	// -- a != 0
	constexpr float cubicA = -9;
	constexpr float cubicB = 27 / 2.0f;
	constexpr float cubicC = -9 / 2.0f;
	constexpr float cubicD = 1 / 2.0f;
	//Convert it to a "depressed cubic" by replacing y with (z - b/(3a))
	// -- y = z - b/(3a)
	// -- y^2 = z^2 - 2bz/(3a) + b^2/(9a^2)
	// -- y^3 = z^3 - bz^2/a + b^2z/(3a^2) - b^3/(27a^3)
	//Sub in values for y terms
	// -- x = az^3 - bz^2 + b^2z/(3a) - b^3/(27a^2) + bz^2 - 2b^2z/(3a) + b^3/(9a^2) + cz - bc/(3a) + d
	//      = az^3 - b^2z/(3a) + cz + 2b^3/(27a^2) - bc/(3a) + d
	//      = az^3 + (c - b^2/(3a))z + 2b^3/(27a^2) - bc/(3a) + d
	// -- z^3 + (c/a - b^2/(3a^2))z + 2b^3/(27a^3) - bc/(3a^2) + d/a - x/a = 0
	//Assign values p and q to the 2nd and 3rd terms of the formula and sub them out
	// -- p = c/a - b^2/(3a^2)
	// -- q = 2b^3/(27a^3) - bc/(3a^2) + d/a - x/a
	// -- z^3 + pz + q = 0
	constexpr float cubicP = (3 * cubicA * cubicC - cubicB * cubicB) / (3 * cubicA * cubicA);
	constexpr float cubicQConstant =
		(2 * cubicB * cubicB * cubicB - 9 * cubicA * cubicB * cubicC) / (27 * cubicA * cubicA * cubicA)
		+ cubicD / cubicA;
	float cubicQ = cubicQConstant - dark / cubicA;
	//Cubic functions of the form a^3 = b have 3 roots:
	// -- a = cbrt(b)
	// -- a = cbrt(b)(-1 + sqrt(3)i)/2
	// -- a = cbrt(b)(-1 - sqrt(3)i)/2
	//If p = 0, replace a with z and b with q for those (in this case it's not)
	//Otherwise, perform "Vieta's substitution" by replacing z with (w - p/(3w))
	// -- z = w - p/(3w)
	// -- w != 0
	// -- y = w - p/(3w) - b/(3a)
	// -- 0 = w^3 - pw + p^2/(3w) - p^3/(27w^3) + pw - p^2/(3w) + q
	//      = w^3 - p^3/(27w^3) + q
	//      = (w^3)^2 + q(w^3) - p^3/27
	// -- w^3 = -q/2 +- sqrt(q^2/4 + p^3/27)
	//Assign values r and s to simplify the equation
	// -- r = -q/2
	// -- s = q^2/4 + p^3/27 (= r^2 + p^3/27)
	// -- w^3 = r +- sqrt(s)
	float cubicR = -cubicQ / 2;
	float cubicRSquared = cubicR * cubicR; // for convenience
	constexpr float cubicPCubedOver27 = cubicP * cubicP * cubicP / 27;
	float cubicS = cubicRSquared + cubicPCubedOver27;
	//If s were >=0, we could stop here and sub in w in the y equation, but for the values we care about, s < 0
	//Assign value t to simplify the equation
	// -- t = sqrt(abs(s))
	// -- w^3 = r +- ti
	float cubicT = sqrtf(-cubicS);
	//Combining square roots with cube roots, w has six roots:
	// -- w = cbrt(r + ti)
	// -- w = cbrt(r - ti)
	// -- w = cbrt(r + ti)(-1 + sqrt(3)i)/2
	// -- w = cbrt(r - ti)(-1 + sqrt(3)i)/2
	// -- w = cbrt(r + ti)(-1 - sqrt(3)i)/2
	// -- w = cbrt(r - ti)(-1 - sqrt(3)i)/2
	//External testing has determined that the equation we should use is #5:
	// -- w = cbrt(r + ti)(-1 - sqrt(3)i)/2
	//Find the cube root of a complex number
	//We can think of a complex number as coordinates on a graph, x + yi
	//Cubing a number cubes the distance from the origin and triples the angle from the +x axis
	//Cube-rooting a number cube-roots the distance and thirds the angle
	//Assign u to the distance and v to the angle
	// -- u = cbrt(sqrt(r^2 + t^2))
	//      = cbrt(sqrt(r^2 + abs(s)))
	// -- v = arctan(t over r) / 3
	float cubicU = cbrtf(sqrtf(cubicRSquared - cubicS));
	float cubicV = atan2f(cubicT, cubicR) / 3;
	//Now we can sub these back in for w and multiply it by the other part
	// -- w = u(cos(v) + sin(v)i)(-1 - sqrt(3)i)/2
	//      = u(-cos(v) + sqrt(3)sin(v) - sin(v)i - sqrt(3)cos(v)i)/2
	//      = u(sqrt(3)sin(v) - cos(v))/2 - u(sin(v) + sqrt(3)cos(v))i/2
	//Rewrite the real and imaginary parts as wR and wI
	// -- wR = u(sqrt(3)sin(v) - cos(v))/2
	// -- wI = -u(sin(v) + sqrt(3)cos(v))/2
	// -- w = wR + wIi
	//Sub in w for the y equation
	// -- y = wR + wIi - p/(3(wR + wIi)) - b/(3a)
	//Division of complex numbers works like this:
	// -- a + bi = (c + di)/(e + fi)
	//           = (c + di)(e - fi)/((e + fi)(e - fi))
	//           = (ce + dei - cfi + df)/(e^2 + efi - efi + f^2)
	//           = (ce + df + (de - cf)i)/(e^2 + f^2)
	//Divide (p/3 + 0i) by (wR + wIi) by subbing in results
	// -- p/(3(wR + wIi)) = p(wR - wIi)/(3(wR^2 + wI^2))
	// -- y = wR + wIi - p(wR - wIi)/(3(wR^2 + wI^2)) - b/(3a)
	//Expand out wR and wIi in the denominator
	// -- wR^2 = u^2(sqrt(3)sin(v) - cos(v))(sqrt(3)sin(v) - cos(v))/4
	//         = u^2(3sin(v)^2 - 2sqrt(3)sin(v)cos(v) + cos(v)^2)/4
	// -- wI^2 = u^2(sin(v) + sqrt(3)cos(v))(sin(v) + sqrt(3)cos(v))/4
	//         = u^2(sin(v)^2 + 2sqrt(3)sin(v)cos(v) + 3cos(v)^2)/4
	// -- wR^2 + wI^2 = u^2(3sin(v)^2 - 2sqrt(3)sin(v)cos(v) + cos(v)^2 + sin(v)^2 + 2sqrt(3)sin(v)cos(v) + 3cos(v)^2)/4
	//                = u^2(4sin(v)^2 + 4cos(v)^2)/4
	//                = u^2
	// -- p(wR - wIi)/(3(wR^2 + wI^2)) = p(wR - wIi)/(3u^2)
	// -- y = wR + wIi - p(wR - wIi)/(3u^2) - b/(3a)
	//      = wR + wIi - pwR/(3u^2) + pwIi/(3u^2) - b/(3a)
	//      = (1 - p/(3u^2))wR + (1 + p/(3u^2))wIi - b/(3a)
	//**** We could stop here and just drop the imaginary part (which will be 0), but let's just double check ****
	//Simplify the imaginary part
	// -- (1 + p/(3u^2))wIi = -(1 + p/(3u^2))u(sin(v) + sqrt(3)cos(v))i/2
	//                      = -(u + p/(3u))(sin(v) + sqrt(3)cos(v))/2
	// -- u + p/(3u) = cbrt(sqrt(r^2 + abs(s)) + p/(3cbrt(sqrt(r^2 + t^2)))
	//               = (3cbrt(r^2 + abs(s)) + p)/(3cbrt(sqrt(r^2 + t^2)))
	//Remember that s < 0 for the values we care about
	// -- 3cbrt(r^2 + abs(s)) + p = 3cbrt(r^2 - s) + p
	//                            = 3cbrt(q^2/4 - q^2/4 - p^3/27)) + p
	//                            = 3cbrt(-p^3/27)) + p
	//                            = 0
	// -- u + p/(3u) = 0
	// -- (1 + p/(3u^2))wIi = 0
	// -- y = (1 - p/(3u^2))wR - b/(3a)
	//      = (u/2 - p/(6u))(sqrt(3)sin(v) - cos(v)) - b/(3a)
	constexpr float sqrt3 = sqrtConst(3);
	constexpr float cubicBOver3A = cubicB / (3 * cubicA);
	constexpr float cubicPOver6 = cubicP / 6;
	return (cubicU / 2 - cubicPOver6 / cubicU) * (sqrt3 * sinf(cubicV) - cosf(cubicV)) - cubicBOver3A;
}
