#include "engine.h"

#include <signal.h>
#include <iostream>
#include <chrono>

// Global Variables
bool is_engine_initialized = false;

// Constatnts
static const int MAX_FPS = 30;

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
			win->callback(win);
		}

		mvprintw(1, 1, "Hmmmm init_thread key_getch");
		refresh();
	}

	void engine::init_thread(std::function<void(std::mutex&)> cb) {
		// "this" capturing like this->, so other local variables (but not "cb"?)
		std::thread* t = new std::thread([this, &cb]() {
			while(this->is_working) {
				cb(this->mutex);

				std::this_thread::sleep_for(std::chrono::milliseconds(1000 / MAX_FPS));
			}
		});

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
		int key_c = -1;

		// Key press event thread
		this->init_thread([this, &key_c](std::mutex& mutex) {
			int key_code = getch();
			int additional_code = getch();
			
			mutex.lock();
			mvprintw(0, 0, "Hmmmm init_thread key_getch");
			refresh();

			if(key_code == -1 || additional_code == key_code) {
				key_code = additional_code;
				additional_code = -1;
			}
			
			key_c = key_code;

			// !ATTANTION! Here should calls engine::stop(),and mutex are locked here
			this->on_key_pressed.call(key_code, additional_code);

			mutex.unlock();
		});

		// Rendering(main) thread
		while(this->is_working) {
			this->mutex.lock();
			for(win::window* win : this->wm.get_windows()) {
				win->callback(win);
			}
			
			mvprintw(1, 0, "Hmmmm frames render");
			mvprintw(2, 0, "%zu", this->wm.get_windows().size());
			refresh();
			
			if (key_c == utility::K_KEYS::KK_ESC) this->stop();
			this->mutex.unlock();
			
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / MAX_FPS));
		}
	}
	
	// !REMEMBER! This function must be called IN RENDER(MAIN) THREAD
	void engine::stop() {
		this->is_working = false;

		for (auto it = this->threads.begin(); it != this->threads.end(); it = this->threads.begin()) {
			this->threads.erase(it);
			std::thread* t = *it;
			
			t->join();
			
			delete t;
		}

		endwin();
	}
}
