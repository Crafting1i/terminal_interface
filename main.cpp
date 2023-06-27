#include <iostream>

#include <ncurses.h>

#include "engine.h"

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <functional>
#include <vector>
#include <string>
#include <chrono>

#include <thread>
#include <mutex>

enum K_KEYS {
	KK_ESC = 27
};

int main() {
	int key_code = -1;
	int additional_kcode = -1;

	int scrn_height, scrn_width;
	getmaxyx(stdscr, scrn_height, scrn_width);

	engine::engine engn;

	win::window key_view_win (25, 1, scrn_width - 25, 0, nullptr);
	key_view_win.style.text_align = styles::keywords::SK_RIGHT;
	key_view_win.callback = [&key_view_win, &key_code, &additional_kcode]() {
		std::string text = "Key pressed: " + std::to_string(key_code) + '(' + std::to_string(additional_kcode) + ')';
		key_view_win.print(text.c_str());
	};

	win::window datatime_win (26, 1, scrn_width - 26, 1, nullptr);
	datatime_win.style.text_align = styles::keywords::SK_RIGHT;

	engn.wm.add_win(&key_view_win);

	int i = 0;
	engn.on_key_pressed([&engn, &key_code, &additional_kcode, &i] (int key_c, int additional_kc) {
		key_code = key_c;
		additional_kcode = additional_kc;
	});
	
	engn.init();
	
	return 0;
}
