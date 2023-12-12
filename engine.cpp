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

namespace ami {
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

		this->threads_pool.start();

		curs_set(0);

		signal(SIGINT, SIG_IGN);

		styles::styles style;
		getmaxyx(stdscr, style.height, style.width);

		this->div = new ami::div(style);
		this->ws = new windows_selector(this->div);
	}

	void engine::start() {
		this->is_working = true;

		ami::key key_pressed, key_esc = "\u001B";

		this->on_key_pressed([this, &key_pressed](const ami::key& key) {
			key_pressed = key;
			if(key) {
				this->ws->update(key);
			}
		});

		this->threads_pool.add_task(ami::threads::task([this](std::atomic<bool>& is_working) {
			while(is_working) {
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
				std::this_thread::sleep_for(std::chrono::milliseconds(80));
			}
		}));

		// Rendering(main) thread
		while(this->is_working) {
			this->mutex_widows.lock();

			this->div->refresh_size();
			this->div->print();

			this->mutex_widows.unlock();

			doupdate();

			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / MAX_FPS));
		}
		this->stop_eng();

		endwin();
	}

	void engine::stop() {
		this->is_working.store(false);
	}

	// class engine : private
	void engine::stop_eng() {
		this->mutex_on_key_pressed.lock();
		this->on_key_pressed.clear();
		this->mutex_on_key_pressed.unlock();

		this->mutex_widows.lock();
		delete this->ws;
		delete this->div;
		this->mutex_widows.unlock();
	}
	
	windows_selector* engine::get_ws() const noexcept {
		return this->ws;
	}

	// class windows_selector : public
	windows_selector::windows_selector(ami::window* win): focused(win) {
		this->info = new ami::p();
		this->info->style.width = styles::s_digit(100, styles::digit_type::DT_PERCENT);
		this->info->style.height = 1;
		this->info->style.is_moveble = false;

		this->info->callback = [this]() {
			ami::win_type win_type = this->focused->get_type();
			this->info->inner_text = std::to_string(this->selected_index + 1) + '/' + std::to_string(this->focused_children_size);
			this->info->inner_text += '(' + std::string(
					win_type == ami::win_type::wt_div ? "div"
					: win_type == ami::win_type::wt_p ? "p"
					: win_type == ami::win_type::wt_progress ? "progress"
					: "unknown"
				) + ')';
		};

		dynamic_cast<ami::div*>(this->focused)->append(this->info);
		this->focused_children_size = dynamic_cast<ami::div*>(this->focused)->get_children().size();
	}
	windows_selector::~windows_selector() {
		this->focused = nullptr;
		delete this->info;
	}

	int windows_selector::get_selected_index() const noexcept {
		return this->selected_index;
	}
	ami::window* windows_selector::get_focused() const noexcept {
		return this->focused;
	}
	void windows_selector::update(const ami::key& key) {
		if(focused->get_type() == ami::wt_input && key != this->key_end) {
			ami::input* in = dynamic_cast<ami::input*>(this->focused);

			if(key == "\u007F") {
				if(in->value.length()) in->value.erase(in->value.length() - 1);
			} else if(key.get_string().length() == 1) in->value += key.get_string();
			return;
		}
		if(focused->get_type() == ami::wt_button && key == this->key_insert) {
			dynamic_cast<ami::button*>(this->focused)->press();
			return;
		}

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
	ami::window* windows_selector::list_up() noexcept {
		if(this->focused->get_type() != ami::win_type::wt_div)
			return this->focused;

		auto children = dynamic_cast<ami::div*>(this->focused)->get_children();
		if(this->selected_index + 1 == children.size()) this->selected_index = 0;
		else this->selected_index += 1;

		return this->focused;
	}
	ami::window* windows_selector::list_down() noexcept {
		if(this->focused->get_type() != ami::win_type::wt_div)
			return this->focused;

		auto children = dynamic_cast<ami::div*>(this->focused)->get_children();
		if(this->selected_index == 0) this->selected_index = children.size() - 1;
		else this->selected_index -= 1;

		return this->focused;
	}
	ami::window* windows_selector::select() {
		if(this->focused->get_type() != ami::win_type::wt_div)
			return this->focused;

		this->focused->style.color_pair_filters.erase(A_REVERSE);
		auto children = dynamic_cast<ami::div*>(this->focused)->get_children();
		this->focused = children[this->selected_index];
		this->selected_index = 0;

		this->focused->style.color_pair_filters.insert(A_REVERSE);

		if(this->focused->get_type() == ami::win_type::wt_div)
			this->focused_children_size = dynamic_cast<ami::div*>(this->focused)->get_children().size();
		else this->focused_children_size = 0;

		return this->focused;
	}
	ami::window* windows_selector::unselect() {
		this->focused->style.color_pair_filters.erase(A_REVERSE);
		
		if(this->focused->get_parent() != nullptr) {
			this->selected_index = 0;
			this->focused_children_size = this->focused->get_parent()->get_children().size();
			this->focused = dynamic_cast<ami::window*>(this->focused->get_parent());
			
			if(this->focused->get_parent() != nullptr)
				this->focused->style.color_pair_filters.insert(A_REVERSE);
		}

		return this->focused;
	}
}
