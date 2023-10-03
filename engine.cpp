#include "engine.h"

#include <signal.h>
#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>

// Global Variables
static bool is_engine_initialized = false;

// Constatnts
//static const int MAX_FPS = 15;

namespace engine {
	// class engine : private
	void engine::init_thread(std::function<void(std::mutex&)> cb, uint32_t timeout_ms) {
		if(timeout_ms == (uint32_t)(-1)) timeout_ms = 1000 / this->MAX_FPS;
		// "this" capturing like this->, so other local variables (but not "cb"?)
		std::thread t ([this, &cb, timeout_ms]() {
			while(this->is_working) {
				cb(this->mutex);

				std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
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
		use_default_colors();

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

		keys::key key_pressed, key_esc = "\u001B";

		this->ws = new windows_selector(this->div);
		this->on_key_pressed([this, &key_pressed](const keys::key& key) {
			key_pressed = key;
			if(key) {
				this->ws->update(key);
			}
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

			this->div->refresh_size();
			this->div->print();

			doupdate();

			this->mutex.unlock();

			if (key_pressed == key_esc) this->stop();
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / MAX_FPS));
		}

		endwin();
	}

	// !REMEMBER! This function must be called IN RENDER(MAIN) THREAD
	void engine::stop() {
		this->mutex.lock();
		this->is_working = false;
		this->mutex.unlock();

		delete this->ws;
		delete this->div;
	}

	// class windows_selector : public
	windows_selector::windows_selector(win::window* win): focused(win) {
		this->info = new win::p();
		//this->info->style.align = styles::keywords::SK_RIGHT;
		//this->info->style.position = styles::keywords::SK_FIXED;
		this->info->style.width = styles::s_digit(100, styles::digit_type::DT_PERCENT);
		this->info->style.height = 1;
		//this->info->style.margin_left = this->focused->get_width() - 8;
		//this->info->style.autotrim = false;
		this->info->style.is_moveble = false;

		this->info->callback = [this]() {
			win::win_type win_type = this->focused->get_type();
			this->info->inner_text = std::to_string(this->selected_index + 1) + '/' + std::to_string(this->focused_children_size);
			this->info->inner_text += '(' + std::string(
					win_type == win::win_type::wt_div ? "div"
					: win_type == win::win_type::wt_p ? "p"
					: win_type == win::win_type::wt_progress ? "progress"
					: "unknown"
				) + ')';
		};

		dynamic_cast<win::div*>(this->focused)->append(this->info);
		this->focused_children_size = dynamic_cast<win::div*>(this->focused)->get_children().size();
	}
	windows_selector::~windows_selector() {
		this->focused = nullptr;
		delete this->info;
	}

	int windows_selector::get_selected_index() const {
		return this->selected_index;
	}
	win::window* windows_selector::get_focused() const {
		return this->focused;
	}
	void windows_selector::update(const keys::key& key) {
		if(key == this->key_page_up) return (void)this->list_up();
		if(key == this->key_page_down) return (void)this->list_down();
		if(key == this->key_insert) return (void)this->select();
		if(key == this->key_end) return (void)this->unselect();

		if(this->focused->style.position != styles::keywords::SK_FIXED) return;
		else if(!this->focused->style.is_moveble) return;

		if(key == this->key_arrow_up) this->move_focused(0, 1);
		else if(key == this->key_arrow_down) this->move_focused(0, -1);
		else if(key == this->key_arrow_right) this->move_focused(1, 0);
		else if(key == this->key_arrow_left) this->move_focused(-1, 0);
	}

	void windows_selector::move_focused(int x, int y) {
		if(this->focused->style.position != styles::keywords::SK_FIXED)
			throw std::logic_error("'this->focused->style.position' is not 'SK_FIXED'");
		
		this->focused->clear();
		
		if(this->focused->style.margin_top > y && y < 0)
			this->focused->style.margin_top = 0;
		else this->focused->style.margin_top -= y;
		if(this->focused->style.margin_left > x && x < 0)
			this->focused->style.margin_left = 0;
		else this->focused->style.margin_left += x;

		//mvprintw(12, 10, "%u %u", this->focused->style.margin_top, this->focused->style.margin_left);
	}

	// class windows_selector : private
	win::window* windows_selector::list_up() {
		if(this->focused->get_type() != win::win_type::wt_div)
			return this->focused;

		auto children = dynamic_cast<win::div*>(this->focused)->get_children();
		if(this->selected_index + 1 == children.size()) this->selected_index = 0;
		else this->selected_index += 1;
		
		//mvprintw(10, 10, "%zu/%zu", this->selected_index, children.size());

		return this->focused;
	}
	win::window* windows_selector::list_down() {
		if(this->focused->get_type() != win::win_type::wt_div)
			return this->focused;

		auto children = dynamic_cast<win::div*>(this->focused)->get_children();
		if(this->selected_index == 0) this->selected_index = children.size() - 1;
		else this->selected_index -= 1;
		
		//mvprintw(10, 10, "%zu/%zu", this->selected_index, children.size());

		return this->focused;
	}
	win::window* windows_selector::select() {
		if(this->focused->get_type() != win::win_type::wt_div)
			return this->focused;

		this->focused->style.color_pair_filters.erase(A_REVERSE);
		auto children = dynamic_cast<win::div*>(this->focused)->get_children();
		this->focused = children[this->selected_index];
		this->selected_index = 0;

		this->focused->style.color_pair_filters.insert(A_REVERSE);

		if(this->focused->get_type() == win::win_type::wt_div)
			this->focused_children_size = dynamic_cast<win::div*>(this->focused)->get_children().size();
		else this->focused_children_size = 0;

		return this->focused;
	}
	win::window* windows_selector::unselect() {
		this->focused->style.color_pair_filters.erase(A_REVERSE);
		
		if(this->focused->get_parent() != nullptr) {
			this->selected_index = 0;
			this->focused_children_size = this->focused->get_parent()->get_children().size();
			this->focused = dynamic_cast<win::window*>(this->focused->get_parent());
			
			if(this->focused->get_parent() != nullptr)
				this->focused->style.color_pair_filters.insert(A_REVERSE);
		}

		return this->focused;
	}
}
