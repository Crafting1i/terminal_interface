#pragma once

#include "win.h"
#include "util.h"
#include "keys.h"
#include "mythreads.h"

#include <vector>

#include <thread>
#include <mutex>
#include <atomic>
#include <queue>

namespace engine {
	class windows_selector {
	private:
		win::window* focused = nullptr;
		win::p* info = nullptr;
		size_t selected_index = 0;
		size_t focused_children_size = 0;
		styles::styles past_styles;

		const keys::key key_page_up = "\u001B[5~",
			key_page_down = "\u001B[6~", key_insert = "\u001B[2~",
			key_end = "\u001B[F",
			key_arrow_up = "\u001B[A", key_arrow_down = "\u001B[B",
			key_arrow_right = "\u001B[C", key_arrow_left = "\u001B[D";
	private:
		win::window* list_down();
		win::window* list_up();
		win::window* select();
		win::window* unselect();

	public:
		explicit windows_selector(win::window* win);
		~windows_selector();

		int get_selected_index() const;
		win::window* get_focused() const;

		void move_focused(int x, int y);

		void update(const keys::key& key);
	};

	class engine {
	public:
		threads::threads_pool threads_pool;
		win::div* div = nullptr;
		utility::event<void, const keys::key&> on_key_pressed;

	private:
		std::atomic<bool> is_working = false;
		std::mutex mutex_widows;
		std::mutex mutex_on_key_pressed;
		windows_selector* ws = nullptr;
		uint8_t MAX_FPS = 15;

	public:
		engine(int threads_count):
			threads_pool(threads::threads_pool(fmax(threads_count, 1)))  {};

		void init();
		void start();
		void stop();
		
	private:
		void stop_eng();
	};
}
