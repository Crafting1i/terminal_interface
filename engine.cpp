#include "engine.h"

#include <signal.h>
#include <chrono>

// Global Variables
bool is_engine_initialized = false;

// Constatnts
const int MAX_FPS = 30;

namespace engine {
	// class windows_manager : public
	void windows_manager::add_win(win::window* win) {
		this->windows.push_back(win);
	}
	void windows_manager::remove_win(win::window* win) {
	}

	decltype(windows_manager::windows) windows_manager::get_windows() {
		return this->windows;
	}

	// class engine : private
	void engine::frame_request() {
		// make some global function
		for(win::window* win : this->wm.get_windows()) {
			win->callback();
		}
	}

	void engine::init_thread(std::function<void(void)> cb) {
		std::thread* t = new std::thread([this, &cb]() {
			this->mutex.lock();
			cb();
			this->mutex.unlock();

			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / MAX_FPS));
		});

		//t.detach();
		this->threads.push_back(t);
	}
	// class engine : public
	void engine::init() {
		if(is_engine_initialized)
			throw std::runtime_error("Engine has been initilized. You can't do it twice");

		initscr();
		start_color();

		intrflush(stdscr, FALSE);
		scrollok(stdscr, TRUE);
		keypad(stdscr, FALSE);
		raw();
		noecho();
		halfdelay(1); // delay 1 = 0.1 sec

		signal(SIGINT, SIG_IGN);

		this->is_working = true;

		// Key press event
		this->init_thread([this]() {
			int key_code = getch();
			int additional_code = getch();

			if(key_code == -1 || additional_code == key_code) {
				key_code = additional_code;
				additional_code = -1;
			}

			this->on_key_pressed.call(key_code, additional_code);
		});

		// Rendering thread
		this->init_thread([this]() { this->frame_request(); });
	}

	void engine::stop() {
		this->mutex.lock();
		this->is_working = false;
		this->mutex.unlock();

		for (std::thread* t : this->threads) {
			t->join();
		}

		endwin();
	}
}
