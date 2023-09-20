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

int main() {
	keys::key key { -1, -1, -1, -1 };

	engine::engine engn;
	engn.init();

	int scrn_height, scrn_width;
	getmaxyx(stdscr, scrn_height, scrn_width);

	styles::styles my_styles;
	my_styles.width = 31;
	my_styles.height = 1;

	win::p key_view_win(my_styles);
	key_view_win.callback = [&key_view_win, &key]() {
		std::string text = "Key pressed: " + std::to_string(key.get_code1())
			+ '(' + std::to_string(key.get_code2()) + ')'
			+ '(' + std::to_string(key.get_code3()) + ')'
			+ '(' + std::to_string(key.get_code4()) + ')';

		key_view_win.inner_text = text;
	};

	//my_styles.margin_top = 1;
	//my_styles.height = 2;
	win::p datetime_win (my_styles);
	datetime_win.callback = [&datetime_win]() {
		const size_t time_str_size = std::size("HH:MM:SS | dd.mm.YYYY");
		char* time = new char[time_str_size];

		std::time_t t = std::time(0);
		std::tm tm = *std::localtime(&t);

		std::strftime(time, time_str_size, "%H:%M:%S | %d.%m.%Y", &tm);
		datetime_win.inner_text = time;
	};


	// Creating battery charge indicator
	styles::styles batt_styles;
	batt_styles.width = 25;
	batt_styles.color = 0xFF0000;
	batt_styles.align = styles::keywords::SK_HORIZONTAL;
	win::div batt_level_div (batt_styles);

//	batt_styles.align = styles::keywords::SK_VERTICAL;
	batt_styles.width = 20;
	win::progress batt_progress(batt_styles);
	batt_progress.callback = [&batt_progress]() {
		batt_progress.value = utility::get_batt_level();
	};

	batt_styles.width = 5;
	win::p batt_level_p (batt_styles);
	batt_level_p.callback = [&batt_level_p]() {
		batt_level_p.inner_text = utility::to_string(utility::get_batt_level(), 2);
	};
	batt_level_div.append(&batt_progress);
	batt_level_div.append(&batt_level_p);

	my_styles.height = 2;
	my_styles.pos_z = 1;
	my_styles.position = styles::keywords::SK_FIXED;
	win::p test_win (my_styles);
	test_win.callback = [&test_win]() {
		test_win.inner_text = "Hello,\n world!";
	};
	test_win.style.position = styles::keywords::SK_FIXED;
	test_win.style.margin_top = 5;


	my_styles.margin_left = 0;
	//my_styles.margin_top = 4;
	my_styles.height = 10;
	my_styles.width  = 40;
	win::div container (my_styles);

	container.append(&key_view_win);
	container.append(&datetime_win);
	container.append(&batt_level_div);

	engn.div->append(&container);
	engn.div->append(&test_win);

	engn.on_key_pressed([&key] (const keys::key& gkey) {
		key = gkey;
	});

	engn.start();

	return 0;
}

