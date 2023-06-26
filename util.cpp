#include "util.h"

#include <stdio.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <regex>

const char* const PATH_BATT_CHARGE_NOW  = "/sys/class/power_supply/BAT1/energy_now";
const char* const PATH_BATT_CHARGE_FULL = "/sys/class/power_supply/BAT1/energy_full";

namespace utility {
	double get_batt_level() {
		FILE* fcharge_now = fopen(PATH_BATT_CHARGE_NOW, "r");
		FILE* fcharge_full = fopen(PATH_BATT_CHARGE_FULL, "r");

		long unsigned int batt_max_mAh = 0;
		long unsigned int batt_now_mAh = 0;

		if (!fcharge_now) return -1;
		if (!fcharge_full) {
			fclose(fcharge_now);
			return -1;
		}

		fscanf(fcharge_full, "%lu", &batt_max_mAh);
		fscanf(fcharge_now,  "%lu", &batt_now_mAh);

		fclose(fcharge_full);
		fclose(fcharge_now);

		return 100.0 * ((double)batt_now_mAh / batt_max_mAh);
	}

	std::vector<std::string> split(const std::string& str, std::string_view pattern) {
		const auto r = std::regex(pattern.data());
		return std::vector<std::string> {
			std::sregex_token_iterator(cbegin(str), cend(str), r, -1),
			std::sregex_token_iterator()
		};
	}

	std::string to_string(double d, int precision) {
		std::stringstream ss;
		ss << std::setprecision(precision) << std::fixed << d;
		return ss.str();
	}

	std::string trim(const std::string& s) {
		return trim_begin(trim_end(s));
	}
	std::string trim_begin(const std::string& s) {
		std::regex regex { "^\\s+", std::regex_constants::ECMAScript };
		std::smatch m;
		std::regex_search(s, m, regex);

		std::string str = s;
		if(!m.empty()) {
			str = s.substr(0, m.position());
		}

		return str;
	}
	std::string trim_end(const std::string& s) {
		std::regex regex { "\\s+$", std::regex_constants::ECMAScript };
		std::smatch m;
		std::regex_search(s, m, regex);

		std::string str = s;
		if(!m.empty()) {
			str = s.substr(0, m.position());
		}

		return str;
	}


	/*
	template<typename r_t, typename ...a_t> void event<r_t, a_t...>::attach(event<r_t, a_t...>::callback_t cb) {
		this->dispatcher.push_back(cb);
	}
	template<typename r_t, typename ...a_t> void event<r_t, a_t...>::operator()(event<r_t, a_t...>::callback_t cb) {
		this->dispatcher.push_back(cb);
	}

	template<typename r_t, typename ...a_t> void event<r_t, a_t...>::call(a_t ...args) {
		for(auto cb : this->dispatcher) {
			cb(args...);
		}
	}*/

}
