#ifdef _WIN32

#include "win32_window.hpp"

#define NX_WIN32_WINDOW_CLASS_NAME "nx_window_class"

namespace nexus {

	namespace windows {

		static bool window_class_initialized = false;

		static LRESULT window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
			win32_window_data *data = (win32_window_data*)(void*)GetWindowLongPtrA(window, GWLP_USERDATA);
			if (data == nullptr) return DefWindowProcA(window, message, wparam, lparam);

			switch (message) {
				case WM_CLOSE: {
					data->is_closed = true;
					if (!data->on_close) break;
					
					data->on_close();
					break;
				}
				case WM_SIZE: {
					if (!data->on_resize) break;
					data->on_resize(LOWORD(lparam), HIWORD(lparam));
					break;
				}
				case WM_MOVE: {
					if (!data->on_move) break;
					data->on_move((int16_t)LOWORD(lparam), (int16_t)HIWORD(lparam));
					break;
				}
				case WM_SETFOCUS: {
					if (!data->on_focus_changed) break;
					data->on_focus_changed(true);
					break;
				}
				case WM_KILLFOCUS: {
					if (!data->on_focus_changed) break;
					data->on_focus_changed(false);
					break;
				}
			}
			return DefWindowProcA(window, message, wparam, lparam);
		}

		win32_window::win32_window(const std::string &title, uint32_t width, uint32_t height) {
			if (!window_class_initialized) {
				WNDCLASSA wc = {};
				wc.hCursor = LoadCursor(NULL, IDC_ARROW);
				wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
				wc.lpfnWndProc = window_proc;
				wc.lpszClassName = NX_WIN32_WINDOW_CLASS_NAME;

				RegisterClassA(&wc);
			}

			_handle = CreateWindowExA(
				0,
				NX_WIN32_WINDOW_CLASS_NAME,
				title.c_str(),
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT,
				width, height,
				nullptr, nullptr, nullptr, nullptr
			);

			NX_ASSERT(_handle != nullptr && "Failed to create window");

			ZeroMemory(&_data, sizeof(_data));

			SetWindowLongPtrA(_handle, GWLP_USERDATA, (LONG_PTR)(void*)&_data);
		}

		win32_window::~win32_window() {
			DestroyWindow(_handle);
		}

		void win32_window::show() {
			ShowWindow(_handle, SW_SHOWNORMAL);
		}

		void win32_window::hide() {
			ShowWindow(_handle, SW_HIDE);
		}

		void win32_window::maximize() {
			ShowWindow(_handle, SW_MAXIMIZE);
		}

		void win32_window::minimize() {
			ShowWindow(_handle, SW_MINIMIZE);
		}

		void win32_window::close() {
			CloseWindow(_handle);
		}

		void win32_window::update() {
			MSG message = {};
			while (PeekMessageA(&message, _handle, 0, 0, PM_REMOVE)) {
				TranslateMessage(&message);
				DispatchMessageA(&message);
			}
		}

		void win32_window::resize(uint32_t width, uint32_t height) {
			SetWindowPos(
				_handle,
				HWND_TOP,
				0, 0,
				width, height,
				SWP_NOMOVE
			);
		}

		uint32_t win32_window::width() const {
			RECT rect;
			GetClientRect(_handle, &rect);
			return rect.right - rect.left;
		}

		uint32_t win32_window::height() const {
			RECT rect;
			GetClientRect(_handle, &rect);
			return rect.bottom - rect.top;
		}

		std::string win32_window::title() const {
			std::string buffer;
			buffer.reserve(128);

			GetWindowTextA(_handle, buffer.data(), 256);
			return buffer;
		}

		void win32_window::title(const std::string &title) {
			SetWindowTextA(_handle, title.c_str());
		}

	}

	namespace native {

		std::shared_ptr<native_window> native_window::create(const std::string &title, uint32_t width, uint32_t height) {
			return std::make_shared<windows::win32_window>(
				title, width, height	
			);
		}

	}

}

#endif // _WIN32