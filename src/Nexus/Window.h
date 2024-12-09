#ifndef __NX_WINDOW_H__
#define __NX_WINDOW_H__

#include "Core.h"

#define NX_MAX_WINDOWS 16

typedef nxHandle nxWindow;

typedef void(*nxWindowCloseCallback)(nxWindow window);
typedef void(*nxWindowSizeCallback)(nxWindow window, uint32_t width, uint32_t height);
typedef void(*nxWindowPosCallback)(nxWindow window, uint32_t x, uint32_t y);
typedef void(*nxWindowFocusCallback)(nxWindow window, uint8_t focused);

nxWindow nxCreateWindow(const char *title, uint32_t width, uint32_t height);
void nxShowWindow(nxWindow window);
void nxMaximizeWindow(nxWindow window);
void nxUpdateWindow(nxWindow window);
uint8_t nxIsWindowClosed(nxWindow window);
void nxSetWindowCloseCallback(nxWindow window, nxWindowCloseCallback callback);
void nxSetWindowSizeCallback(nxWindow window, nxWindowSizeCallback callback);
void nxSetWindowPosCallback(nxWindow window, nxWindowPosCallback callback);
void nxSetWindowFocusCallback(nxWindow window, nxWindowFocusCallback callback);
void nxCloseWindow(nxWindow window);
void *nxGetWindowNativeHandle(nxWindow window);
void nxDestroyWindow(nxWindow *window);

#endif // __NX_WINDOW_H__