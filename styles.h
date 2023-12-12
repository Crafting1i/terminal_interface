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
	/// Experimental. Useless now
	SK_NOFILTER, SK_REVERSE, SK_BOLD, SK_ITALIC, 
};
enum class digit_type {
	DT_PIXEL, DT_PERCENT
};

/// @brief Digit type to compute windows' size.
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
	/// @return `true` if the value above then 0
	explicit operator bool() const {
		return this->value > 0;
	}
	/// @return `false` if defferent types or this value less than d else `true`
	bool operator>(const s_digit& d) const {
		if(this->type != d.type) return false;
		return this->value > d.value;
	}
	/// @return `false` if defferent types or this value above than d else `true`
	bool operator<(const s_digit& d) const {
		if(this->type != d.type) return false;
		return this->value < d.value;
	}

	/// @return Expressions' summ
	/// @throw `std::logic_error` if different types
	s_digit operator+(const s_digit& d) {
		if(this->type != d.type) throw std::logic_error("Bad operation with s_digit of different types");
		return s_digit(this->value + d.value, this->type);
	}
	/// @return Expressions' difference
	/// @throw `std::logic_error` if different types
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

	/// @brief `Padding` is inner padding of block. 
	unsigned int padding_top = 0,    padding_right = 0,
		padding_bottom = 0, padding_left = 0;
	/// @brief `Margin` is outer padding of block.
	unsigned int margin_top = 0,     margin_right = 0,
		margin_bottom = 0,  margin_left = 0;

	/// @brief Colors filters from Ncurses
	std::unordered_set<chtype> color_pair_filters {};
	/// @brief Color imagination from Ncurses
	int color_pair = COLOR_PAIR(0);
	/// @brief Affects rendering priority. Than bigger than later it renders
	unsigned int pos_z = 0;

	/// @brief Enable/disable implicit trimming
	bool autotrim   = true;
	/// @brief Make visible/invisible
	bool is_visible = true;
	/// @brief Allow/deny moving by `windows_selector`
	bool is_moveble = true;

	/// @brief Align strings. May be `LEFT`, `RIGHT`, or `CENTER`
	keywords text_align = keywords::SK_LEFT;
	/// @brief Position control. May be `STATIC` or `FIXED`
	keywords position = keywords::SK_STATIC;
	/// @brief Widows align (for `div`s). May be `VERTICAL` or `HORIZONTAL`
	keywords align = keywords::SK_VERTICAL;
};


}
}
