#pragma once

#include <nexus/core.hpp>

namespace nexus {

	typedef void(*close_callback)();
	typedef void(*size_callback)(uint32_t width, uint32_t height);
	typedef void(*pos_callback)(uint32_t x, uint32_t y);
	typedef void(*focus_callback)(bool focused);

	namespace native {

		class native_window {
		public:
			static std::shared_ptr<native_window> create(const std::string &title, uint32_t width, uint32_t height);

		public:
			native_window() { }
			virtual ~native_window() { }

			virtual void show() = 0;
			virtual void hide() = 0;
			virtual void maximize() = 0;
			virtual void minimize() = 0;
			virtual void close() = 0;

			virtual void update() = 0;


			virtual void on_close(close_callback callback) = 0;
			virtual void on_resize(size_callback callback) = 0;
			virtual void on_move(pos_callback callback) = 0;
			virtual void on_focus_changed(focus_callback callback) = 0;

			virtual void resize(uint32_t width, uint32_t height) = 0;
			virtual uint32_t width() const = 0;
			virtual uint32_t height() const = 0;
			virtual std::string title() const = 0;
			virtual bool is_closed() const = 0;
			virtual void title(const std::string &title) = 0;

			virtual void *handle() = 0; 
		};

	}

}