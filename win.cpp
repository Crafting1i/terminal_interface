#include "win.h"
#include "util.h"

#include <iostream>

#include <string>
#include <cmath>

namespace win {

window::window(int width, int height, int x, int y, window* parrent) {
	this->parrent = parrent;
	int ph, pw; // p = parrent

	if(!parrent) getmaxyx(stdscr, ph, pw);
	else {
		ph = parrent->get_height();
		pw = parrent->get_width();
	}

	int nx = std::abs(fmin(x, pw));
	int ny = std::abs(fmin(y, ph));

	int nh = std::abs(fmin(ph, height + ny) - ny);
	int nw = std::abs(fmin(pw, width + nx) - nx);

	this->handle = newwin(nh, nw, ny, nx);

	this->width  = nw;
	this->height = nh;
	this->x = nx;
	this->y = ny;
};
window::~window() {
	this->parrent = nullptr;

	for(window* win : this->children) {
		//delete win;
	}
};

int window::get_x() {
	return x;
};
int window::get_y() {
	return y;
};
int window::get_width() {
	return width;
};
int window::get_height() {
	return height;
};

bool window::print(const char* text) {
	int content_width  = this->width  - style.padding_left - style.padding_right;
	int content_height = this->height - style.padding_top - style.padding_bottom;

	std::string txt = (style.autotrim) ? utility::trim(text) : text;
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

	return true;
};

}
