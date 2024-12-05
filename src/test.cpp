#include <cstdio>

#include <nexus/system/window.hpp>

int main(int argc, char **argv) {
	nexus::window window = nexus::window::create("Hello World!", 1920 / 2, 1080 / 2);
	window.show();
	window.maximize();

	while (!window.is_closed()) {
		window.update();
	}

	return 0;
}