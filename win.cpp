#include "win.h"
#include "util.h"

#include <iostream>

#include <string>
#include <cmath>

namespace win {

// class window: public
void window::refresh_size() {
	int ph, pw; // p = parent

	if(!parent) getmaxyx(stdscr, ph, pw);
	else {
		ph = parent->get_height();
		pw = parent->get_width();
	}

	int nx = fmin(style.margin_left, pw);
	int ny = fmin(style.margin_top, ph);

	int nh = fmin(ph, style.height + this->ppadding_y + ny) - ny;
	int nw = fmin(pw, style.width + this->ppadding_x + nx) - nx;

	mvwin(this->handle, ny, nx);
	wresize(this->handle, nh, nw);

	this->width  = nw;
	this->height = nh;
};

window::window(const styles::styles& style) {
	this->style = style;

	this->handle = derwin(
		this->parent ? this->parent->handle : stdscr,
		1, 1, 0, 0
	);
	if(!this->handle) throw std::runtime_error("Handle creating have failed");

	this->refresh_size();
};
window::~window() {
	delwin(this->handle);
	this->parent = nullptr;
};

int window::get_width() {
	return width;
};
int window::get_height() {
	return height;
};

// class div : public
div::~div() {
	for(window* win : this->children) {
		//delete win;
	}
};

void div::append(window* win) {
	this->children.push_back(win);
};
bool div::remove(const window* win) {
	auto it = std::find(this->children.begin(), this->children.end(), win);
	if(it == this->children.end()) return false;

	this->children.erase(it);
	return true;
};

void div::print() {
	this->refresh_size();

	for(window* win : this->children) {
		if(win->callback) win->callback(win);
		win->print();
	}
};

// class p : public
p::~p() {
	this->parent = nullptr;
};

void p::print() {
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
	const int BG_COLOR_ID = 2;
	const int COLOR_PAIR_ID = 8;

	const int color_mask = 0b1111'1111;
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
