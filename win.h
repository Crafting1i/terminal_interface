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

enum win_type {
	wt_none, wt_div, wt_p, wt_progress, wt_input, wt_button
};

class window {
	friend class div;
protected:
	uint32_t cwidth, cheight, cx, cy; //! c = computed
	uint32_t width = -1, height = -1;

	WINDOW* handle = nullptr;
	div* parent = nullptr;

	bool is_focus = false;

	uint32_t ppadding_x = 0, ppadding_y = 0;
public:
	styles::styles style;
	const win_type type = wt_none;
	std::function<void()> callback;
protected:
	void rewrite_parent(div* parent);
	void color_win();
	std::string align_line(std::string& str, int width);

public:
	window(const window& win) = delete;
	window& operator=(const window&) = delete;

	window(const styles::styles& style = {});
	virtual ~window();

	uint32_t get_width() const;
	uint32_t get_height() const;
	virtual win_type get_type() const;
	WINDOW* get_handle() const;

	void set_parent();
	div* get_parent() const;

	// Sized gots from the style
	void refresh_size();

	virtual void print() = 0;
	virtual void clear();

};

class div : public window {
private:
	std::vector<window*> children;

public:
	div(const styles::styles& style = {}): window(style) {};
	virtual ~div();

	void append(window* win);
	bool remove(const window* win);

	decltype(children) get_children();

	virtual void print();
	virtual void clear();
	virtual win_type get_type() const;
};

class p : public window {
public:
	std::string inner_text;

public:
	p(const styles::styles& style = {}): window(style) {};
	virtual ~p();

	virtual void print();
	virtual win_type get_type() const;
};

class progress : public window {
public:
	long long max = 100, min = 0, value = 0;
	char fill = '#';

public:
	progress(const styles::styles& style = {}): window(style) {};
	virtual ~progress();

	virtual void print();
	virtual win_type get_type() const;
};

class input : public window {
public:
enum input_type {
	common, password
};
public:
	std::string value;
	input_type type = input_type::common;

public:
	input(const styles::styles& style = {}): window(style) {};
	virtual ~input();

	virtual void print();
	virtual win_type get_type() const;
};

class button : public window {
public:
	std::string value;
	utility::event<void> on_pressed;

public:
	button(const styles::styles& style = {}): window(style) {};
	virtual ~button();

	virtual void print();
	virtual win_type get_type() const;

	void press() {
		on_pressed.call();
	}
};

}
