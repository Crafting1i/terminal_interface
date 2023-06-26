#include <string>
#include <vector>

#include <functional>

namespace utility {
	double get_batt_level();

	std::vector<std::string> split(const std::string& str, std::string_view pattern);
	std::string to_string(double d, int precision);

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
	};
}

