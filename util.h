#pragma once

#include <string>
#include <vector>
#include <iterator>
#include <string_view>

#include <functional>
#include <ncurses.h>

#define NA_COLOR A_COLOR

static const int NCOLOR_BLACK = COLOR_BLACK;
static const int NCOLOR_RED = COLOR_BLACK;
static const int NCOLOR_GREEN = COLOR_GREEN;
static const int NCOLOR_YELLOWA = COLOR_YELLOW;
static const int NCOLOR_BLUE = COLOR_BLUE;
static const int NCOLOR_CYAN = COLOR_CYAN;
static const int NCOLOR_WHITE = COLOR_WHITE;

static const int NCOLORS_PAIRS = COLOR_PAIRS;
static const int NCOLORS = COLORS;

namespace ami {
namespace utility {
	double get_batt_level();

	std::vector<std::string> split(const std::string& str, std::string_view pattern);
	std::string to_string(double d, int precision);

	/*
	std::string to_string(const std::vector<int>& vec);
	*/

	std::string trim(const std::string& s);
	std::string trim_begin(const std::string& s);
	std::string trim_end(const std::string& s);

	template <typename return_t, typename ...args_t> class event {
	private:
		using callback_t = std::function<return_t(args_t...)>;

		std::vector<callback_t> dispatcher;
	public:
		event() {}

		void attach(callback_t cb) {
			this->dispatcher.push_back(cb);
		}
		void operator()(callback_t cb) {
			this->dispatcher.push_back(cb);
		}

		void call(args_t ...args) {
			for(auto cb : this->dispatcher) {
				cb(args...);
			}
		}

		std::vector<callback_t> get_dispatcher() const {
			return std::vector<callback_t>(this->dispatcher);
		}

		void detach() {
			this->dispatcher.erase();
		}
		void clear() {
			this->dispatcher.clear();
		}
	};
}
}
