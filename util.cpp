#include "util.h"

#include <stdio.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <regex>

const char* const PATH_BATT_CHARGE_NOW  = "/sys/class/power_supply/BAT1/energy_now";
const char* const PATH_BATT_CHARGE_FULL = "/sys/class/power_supply/BAT1/energy_full";

namespace ami {
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

	/*
	template<typename Iter> void bubble_sort_cb(
		Iter begin, Iter end,
		std::function<bool(
			const typename std::iterator_traits<Iter>::value_type&,
			const typename std::iterator_traits<Iter>::value_type&
		)> cb
	) {
		bool is_swapped = false;
		for(auto it_i = begin; it_i != end; it_i++) {
			is_swapped = false;
			for(auto it_j = it_i; it_j != end && it_j + 1 != end; it_j++) {
				if(cb(it_j, it_j + 1)) {
					std::swap(&(*it_j), &(*(it_j + 1)));
					is_swapped = true;
				}
			}

			if(!is_swapped) break;
		}
	}

	std::string to_string(const std::vector<int>& vec) {
		std::string res;

		for(int i : vec) {
			if(res.size()) res += ", ";
			res += std::to_string(i);
		}

		return res;
	}
	*/

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

}
}
