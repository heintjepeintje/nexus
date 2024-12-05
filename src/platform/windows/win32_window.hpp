#pragma once

#ifdef _WIN32

#include <nexus/core.hpp>
#include <nexus/native/native_window.hpp>

#include <windows.h>

namespace nexus {

	namespace windows {

		struct win32_window_data {
			close_callback on_close;
			size_callback on_resize;
			pos_callback on_move;
			focus_callback on_focus_changed;
			bool is_closed;
		};

		class win32_window : public native::native_window {
		public:
			win32_window(const std::string &title, uint32_t width, uint32_t height);
			~win32_window();

			virtual void show() override;
			virtual void hide() override;
			virtual void maximize() override;
			virtual void minimize() override;
			virtual void close() override;

			virtual void update() override;

			inline virtual void on_close(close_callback callback) override { _data.on_close = callback; }
			inline virtual void on_resize(size_callback callback) override { _data.on_resize = callback; }
			inline virtual void on_move(pos_callback callback) override { _data.on_move = callback; }
			inline virtual void on_focus_changed(focus_callback callback) override { _data.on_focus_changed = callback; }

			virtual void resize(uint32_t width, uint32_t height);
			virtual uint32_t width() const;
			virtual uint32_t height() const;
			virtual std::string title() const;
			virtual bool is_closed() const override { return _data.is_closed; }
			virtual void title(const std::string &title);

			inline virtual void *handle() { return (void*)_handle; }
		
		private:
			HWND _handle;
			win32_window_data _data;
		};

	}

}

#endif // _WIN32