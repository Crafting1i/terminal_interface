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
#include <numeric>

#include <ios>
#include <iostream>
#include <fstream>

#include <thread>
#include <mutex>

void process_mem_usage(double& vm_usage, double& resident_set);
bool get_cpu_times(size_t &idle_time, size_t &total_time);
std::vector<size_t> get_cpu_times();

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

	// win::p test_win (my_styles);
	// test_win.callback = [&test_win]() {
	// 	test_win.inner_text = "Hello,\n world!";
	// };
	// test_win.style.position = styles::keywords::SK_FIXED;
	// test_win.style.margin_top = 5;
	// test_win.style.pos_z = 1;
	// test_win.style.height = 2;

	win::div container(my_styles);
	container.style.width  = 40;
	container.style.height = 10;

	container.append(&key_view_win);
	container.append(&datetime_win);
	container.append(&batt_level_div);



	win::p sysstat_p;
	
	double cpu_usage = 0.0;
	sysstat_p.style.color_pair = COLOR_PAIR(1);
	sysstat_p.style.height = 2;
	sysstat_p.style.width = 19;
	sysstat_p.style.pos_z = 15;
	sysstat_p.callback = [&sysstat_p, &cpu_usage]() {
		sysstat_p.inner_text = "CPU usage: " + utility::to_string(cpu_usage, 2) + "%";
	};

	win::p sysmem_p;
	struct rusage rusage;
	getrusage(RUSAGE_SELF, &rusage);
	sysmem_p.style.color_pair = COLOR_PAIR(1);
	sysmem_p.style.height = 2;
	sysmem_p.style.width = 19;
	sysmem_p.callback = [&sysmem_p, &rusage]() {
		sysmem_p.inner_text = "Mem:" + utility::to_string(rusage.ru_maxrss / 1024.0, 1) + "kB";
	};

	win::div sysstat_container;
	sysstat_container.style.color_pair = COLOR_PAIR(1);
	sysstat_container.style.width  = 22;
	sysstat_container.style.height = 10;
	sysstat_container.style.position = styles::keywords::SK_FIXED;
	sysstat_container.style.margin_left = scrn_width - 40;

	sysstat_container.append(&sysstat_p);
	sysstat_container.append(&sysmem_p);

	engn.div->append(&container);
	engn.div->append(&sysstat_container);
	//engn.div->append(&test_win);

	engn.on_key_pressed([&key] (const keys::key& gkey) {
		key = gkey;
	});

	engn.init_thread([&rusage](std::mutex& mutex) {
		mutex.lock();
		getrusage(RUSAGE_SELF, &rusage);
		mutex.unlock();
	}, 5000);


	size_t previous_idle_time = 0, previous_total_time = 0;
	engn.init_thread([&previous_idle_time, &previous_total_time, &cpu_usage](std::mutex& mutex) {
		size_t idle_time = 0, total_time = 0;
		get_cpu_times(idle_time, total_time);

		const float idle_time_delta = idle_time - previous_idle_time;
		const float total_time_delta = total_time - previous_total_time;
		
		mutex.lock();
		cpu_usage = 100.0 * (1.0 - idle_time_delta / total_time_delta);

		previous_idle_time = idle_time;
    previous_total_time = total_time;
		mutex.unlock();
	}, 2000);


	engn.start();

	return 0;
}

void process_mem_usage(double& vm_usage, double& resident_set) {
  using std::ios_base;
  using std::ifstream;
  using std::string;

	vm_usage     = 0.0;
	resident_set = 0.0;

	// 'file' stat seems to give the most reliable results
	//
	ifstream stat_stream("/proc/self/stat",ios_base::in);

	// dummy vars for leading entries in stat that we don't care about
	//
	string pid, comm, state, ppid, pgrp, session, tty_nr;
	string tpgid, flags, minflt, cminflt, majflt, cmajflt;
	string utime, stime, cutime, cstime, priority, nice;
	string O, itrealvalue, starttime;

	// the two fields we want
	//
	unsigned long vsize;
	long rss;

	stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
							>> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
							>> utime >> stime >> cutime >> cstime >> priority >> nice
							>> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

	stat_stream.close();

	long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
	vm_usage     = vsize / 1024.0;
	resident_set = rss * page_size_kb;
}

std::vector<size_t> get_cpu_times() {
	std::ifstream proc_stat("/proc/stat");
	proc_stat.ignore(5, ' '); // Skip the 'cpu' prefix.
	std::vector<size_t> times;
	for (size_t time; proc_stat >> time; times.push_back(time));
	return times;
}

bool get_cpu_times(size_t &idle_time, size_t &total_time) {
	const std::vector<size_t> cpu_times = get_cpu_times();
	if (cpu_times.size() < 4)
			return false;
	idle_time = cpu_times[3];
	total_time = std::accumulate(cpu_times.begin(), cpu_times.end(), 0);
	return true;
}
