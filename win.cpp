#include "win.h"
#include "util.h"

#include <iostream>

#include <string.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <regex>

//#include <tcl.h>

namespace win {
// class window : protected
void window::rewrite_parent(div* parent) {
	if(this->parent) this->parent->remove(this);
	this->parent = parent;

	// delwin(this->handle);

	// this->handle = derwin(parent ? parent->handle : stdscr, 1, 1, 0, 0);
	// if(!this->handle) throw std::runtime_error("Handle creating have failed");

	this->refresh_size();
}

// class window : public
void window::refresh_size() {
	if(!this->handle) throw std::runtime_error("Can't use without window handle");
	werase(this->handle);

	int ph, pw; // p = parent

	if(!parent) getmaxyx(stdscr, ph, pw);
	else {
		ph = parent->get_height();
		pw = parent->get_width();
	}

	int nx = this->ppadding_x ? this->ppadding_x : fmin(style.margin_left, pw);
	int ny = this->ppadding_y ? this->ppadding_y : fmin(style.margin_top,  ph);

	uint32_t style_width = this->style.width.get_type() == styles::digit_type::DT_PIXEL
		? this->style.width.get_value() : this->style.width.get_value() * pw / 100;
	uint32_t style_height = this->style.height.get_type() == styles::digit_type::DT_PIXEL
		? this->style.height.get_value() : this->style.height.get_value() * ph / 100;

	uint32_t style_max_width = this->style.max_width.get_type() == styles::digit_type::DT_PIXEL
		? this->style.max_width.get_value() : this->style.max_width.get_value() * pw / 100;
	uint32_t style_max_height = this->style.max_height.get_type() == styles::digit_type::DT_PIXEL
		? this->style.max_height.get_value() : this->style.max_height.get_value() * ph / 100;

	uint32_t style_min_width = this->style.min_width.get_type() == styles::digit_type::DT_PIXEL
		? this->style.min_width.get_value() : this->style.min_width.get_value() * pw / 100;
	uint32_t style_min_height = this->style.min_height.get_type() == styles::digit_type::DT_PIXEL
		? this->style.min_height.get_value() : this->style.min_height.get_value() * ph / 100;

	this->height = fmin(
		fmax(style_height, style_min_height),
		fmin(ph - ppadding_y, style_max_height)
	);
	this->width = fmin(
		fmax(style_width, style_min_width),
		fmin(pw - ppadding_x, style_max_width)
	);



	//! Вонючие вы винючки, чтобы вас всех поразило бессилие!
	// int err = wresize(this->handle, nh, nw);
	// int err1 = mvwin(this->handle, ny, nx);
	// if(err != OK) {
	// 	mvprintw(17, 15, "Can't resize window: %s", strerror(errno));
	// 	void* obj = malloc(1024 * 1024 * 5);
	// 	if(obj) {
	// 		free(obj);
	// 	} else {
	// 		mvprintw(20, 15, "Can't alloc mem: %s", strerror(errno));
	// 	}
	// }
	if(cwidth == width && cheight == height && cx == nx && cy == ny) return;
	if ((cx != nx || cy != ny) && cwidth == width && cheight == height)
		return (void)mvwin(this->handle, cy = ny, cx = nx);
	delwin(this->handle);
	this->handle = newwin(this->height, this->width, ny, nx);

	this->cx = nx;
	this->cy = ny;
	this->cwidth  = this->width;
	this->cheight = this->height;
}

window::window(const styles::styles& style) {
	this->style = style;

	this->handle = newwin(1, 1, 0, 0);
	if(!this->handle) throw std::runtime_error("Handle creating have failed");

	this->refresh_size();
}
window::~window() {
	delwin(this->handle);

	if(this->parent) this->parent->remove(this);
	this->parent = nullptr;
}

uint32_t window::get_width() const {
	return width;
}
uint32_t window::get_height() const {
	return height;
}
WINDOW* window::get_handle() const {
	return handle;
}

div* window::get_parent() const {
	return this->parent;
}

void window::clear() {
	werase(this->handle);
	touchwin(this->handle);
	wnoutrefresh(this->handle);
}

void window::color_win() {
	if(!can_change_color()) return;

	// init_color(
	// 	TEXT_COLOR_ID,
	// 	1000 * ((style.color >> (hex2bin_koef * 2)) & color_mask) / 255,
	// 	1000 * ((style.color >>  hex2bin_koef)      & color_mask) / 255,
	// 	1000 * (style.color                         & color_mask) / 255
	// );
	auto pair = style.color_pair;
	for(auto filter : style.color_pair_filters) pair |= filter;
	wattrset(this->handle, pair);
}

// class div : public
div::~div() {
	this->children.clear();
};

void div::append(window* win) {
	this->children.push_back(win);
	werase(win->handle);

	win->rewrite_parent(this);
};
bool div::remove(const window* win) {
	auto it = std::find(this->children.begin(), this->children.end(), win);
	if(it == this->children.end()) return false;

	(*it)->rewrite_parent(nullptr);
	werase((*it)->handle);

	this->children.erase(it);
	return true;
};

decltype(div::children) div::get_children() {
	return std::vector<window*>(this->children);
};

void div::print() {
	//this->refresh_size();

	std::sort(this->children.begin(), this->children.end(), [](const window* win1, const window* win2) {
		return win1->style.pos_z < win2->style.pos_z && win2->style.position != styles::keywords::SK_STATIC;
	});

	uint32_t padding_y, padding_x;
	// Закостылила, надо как-то будет это исправить. см. window::refresh_size()
	getbegyx(this->handle, padding_y, padding_x);
	for(size_t i = 0; i < height; i += 1) {
		for(size_t j = 0; j < width; j += 1) {
      mvwaddch(this->handle, i, j, ' ');
    }
	}
	this->color_win();
	wnoutrefresh(this->handle);

	for(window* win : this->children) {
		if(!win->parent) continue;

		if(win->style.position != styles::keywords::SK_FIXED) {
			if(this->style.align == styles::keywords::SK_VERTICAL)
				padding_y += win->style.margin_top;
			else padding_x += win->style.margin_left;

			win->ppadding_y = padding_y;
			win->ppadding_x = padding_x;

			win->refresh_size();

			// Means
			// "Not above then (this->height - paddding_y), style.max_height
			// and not less then style.min_height"
			// style.max_height have priority
			// uint32_t style_width = win->style.width.get_type() == styles::digit_type::DT_PIXEL
			// 	? win->style.width.get_value() : win->style.width.get_value() * this->width / 100;
			// uint32_t style_height = win->style.height.get_type() == styles::digit_type::DT_PIXEL
			// 	? win->style.height.get_value() : win->style.height.get_value() * this->height / 100;

			// uint32_t style_max_width = win->style.max_width.get_type() == styles::digit_type::DT_PIXEL
			// 	? win->style.max_width.get_value() : win->style.max_width.get_value() * this->width / 100;
			// uint32_t style_max_height = win->style.max_height.get_type() == styles::digit_type::DT_PIXEL
			// 	? win->style.max_height.get_value() : win->style.max_height.get_value() * this->height / 100;

			// uint32_t style_min_width = win->style.min_width.get_type() == styles::digit_type::DT_PIXEL
			// 	? win->style.min_width.get_value() : win->style.min_width.get_value() * this->width / 100;
			// uint32_t style_min_height = win->style.min_height.get_type() == styles::digit_type::DT_PIXEL
			// 	? win->style.min_height.get_value() : win->style.min_height.get_value() * this->height / 100;

			// win->height = fmin(
			// 	fmax(style_height, style_min_height),
			// 	fmin(this->height - padding_y, style_max_height)
			// );
			// win->width = fmin(
			// 	fmax(style_width, style_min_width),
			// 	fmin(this->width - padding_x, style_max_width)
			// );

			if(this->style.align == styles::keywords::SK_VERTICAL)
				padding_y += win->height + win->style.margin_bottom;
			else padding_x += win->width + win->style.margin_right;
		} else {
			win->ppadding_y = win->style.margin_top;
			win->ppadding_x = win->style.margin_left;
		}

		if(win->callback) win->callback();
		win->print();
	}
}

void div::clear() {
	for(window* win : this->children) {
		win->clear();
	}
	werase(this->handle);
	touchwin(this->handle);
	wnoutrefresh(this->handle);
}

// class p : private
p::~p() {
	this->parent = nullptr;
}

std::string p::align_line(std::string& line, int width) {
	switch (this->style.text_align) {
	case styles::keywords::SK_RIGHT:
		for (int i = 0; i < line.length() - width; i += 1) {
			line = ' ' + line;
		}
		break;
	case styles::keywords::SK_CENTER:
		for (int i = 0; i < (line.length() - width) / 2; i += 1) {
			line = ' '  + line;
		}
	case styles::keywords::SK_LEFT:
	default:
		for (int i = 0; i < line.length() - width; i += 1) {
			line += ' ';
		}
	};

	return line;
}

// class p : public
void p::print() {
	if(!this->handle) throw std::runtime_error("Can't use without window handle");
	//this->refresh_size();

	if(!this->style.is_visible || this->width == 0 || this->height == 0) {
		werase(this->handle);
		wnoutrefresh(this->handle);
		return;
	}
	int x, y;
	getmaxyx(this->handle, y, x);

	int content_width  = this->width  - style.padding_left - style.padding_right;
	int content_height = this->height - style.padding_top - style.padding_bottom;

	std::string txt = (style.autotrim) ? utility::trim(this->inner_text) : this->inner_text;
	txt = std::regex_replace(txt, std::regex("\\a|\\b|\\f|\\n|\\r|\\t|\\v"), "");

	std::string result = "";

	for (int i = 0; i < content_height /* && txt.length()*/; i += 1) {
		//std::string line = txt.substr(0, content_width);
		std::string line;
		if(txt.length()) line = txt.substr(0, content_width);

		for (int i = 0; i < style.padding_left; i += 1) {
			line = ' ' + line;
		}
		for (int i = 0; i < style.padding_right; i += 1) {
			line += ' ';
		}

		line = this->align_line(line, content_width);

		result += line;

		if(txt.length() <= content_width) txt.clear();
		else {
			txt = txt.substr(content_width);
		}
	}

	this->color_win();
	mvwprintw(this->handle, style.padding_top, style.padding_left, "%s", result.c_str());

	touchwin(this->handle);
	wnoutrefresh(this->handle);
};

//class progress : public
void progress::print() {
	if(!this->handle) throw std::runtime_error("Can't use without window handle");
	//this->refresh_size();

	if(!this->style.is_visible || this->width == 0 || this->height == 0) {
		werase(this->handle);
		wnoutrefresh(this->handle);
		return;
	}

	int content_width  = this->width  - style.padding_left - style.padding_right;
	int content_height = this->height - style.padding_top - style.padding_bottom;

	this->value = fmax(fmin(max, value), min);
	int progress = content_width * (value - min) / (max - min);
	std::string line = "";
	std::string result = "";

	for(int i = 0; i < progress; i += 1) line += this->fill;
	for(int i = 0; i < content_width - progress; i += 1) line += ' ';
	for(int i = 0; i < content_height; i += 1) result += line;

	this->color_win();
	mvwprintw(this->handle, style.padding_top, style.padding_left, "%s", result.c_str());

	touchwin(this->handle);
	wnoutrefresh(this->handle);
}

progress::~progress() {
	this->parent = nullptr;
}

// Windows get_type() funtion defining
win_type window::get_type() const {
	return this->type;
}
win_type p::get_type() const {
	return this->type;
}
win_type div::get_type() const {
	return this->type;
}
win_type progress::get_type() const {
	return this->type;
}

}
