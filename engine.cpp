#include "engine.h"

#include <signal.h>
#include <iostream>
#include <chrono>

// Global Variables
bool is_engine_initialized = false;

// Constatnts
static const int MAX_FPS = 30;

namespace engine {
	// class engine : private
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
//		halfdelay(1); // delay 1 = 0.1 sec
		nodelay(stdscr, TRUE);

		curs_set(0);

		signal(SIGINT, SIG_IGN);

		styles::styles style;
		getmaxyx(stdscr, style.height, style.width);

		this->div = new win::div(style);
	}

	void engine::start() {
		this->is_working = true;

		int key_c = -1;
		int a_key_c = -1;
		//int spec1_key_c = -1;
		//int spec2_key_c = -1;

		this->on_key_pressed([&key_c, &a_key_c](int k_code, int a_code, int s1_code, int s2_code) {
			key_c = k_code;
			a_key_c = a_code;
		});

		// Key press event thread
		this->init_thread([this](std::mutex& mutex) {
			int key_code        = getch();
			int additional_code = -1;
			int special1_code   = -1;
			int special2_code   = -1;

			if(key_code != -1) {
				additional_code = getch();
			}
			if(additional_code != -1) {
				special1_code = getch();
			}
			if(special1_code != -1) {
				special2_code = getch();
			}

			if(special2_code != -1 && (special2_code == key_code || special2_code == additional_code || special2_code == special1_code)) {
				ungetch(special2_code);
				special2_code = -1;
			}
			if(special1_code != -1 && (special1_code == key_code || special1_code == additional_code)) {
				ungetch(special1_code);
				special1_code = -1;
			}
			if(additional_code != -1 && additional_code == key_code) {
				ungetch(additional_code);
				additional_code = -1;
			}

			mutex.lock();
			// !ATTANTION! Here should calls engine::stop(), and mutex are locked here
			this->on_key_pressed.call(key_code, additional_code, special1_code, special2_code);

			mutex.unlock();
		});

		// Rendering(main) thread
		while(this->is_working) {
			this->mutex.lock();

			this->div->print();

			this->mutex.unlock();

			if (key_c == utility::K_KEYS::KK_ESC && a_key_c == -1) this->stop();
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / MAX_FPS));
		}

		endwin();
	}

	// !REMEMBER! This function must be called IN RENDER(MAIN) THREAD
	void engine::stop() {
		this->mutex.lock();
		this->is_working = false;
		this->mutex.unlock();

		// Mutex still locked
		for (auto it = this->threads.begin(); it != this->threads.end(); it = this->threads.begin()) {
			this->threads.erase(it);
			std::thread* t = *it;

			t->join();

			delete t;
		}

		delete this->div;
	}
}
