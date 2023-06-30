#include "win.h"
#include "util.h"

#include <vector>

#include <thread>
#include <mutex>
#include <atomic>

namespace engine {
	class windows_manager {
	private:
		std::vector<win::window*> windows;
	public:
		void add_win(win::window* win);
		void remove_win(win::window* win);

		decltype(windows_manager::windows) get_windows();
	};

	class engine {
	private:
		std::mutex mutex;
		std::vector<std::thread*> threads;

		std::atomic<bool> is_working = false;
	public:
		windows_manager wm;
		utility::event<void, int, int> on_key_pressed;

	private:
		void frame_request();
		void init_thread(std::function<void(std::mutex&)> cb);

	public:
		engine() {};

		void init();
		void start();

		void stop();
	};
}

