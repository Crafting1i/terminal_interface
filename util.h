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
	/// @return Battery percentage
	double get_batt_level();
	/// @brief Splits a string based on a regex pattern
	/// @param str String to split
	/// @param pattern Regex pattern to split
	/// @return Vector of strings
	std::vector<std::string> split(const std::string& str, std::string_view pattern);
	/// @brief Converts a number to a string with a certain precision (number of decimal places)
	/// @param d Number to convert
	/// @param precision Precision
	std::string to_string(double d, int precision);

	/*
	std::string to_string(const std::vector<int>& vec);
	*/

	/// @brief Delete the specified symbols begin and end of string.
	/// @param s Unformated string
	/// @return Trimed string
	std::string trim(const std::string& s);
	/// @brief Delete the specified symbols begin of string.
	/// @param s Unformated string
	/// @return Trimed string
	std::string trim_begin(const std::string& s);
	/// @brief Delete the specified symbols end of string.
	/// @param s Unformated string
	/// @return Trimed string
	std::string trim_end(const std::string& s);

	template <typename return_t, typename ...args_t> class event {
	private:
		using callback_t = std::function<return_t(args_t...)>;

		std::vector<callback_t> dispatcher;
	public:
		event() {}
		// Can't be copyed
		event(const event&) = delete;
		event& operator=(const event&) = delete;

		/// @brief Add a new function to the dispatcher
		/// @param cb Any callable object to be executed
		void attach(callback_t cb) {
			this->dispatcher.push_back(cb);
		}
		/// @brief Add a new function to the dispatcher
		/// @param cb Any callable object to be executed
		void operator()(callback_t cb) {
			this->dispatcher.push_back(cb);
		}

		/// @brief Call all registered callbacks
		/// @param ...args Arguments to be passed to callbacks
		void call(args_t ...args) const {
			for(auto cb : this->dispatcher) {
				cb(args...);
			}
		}

		/// @return Copy of dispatcher (vector of callbacks)
		std::vector<callback_t> get_dispatcher() const {
			return std::vector<callback_t>(this->dispatcher);
		}

		/// @brief Remove listener at given position.
		/// @param __position Iterator pointing to element to be erased.
		auto erase(typename std::vector<callback_t>::const_iterator __position) {
			return this->dispatcher.erase(__position);
		}
		/// @brief Remove a range of listeners.
		/// @param __first First element to remove
		/// @param __last Last element to remove
		auto erase(
			typename std::vector<callback_t>::const_iterator __first,
			typename std::vector<callback_t>::const_iterator __last
			) {
			return this->dispatcher.erase(__first, __last);
		}
		/// @brief Clears the dispatcher
		void clear() {
			this->dispatcher.clear();
		}
	};
}
}
