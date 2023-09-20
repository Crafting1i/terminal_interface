#include <ncurses.h>

#include "styles.h"
#include <functional>
#include <cmath>

namespace win {

class window;
class div;
class p;

enum win_type {
	wt_none, wt_div, wt_p, wt_progress
};

class window {
	friend class div;
protected:
	int width, height;
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

public:
	window(const window& win) = delete;
	window& operator=(const window&) = delete;

	window(const styles::styles& style = {});
	virtual ~window();

	int get_width() const;
	int get_height() const;
	virtual win_type get_type() const;

	void set_parent();
	div* get_parent() const;

	// Sized gots from the style
	void refresh_size();

	virtual void print() = 0;
	virtual void clear();

};

class div : public window {
protected:
	const win_type type = win_type::wt_div;

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
protected:
	const win_type type = win_type::wt_p;

public:
	std::string inner_text;

private:
	std::string align_line(std::string& str, int width);

public:
	p(const styles::styles& style = {}): window(style) {};
	virtual ~p();

	virtual void print();
	virtual win_type get_type() const;
};

class progress : public window {
protected:
	const win_type type = win_type::wt_progress;

public:
	long long max = 100, min = 0, value = 0;
	char fill = '#';

public:
	progress(const styles::styles& style = {}): window(style) {};
	virtual ~progress();

	virtual void print();
	virtual win_type get_type() const;
};

}
