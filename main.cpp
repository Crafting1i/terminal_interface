#include <iostream>

#include <ncurses.h>

#include "engine.h"

#include <sys/resource.h>
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

	init_pair(1, COLOR_BLUE, COLOR_YELLOW);
	styles::styles my_styles;
	my_styles.width  = 31;
	my_styles.height = 1;
	my_styles.color_pair = COLOR_PAIR(1);

	win::p key_view_win(my_styles);
	key_view_win.style.width = 31;
	key_view_win.callback = [&key_view_win, &key]() {
		std::string text = "Key pressed: " + std::to_string(key.get_code1())
			+ '(' + std::to_string(key.get_code2()) + ')'
			+ '(' + std::to_string(key.get_code3()) + ')'
			+ '(' + std::to_string(key.get_code4()) + ')';

		key_view_win.inner_text = text;
	};

	win::p datetime_win (my_styles);
	datetime_win.style.width = 31;
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
	batt_styles.color_pair = COLOR_PAIR(1);
	batt_styles.align = styles::keywords::SK_HORIZONTAL;
	win::div batt_level_div (batt_styles);

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

	win::p test_win (my_styles);
	test_win.callback = [&test_win]() {
		test_win.inner_text = "Hello,\n world!";
	};
	test_win.style.position = styles::keywords::SK_FIXED;
	test_win.style.margin_top = 5;
	test_win.style.pos_z = 1;
	test_win.style.height = 2;

	win::div container(my_styles);
	container.style.width  = 40;
	container.style.height = 10;

	container.append(&key_view_win);
	container.append(&datetime_win);
	container.append(&batt_level_div);



	win::p sysstat_p;
	sysstat_p.style.color_pair = COLOR_PAIR(1);
	sysstat_p.style.height = 2;
	sysstat_p.style.width = 20;
	struct rusage rusage;
	sysstat_p.callback = [&sysstat_p, &rusage]() {
		//getrusage(RUSAGE_SELF, &rusage);
		sysstat_p.inner_text = "CPU usage: ";
		// sysstat_p.inner_text += utility::to_string(
		// 	(double)rusage.ru_utime.tv_sec / rusage.ru_stime.tv_sec *  100,
		// 	2
		// );
	};

	win::div sysstat_container;
	sysstat_container.style.color_pair = COLOR_PAIR(1);
	sysstat_container.style.width  = 22;
	sysstat_container.style.height = 10;
	sysstat_container.style.position = styles::keywords::SK_FIXED;
	sysstat_container.style.margin_left = scrn_width - 22;

	sysstat_container.append(&sysstat_p);

	engn.div->append(&container);
	engn.div->append(&sysstat_container);
	engn.div->append(&test_win);

	engn.on_key_pressed([&key] (const keys::key& gkey) {
		key = gkey;
	});

	engn.start();

	return 0;
}

