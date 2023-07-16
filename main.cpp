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
	int special1_kcode = -1;
	int special2_kcode = -1;

	engine::engine engn;
	engn.init();

	int scrn_height, scrn_width;
	getmaxyx(stdscr, scrn_height, scrn_width);

	styles::styles my_styles;
	my_styles.width = 31;
	my_styles.height = 1;

	win::p key_view_win(my_styles);
	key_view_win.callback = [&key_view_win, &key_code, &additional_kcode, &special1_kcode, &special2_kcode](win::window* thisptr) {
		std::string text = "Key pressed: " + std::to_string(key_code)
			+ '(' + std::to_string(additional_kcode) + ')'
			+ '(' + std::to_string(special1_kcode) + ')'
			+ '(' + std::to_string(special2_kcode) + ')';

		key_view_win.inner_text = text;
	};

	my_styles.margin_top = 1;
	win::p datetime_win (my_styles);
	datetime_win.callback = [&datetime_win](win::window* thisptr) {
		const size_t time_str_size = std::size("HH:MM:SS | dd.mm.YYYY");
		char* time = new char[time_str_size];

		std::time_t t = std::time(0);
		std::tm tm = *std::localtime(&t);

		std::strftime(time, time_str_size, "%H:%M:%S | %d.%m.%Y", &tm);
		datetime_win.inner_text = time;
	};

	//my_styles.margin_top = 2;
	win::p batt_level_win (my_styles);
	batt_level_win.callback = [&batt_level_win](win::window* thisptr) {
		std::string chargelvl = "Charge: " + utility::to_string(utility::get_batt_level(), 2);
		batt_level_win.inner_text = chargelvl;
	};

	//my_styles.margin_top = 0;
	my_styles.height = 15;
	my_styles.width  = 40;
	win::div container (my_styles);

	container.append(&key_view_win);
	container.append(&datetime_win);
	container.append(&batt_level_win);

	engn.wm.add_win(&container);

	engn.on_key_pressed([&key_code, &additional_kcode, &special1_kcode, &special2_kcode] (int k_code, int a_code, int s1_code, int s2_code) {
		key_code = k_code;
		additional_kcode = a_code;
		special1_kcode = s1_code;
		special2_kcode = s2_code;
	});

	engn.start();

	return 0;
}

