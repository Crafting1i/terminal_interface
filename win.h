#pragma once

#include <ncurses.h>

#include "util.h"
#include "styles.h"
#include <functional>
#include <cmath>

namespace ami {

class window;
class div;
class p;
class progress;
class input;
class button;

/// @brief types of windows
enum win_type {
	wt_none, wt_div, wt_p, wt_progress, wt_input, wt_button
};

/// @brief base class of windows. Used as an abstraction for a general representation of windows.
/// You shouldn't create an instance of it.
class window {
	friend class div;
protected:
	uint32_t cwidth, cheight, cx, cy; //! c = computed
	uint32_t width = -1, height = -1;

	/// @brief Handle for Ncurses window
	WINDOW* handle = nullptr;
	/// @brief Pointer to parrent window
	div* parent = nullptr;

	bool is_focus = false;

	/// @brief Parent window offset
	uint32_t ppadding_x = 0, ppadding_y = 0;
public:
	styles::styles style;
	const win_type type = wt_none;
	/// @brief Funtion which calls every time on frame renderes
	std::function<void()> callback;
protected:
	/// @brief Rewrites parent on window
	/// @param parent Pointer to parent window
	void rewrite_parent(div* parent);
	/// @brief Colorize text at window
	void color_win();
	/// @brief Alignы line
	/// @param str String to align
	/// @param width Width of line
	/// @return Aligned line string
	std::string align_line(std::string& str, int width);

public:
	// Can't be copyed
	window(const window& win) = delete;
	window& operator=(const window&) = delete;

	window(const styles::styles& style = {});
	virtual ~window();

	/// @return Width of the window
	uint32_t get_width() const noexcept;
	/// @return Height of the window
	uint32_t get_height() const noexcept;
	/// @return Type of the window
	virtual win_type get_type() const noexcept;
	/// @return Pointer to Ncurses handle of the window
	WINDOW* get_handle() const noexcept;

	/// @return Pointer to handle of the window's parent
	div* get_parent() const noexcept;

	/// @brief Resize and moves widnows by parameter from the style
	void refresh_size();

	/// @brief Method which prints window at the screan. Must be overridden by derived classes.
	virtual void print() = 0;
	/// @brief Erases a window from the screen
	virtual void clear();
};

class div : public window {
private:
	std::vector<window*> children;

public:
	div(const styles::styles& style = {}): window(style) {};
	virtual ~div();

	/// @brief Inserts a new window to self children and rewrites its parent
	/// @param win Pointer to widnow
	/// @throws `std::invalid_argument` if pointer to this windows or one of its parents 
	void append(window* win);
	bool remove(const window* win);

	/// @return Copy of vector with children
	decltype(children) get_children() const noexcept;

	virtual void print();
	virtual void clear();
	virtual win_type get_type() const noexcept;
};

class p : public window {
public:
	/// @brief Text content of the window
	std::string inner_text;

public:
	p(const styles::styles& style = {}): window(style) {};
	virtual ~p();

	virtual void print();
	virtual win_type get_type() const noexcept;
};

class progress : public window {
public:
	/// @brief Max value of the progress
	long long max = 100;
	/// @brief Min value of the progress. Can be negative
	long long	min = 0;
	/// @brief Current value of the progress
	long long value = 0;
	/// @brief Character which fills the progress
	char fill = '#';

public:
	progress(const styles::styles& style = {}): window(style) {};
	virtual ~progress();

	virtual void print();
	virtual win_type get_type() const noexcept;
};

class input : public window {
public:
/// @brief Input types. Change the behavior of the window.
enum input_type {
	/// @brief Do nothing
	common,
	/// @brief Replace all characters with `*`
	password
};
public:
	/// @brief Value of the input
	std::string value;
	/// @brief Type of the input
	input_type type = input_type::common;

public:
	input(const styles::styles& style = {}): window(style) {};
	virtual ~input();

	virtual void print();
	virtual win_type get_type() const noexcept;
};

class button : public window {
public:
	/// @brief Text on the button
	std::string value;
	/// @brief Listener which handles the button click event
	utility::event<void> on_pressed;

public:
	button(const styles::styles& style = {}): window(style) {};
	virtual ~button();

	virtual void print();
	virtual win_type get_type() const noexcept;

	/// @brief Make an emmit to listener
	void press() {
		on_pressed.call();
	}
};

}
