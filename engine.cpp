#include "engine.h"

#include <signal.h>
#include <iostream>
#include <chrono>

// Global Variables
static bool is_engine_initialized = false;

// Constatnts
static const int MAX_FPS = 15;

namespace engine {
	// class engine : private
	void engine::init_thread(std::function<void(std::mutex&)> cb) {
		// "this" capturing like this->, so other local variables (but not "cb"?)
		std::thread t ([this, &cb]() {
			while(this->is_working) {
				cb(this->mutex);

				std::this_thread::sleep_for(std::chrono::milliseconds(1000 / MAX_FPS));
			}
		});

		t.detach();
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
		// halfdelay(1); // delay 1 = 0.1 sec
		nodelay(stdscr, TRUE);

		curs_set(0);

		signal(SIGINT, SIG_IGN);

		styles::styles style;
		getmaxyx(stdscr, style.height, style.width);

		this->div = new win::div(style);
	}

	void engine::start() {
		this->is_working = true;

		keys_table::key key_pressed, key_esc = "\u001B";

		this->ws = new windows_selector(this->div);
		this->on_key_pressed([this, &key_pressed](const keys_table::key& key) {
			key_pressed = key;
			if(key) this->ws->update(key);
		});

		// Key press event thread
		this->init_thread([this](std::mutex& mutex) {
			mutex.lock();
			char code1 = getch();
			char code2 = -1;
			char code3 = -1;
			char code4 = -1;

			if(code1 != -1) {
				code2 = getch();
				if (code2 == code1) {
				ungetch(code2);
				code2 = -1;
			}
			}
			if(code2 != -1) {
				code3 = getch();
				if(code3 == code1 || code3 == code2) {
					ungetch(code3);
					code3 = -1;
				}
			}
			if(code3 != -1) {
				code4 = getch();
				if(code4 == code1 || code4 == code2 || code4 == code3) {
					ungetch(code4);
					code4 = -1;
				}
			}
			
			// !ATTANTION! Here should calls engine::stop(), and mutex are locked here
			this->on_key_pressed.call({ code1, code2, code3, code4 });

			mutex.unlock();
		});

		// Rendering(main) thread
		while(this->is_working) {
			this->mutex.lock();

			this->div->print();

			doupdate();

			this->mutex.unlock();

			auto key_it = keys_table::KEYS.find("ESC");
			if (key_it != keys_table::KEYS.end() && key_it->second == key_pressed) this->stop();
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / MAX_FPS));
		}

		endwin();
	}

	// !REMEMBER! This function must be called IN RENDER(MAIN) THREAD
	void engine::stop() {
		this->mutex.lock();
		this->is_working = false;
		this->mutex.unlock();

		//delete this->ws;
		delete this->div;
	}

	// class windows_selector : public
	windows_selector::~windows_selector() {
		this->focused = nullptr;
	}

	int windows_selector::get_selected_index() const {
		return this->selected_index;
	}
	win::window* windows_selector::get_focused() const {
		return this->focused;
	}
	void windows_selector::update(const keys_table::key& key) {
		if(key == this->key_arrow_up) this->list_up();
		if(key == this->key_arrow_down) this->list_down();
		if(key == this->key_enter) this->select();
		if(key == this->key_backspace) this->unselect();
	}

	// class windows_selector : private
	win::window* windows_selector::list_up() {
		return this->focused;
	}
	win::window* windows_selector::list_down() {
		return this->focused;
	}
	win::window* windows_selector::select() {
		return this->focused;
	}
	win::window* windows_selector::unselect() {
		return this->focused;
	}
}
