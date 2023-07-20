#include "win.h"
#include "util.h"

#include <vector>

#include <thread>
#include <mutex>
#include <atomic>

namespace engine {
	class engine {
	private:
		std::mutex mutex;
		std::vector<std::thread*> threads;

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

