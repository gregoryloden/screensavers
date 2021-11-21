#include <windows.h>
#include <gl/gl.h>

#define declareGlFunction(returnType, params, name) \
	typedef returnType(WINAPI* name##_Type)params;\
	extern name##_Type name;
#define declareGlFunctionSource(name) name##_Type name
#define assignGlFunction(name) name = (name##_Type)wglGetProcAddress(#name);
