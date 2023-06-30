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

	engine::engine engn;
	engn.init();

	int scrn_height, scrn_width;
	getmaxyx(stdscr, scrn_height, scrn_width);


	win::window key_view_win(25, 1, scrn_width - 25, 0, nullptr);
	key_view_win.style.text_align = styles::keywords::SK_RIGHT;

	int i = 0;
	std::function<void(win::window*)> kvw_callback = [&key_view_win, &key_code, &additional_kcode, &i](win::window* thisptr) {
		std::string text = "Key pressed: " + std::to_string(key_code) + '(' + std::to_string(additional_kcode) + ')';
		thisptr->print(text.c_str());

		mvprintw(3, 0, "%d", i++);
	};
	key_view_win.callback.swap(kvw_callback);

	//win::window datatime_win (26, 1, scrn_width - 26, 1, nullptr);
	//datatime_win.style.text_align = styles::keywords::SK_RIGHT;

	engn.wm.add_win(&key_view_win);

	engn.on_key_pressed([&engn, &key_code, &additional_kcode] (int key_c, int additional_kc) {
		key_code = key_c;
		additional_kcode = additional_kc;
	});

	engn.start();

	return 0;
}

