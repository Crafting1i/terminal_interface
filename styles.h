#pragma once

#include <string>
#include <climits>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <unordered_set>


namespace ami {
namespace styles {

enum keywords {
	SK_LEFT, SK_RIGHT, SK_CENTER,
	SK_STATIC, SK_FIXED,
	SK_VERTICAL, SK_HORIZONTAL,
	SK_NOFILTER, SK_REVERSE, SK_BOLD, SK_ITALIC, 
};
enum class digit_type {
	DT_PIXEL, DT_PERCENT
};

class s_digit {
private:
	uint32_t value;
	digit_type type = digit_type::DT_PIXEL;

public:
	s_digit(unsigned int val, digit_type t = digit_type::DT_PIXEL): type(t) {
		if(type == digit_type::DT_PERCENT) {
			this->value = fmin(val, 100);
		} else this->value = val;
	}

	s_digit& operator=(unsigned int val) {
		if(type == digit_type::DT_PERCENT) {
			this->value = fmin(val, 100);
		} else this->value = val;

		return *this;
	}

	digit_type get_type() {
		return this->type;
	}
	uint32_t get_value() {
		return this->value;
	}

	/*explicit operator uint32_t() const {
		return this->value;
	}*/
	explicit operator bool() const {
		return this->value > 0;
	}
	bool operator>(const s_digit& d) const {
		if(this->type != d.type) return false;
		return this->value > d.value;
	}
	bool operator<(const s_digit& d) const {
		if(this->type != d.type) return false;
		return this->value < d.value;
	}

	s_digit operator+(const s_digit& d) {
		if(this->type != d.type) throw std::logic_error("Bad operation with s_digit of different types");
		return s_digit(this->value + d.value, this->type);
	}
	s_digit operator-(const s_digit& d) {
		if(this->type != d.type) throw std::logic_error("Bad operation with s_digit of different types");
		return s_digit(this->value - d.value, this->type);
	}
};

class styles {
public:
	styles() {};
	s_digit width     { 1 },        height     { 1 };
	s_digit max_width { UINT_MAX }, max_height { UINT_MAX };
	s_digit min_width { 0 },        min_height { 0 };


	unsigned int padding_top = 0,    padding_right = 0,
		padding_bottom = 0, padding_left = 0;
	unsigned int margin_top = 0,     margin_right = 0,
		margin_bottom = 0,  margin_left = 0;

	std::unordered_set<chtype> color_pair_filters {};
	int color_pair = COLOR_PAIR(0);
	unsigned int pos_z = 0;

	bool autotrim   = true;
	bool is_visible = true;
	bool is_moveble = true;

	keywords text_align = keywords::SK_LEFT; // left, right, center
	keywords position = keywords::SK_STATIC; // static, fixed
	keywords align = keywords::SK_VERTICAL;  // vertical, horizontal
};


}
}
