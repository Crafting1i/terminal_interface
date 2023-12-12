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

namespace ami {
	/// @brief Class used by the engine to select windows. You should not use it directly.
	class windows_selector {
	private:
		ami::window* focused = nullptr;
		ami::p* info = nullptr;
		size_t selected_index = 0;
		size_t focused_children_size = 0;
		styles::styles past_styles;

	public:
		/// @brief Key used to select windows
		ami::key key_page_up = "\u001B[5~",
			key_page_down = "\u001B[6~", key_insert = "\u001B[2~",
			key_end = "\u001B[F",
			key_arrow_up = "\u001B[A", key_arrow_down = "\u001B[B",
			key_arrow_right = "\u001B[C", key_arrow_left = "\u001B[D";
	private:
		/// @brief Selects (index of) next window from childern. For select use `select()`
		/// @details Selects (index of) next window from childern (if has) or first window if it's end of list. For select use `select()` 
		/// @return Pointer to the current selected window
		ami::window* list_down() noexcept;
		/// @brief Selects (index of) previous window from childern. For select use `select()`
		/// @details Selects (index of) previous window from childern (if has) or last window if it's begin of list. For select use `select()` 
		/// @return Pointer to the current selected window
		ami::window* list_up() noexcept;
		/// @brief Selects children of current selected window if has. Otherwise does nothing.
		/// @return If successful, returns a pointer to the new selected window, otherwise to the currently selected window 
		ami::window* select();
		
		/// @brief Selects parent of current selected window if has. Otherwise does nothing.
		/// @return If successful, returns a pointer to the new selected window, otherwise to the currently selected window 
		ami::window* unselect();

	public:
		// Creates new selector. Do not use it directly.
		explicit windows_selector(ami::window* win);
		~windows_selector();

		/**
		 * @brief Get the selected index window
		 * @return Index of the selected window
		 */
		int get_selected_index() const noexcept;
		/**
		 * @brief Get a pointer of the focused window
		 * @return Pointer to the focused window
		 */
		ami::window* get_focused() const noexcept;

		/**
		 * @brief Moves the window to the appropriate coordinates
		 * @param x The distance along the X coordinate that the window should move
		 * @param y The distance along the Y coordinate that the window should move
		 */
		void move_focused(int x, int y);

		/**
		 * @brief Handles a button click for focused window. Typically you shouldn't use this function explicitly
		 * @param key Button handler used to handle
		 */
		void update(const ami::key& key);
	};

	class engine {
	public:
		ami::threads::threads_pool threads_pool;
		// The base of windows hierarchy. Tipycllyt you should not redefine this.
		ami::div* div = nullptr;
		// Event listener for handing key events.
		utility::event<void, const ami::key&> on_key_pressed;
		uint8_t MAX_FPS = 15;

	private:
		std::atomic<bool> is_working = false;
		std::mutex mutex_widows;
		std::mutex mutex_on_key_pressed;
		windows_selector* ws = nullptr;

	public:
		/// @brief Creating engine instance
		/// @param threads_count Count of threads. Minimum is 1 (even if less is specified).
		engine(int threads_count):
			threads_pool(ami::threads::threads_pool(fmax(threads_count, 1)))  {};

		/// @brief Initialize the engine
		void init();
		/// @brief Starts the rendering. Must be called in end of `main()` function.
		void start();
		/// @brief Stops rendering and destructs the engine
		void stop();

		/// @return pointer to `windows_selector` instance. 
		windows_selector* get_ws() const noexcept;
		
	private:
		void stop_eng();
	};
}
