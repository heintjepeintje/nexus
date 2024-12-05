#pragma once

#include <nexus/core.hpp>
#include <nexus/native/native_window.hpp>

namespace nexus {

	class window {
	public:
		static window create(const std::string &title, uint32_t width, uint32_t height) {
			return window(native::native_window::create(
				title, width, height
			));
		}

	public:
		window() { }
		~window() { }

		inline void show() { _handle->show(); }
		inline void hide() { _handle->hide(); }
		inline void maximize() { _handle->maximize(); }
		inline void minimize() { _handle->minimize(); }
		inline void close() { _handle->close(); }

		inline void update() const { _handle->update(); } 

		inline void on_close(close_callback callback) { _handle->on_close(callback); }
		inline void on_resize(size_callback callback) { _handle->on_resize(callback); }
		inline void on_move(pos_callback callback) { _handle->on_move(callback); }
		inline void on_focus_changed(focus_callback callback) { _handle->on_focus_changed(callback); }

		inline void resize(uint32_t width, uint32_t height) { _handle->resize(width, height); }
		inline uint32_t width() const { return _handle->width(); }
		inline uint32_t height() const { return _handle->height(); }
		inline std::string title() const { return _handle->title(); }
		inline bool is_closed() const { return _handle->is_closed(); }
		inline void title(const std::string &title) { _handle->title(title); }

		inline std::shared_ptr<native::native_window> handle() const { return _handle; }

	private:
		window(const std::shared_ptr<native::native_window> &handle) : _handle(handle) { }

	private:
		std::shared_ptr<native::native_window> _handle;
	};

}