#ifdef _WIN32

#include <Nexus/Window.h>
#include <stdio.h>

#include <windows.h>

#define NX_WIN32_WINDOW_CLASS_NAME "nxWindowClass"

typedef struct {
	nxWindowCloseCallback onClose;
	nxWindowSizeCallback onResize;
	nxWindowPosCallback onMove;
	nxWindowFocusCallback onFocusChanged;
	uint8_t isClosed;
} win32WindowData;

typedef struct {
	win32WindowData data;
	HWND hwnd;
	uint8_t used;
} win32Window;

win32Window gWindows[NX_MAX_WINDOWS] = {};

uint8_t gIsWindowClassInitialized = NX_FALSE;

LRESULT windowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	nxWindow handle = GetWindowLongA(window, GWLP_USERDATA);
	if (message == WM_CLOSE) {
		gWindows[handle].data.isClosed = NX_TRUE;
	} 

	switch (message) {
		case WM_CLOSE: {
			if (gWindows[handle].data.onClose == NULL) break;
			gWindows[handle].data.onClose(handle);
			break;
		}
		case WM_SIZE: {
			if (gWindows[handle].data.onResize == NULL) break;
			gWindows[handle].data.onResize(handle, LOWORD(lparam), HIWORD(lparam));
			break;
		}
		case WM_MOVE: {
			if (gWindows[handle].data.onMove == NULL) break;
			gWindows[handle].data.onMove(handle, (uint16_t)LOWORD(lparam), (uint16_t)HIWORD(lparam));
			break;
		}
		case WM_SETFOCUS: {
			if (gWindows[handle].data.onFocusChanged == NULL) break;
			gWindows[handle].data.onFocusChanged(handle, NX_TRUE);
			break;
		}
		case WM_KILLFOCUS: {
			if (gWindows[handle].data.onFocusChanged == NULL) break;
			gWindows[handle].data.onFocusChanged(handle, NX_FALSE);
			break;
		}
	}

	return DefWindowProcA(window, message, wparam, lparam);
}

nxWindow nxCreateWindow(const char *title, uint32_t width, uint32_t height) {
	if (!gIsWindowClassInitialized) {
		WNDCLASSA wc = {};
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.lpfnWndProc = windowProc;
		wc.lpszClassName = NX_WIN32_WINDOW_CLASS_NAME;

		RegisterClassA(&wc);

		gIsWindowClassInitialized = NX_TRUE;	
	}

	nxWindow window = NX_INVALID_HANDLE;
	for (uint32_t i = 0; i < NX_MAX_WINDOWS; i++) {
		if (gWindows[i].used == NX_FALSE) {
			gWindows[i].used = NX_TRUE;
			window = i;
			break;
		}
	}

	gWindows[window].hwnd = CreateWindowA(
		NX_WIN32_WINDOW_CLASS_NAME,
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL, NULL, NULL, NULL
	);

	NX_ASSERT(gWindows[window].hwnd != NULL)

	SetWindowLongA(gWindows[window].hwnd, GWLP_USERDATA, window);

	return window;
}

void nxShowWindow(nxWindow window) {
	ShowWindow(gWindows[window].hwnd, SW_SHOWNORMAL);
}

void nxMaximizeWindow(nxWindow window) {
	ShowWindow(gWindows[window].hwnd, SW_MAXIMIZE);
}

void nxUpdateWindow(nxWindow window) {
	MSG message = {};
	while (PeekMessageA(&message, gWindows[window].hwnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}
}

uint8_t nxIsWindowClosed(nxWindow window) {
	return gWindows[window].data.isClosed;
}

void nxSetWindowCloseCallback(nxWindow window, nxWindowCloseCallback callback) {
	gWindows[window].data.onClose = callback;
}

void nxSetWindowSizeCallback(nxWindow window, nxWindowSizeCallback callback) {
	gWindows[window].data.onResize = callback;
}

void nxSetWindowPosCallback(nxWindow window, nxWindowPosCallback callback) {
	gWindows[window].data.onMove = callback;
}

void nxSetWindowFocusCallback(nxWindow window, nxWindowFocusCallback callback) {
	gWindows[window].data.onFocusChanged = callback;
}

void nxCloseWindow(nxWindow window) {
	CloseWindow(gWindows[window].hwnd);
}

void *nxGetWindowNativeHandle(nxWindow window) {
	return (void*)gWindows[window].hwnd;
}

void nxDestroyWindow(nxWindow *window) {
	DestroyWindow(gWindows[*window].hwnd);
	memset(&gWindows[*window], 0, sizeof(win32Window));
	gWindows[*window].used = NX_FALSE;
	*window = NX_INVALID_HANDLE;
}


#endif // _WIN32