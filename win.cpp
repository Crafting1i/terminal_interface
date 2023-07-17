#include "win.h"
#include "util.h"

#include <iostream>

#include <string>
#include <cmath>

namespace win {
// class window : protected
void window::rewrite_parent(div* parent) {
	if(this->parent) this->parent->remove(this);
	this->parent = parent;

	delwin(this->handle);

	this->handle = derwin(parent ? parent->handle : stdscr, 1, 1, 0, 0);
	if(!this->handle) throw std::runtime_error("Handle creating have failed");

	this->refresh_size();
}

// class window : public
void window::refresh_size() {
	if(!this->handle) throw std::runtime_error("Can't use without window handle");
	int ph, pw; // p = parent

	if(!parent) getmaxyx(stdscr, ph, pw);
	else {
		ph = parent->get_height();
		pw = parent->get_width();
	}

	int nx = this->ppadding_x ? this->ppadding_x : fmin(style.margin_left, pw);
	int ny = this->ppadding_y ? this->ppadding_y : fmin(style.margin_top,  ph);


	int nh = fmin(ph + ny, style.height + ny) - ny;
	int nw = fmin(pw + nx, style.width + nx)  - nx;

	int mvcode;
//	if(this->parent) mvcode = mvderwin(this->handle, ny, nx);
//	else mvcode = mvwin(this->handle, ny, nx);
	mvwin(this->handle, ny, nx);
	wresize(this->handle, nh, nw);

	//mvprintw(7 + ny, 20, "%d", mvcode == OK);

	//int x, y;
	//getbegyx(this->handle, y, x);
	//mvprintw(7 + ny, 0, "(%d;%d) %dx%d", nx, ny, nw, nh);

	this->width  = nw;
	this->height = nh;
};

window::window(const styles::styles& style) {
	this->style = style;

	this->handle = newwin(1, 1, 0, 0);
	if(!this->handle) throw std::runtime_error("Handle creating have failed");

	this->refresh_size();
};
window::~window() {
	delwin(this->handle);

	if(this->parent) this->parent->remove(this);
	this->parent = nullptr;
};

int window::get_width() const {
	return width;
};
int window::get_height() const {
	return height;
};

// class div : public
div::~div() {
	this->children.clear();
};

void div::append(window* win) {
	this->children.push_back(win);
	win->rewrite_parent(this);
};
bool div::remove(const window* win) {
	auto it = std::find(this->children.begin(), this->children.end(), win);
	if(it == this->children.end()) return false;

	(*it)->rewrite_parent(nullptr);
	this->children.erase(it);
	return true;
};

void div::print() {
	this->refresh_size();

	int padding_y, padding_x;

	// Закостылила, надо как-то будет это исправить. см. window::refresh_size()
	getbegyx(this->handle, padding_y, padding_x);
	for(window* win : this->children) {
		if(!win->parent) continue;

		if(win->style.display != styles::keywords::SK_FIXED) {
			win->ppadding_x = padding_x;

//			mvprintw(10, win->style.margin_top, "%d", win->get_height());

			padding_y += win->style.margin_top;
			win->ppadding_y = padding_y;
			padding_y += win->get_height() + win->style.margin_bottom;
		}

		if(win->callback) win->callback(win);

		win->print();
	}
};

// class p : public
p::~p() {
	this->parent = nullptr;
};

void p::print() {
	if(!this->handle) throw std::runtime_error("Can't use without window handle");
	this->refresh_size();

	int content_width  = this->width  - style.padding_left - style.padding_right;
	int content_height = this->height - style.padding_top - style.padding_bottom;

	std::string txt = (style.autotrim) ? utility::trim(this->inner_text) : this->inner_text;
	std::string result = "";

	while (txt.length()) {
		std::string line = txt.substr(0, content_width);

		for (int i = 0; i < style.padding_left; i += 1) {
			line = ' ' + line;
		}
		for (int i = 0; i < style.padding_right; i += 1) {
			line += ' ';
		}

		switch (style.text_align) {
		case styles::keywords::SK_RIGHT:
			for (int i = 0; i < line.length() - content_width; i += 1) {
				line = ' ' + line;
			}
			break;
		case styles::keywords::SK_CENTER:
			for (int i = 0; i < (line.length() - content_width) / 2; i += 1) {
				line = ' '  + line;
			}

		case styles::keywords::SK_LEFT:
		default:
			for (int i = 0; i < line.length() - content_width; i += 1) {
				line += ' ';
			}
		};

		result += line;

		if(txt.length() <= content_width) txt.clear();
		else {
			txt = txt.substr(content_width);
			// result += "\n";
		}
	}

	const int TEXT_COLOR_ID = 1;
	const int BG_COLOR_ID   = 2;
	const int COLOR_PAIR_ID = 8;

	const int color_mask   = 0b1111'1111;
	const int hex2bin_koef = 8;

	init_color(
		TEXT_COLOR_ID,
		1000 * (((style.color >> (hex2bin_koef * 2)) & color_mask) / 255),
		1000 * (((style.color >> hex2bin_koef) & color_mask) / 255),
		1000 * ((style.color & color_mask) / 255)
	);

	init_color(
		BG_COLOR_ID,
		1000 * (((style.background_color >> (hex2bin_koef * 2)) & color_mask) / 255),
		1000 * (((style.background_color >> hex2bin_koef) & color_mask) / 255),
		1000 * ((style.background_color & color_mask) / 255)

	);
	init_pair(COLOR_PAIR_ID, TEXT_COLOR_ID, BG_COLOR_ID);

	wattrset(this->handle, COLOR_PAIR(COLOR_PAIR_ID));

	mvwprintw(this-> handle, style.padding_top, style.padding_left, "%s", result.c_str());

	wrefresh(this->handle);
};

}
