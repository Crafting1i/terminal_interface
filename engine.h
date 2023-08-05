#include "win.h"
#include "util.h"

#include <vector>

#include <thread>
#include <mutex>
#include <atomic>

namespace engine {
	class windows_selector {
	private:
		win::window* focused = nullptr;
		size_t selected_index = 0;
		styles::styles past_styles;
	public:
		explicit windows_selector(win::window* win): focused(win) {};
		~windows_selector();

		int get_selected_index() const;
		win::window* get_focused() const;

		win::window* list_down();
		win::window* list_up();
		win::window* select();
		win::window* unselect();
	};

	class engine {
	private:
		std::mutex mutex;
		std::vector<std::thread*> threads;
		windows_selector* ws = nullptr;

		std::atomic<bool> is_working = false;
	public:
		win::div* div = nullptr;
		utility::event<void, int, int, int, int> on_key_pressed;

	private:
		void init_thread(std::function<void(std::mutex&)> cb);

	public:
		engine() {};

		void init();
		void start();

		void stop();
	};
}

