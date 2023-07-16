#include <ncurses.h>

#include "styles.h"
#include <functional>
#include <cmath>

namespace win {

class window {
	friend class div;
protected:
	int width, height;
	WINDOW* handle = nullptr;
	const window* parent = nullptr;

	uint32_t ppadding_x = 0, ppadding_y = 0;

public:
	styles::styles style;
	std::function<void(window*)> callback;

protected:
	void rewrite_parent(const window* parent);

public:
	window(const window& win) = delete;
	window& operator=(const window&) = delete;

	window(const styles::styles& style = {});
	virtual ~window();

	int get_width() const;
	int get_height() const;

	void set_parent();

	// Sized gots from the style
	void refresh_size();

	virtual void print() = 0;
};

class div : public window {
	private:
		std::vector<window*> children;

	public:
		div(const styles::styles& style = {}): window(style) {};
		virtual ~div();

		void append(window* win);
		bool remove(const window* win);

		virtual void print();
};

class p : public window {
public:
	std::string inner_text;

public:
	p(const styles::styles& style = {}): window(style) {};
	virtual ~p();

	virtual void print();
};

}
