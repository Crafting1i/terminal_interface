#include <ncurses.h>

#include "styles.h"
#include <functional>
#include <cmath>

namespace win {

class window {
protected:
	int width, height, x, y;
	WINDOW* handle;
	window* parent;

public:
	styles::styles style;
	std::function<void(window*)> callback;

public:
	window(const window&) = delete;
	window& operator=(const window&) = delete;

	window(int width, int height, int x, int y, window* parent = nullptr);
	virtual ~window();

	int get_x();
	int get_y();
	int get_width();
	int get_height();

	virtual void print() = 0;
};

class div : public window {
	private:
		std::vector<window*> children;

	public:
		div(int width, int height, int x, int y, window* parent = nullptr): window(width, height, x, y, parent) {};
		virtual ~div();

		void append(window* win);
		bool remove(const window* win);

		virtual void print();
};

class p : public window {
public:
	std::string inner_text;

public:
	p(int width, int height, int x, int y, window* parent = nullptr): window(width, height, x, y, parent) {};
	virtual ~p();

	virtual void print();
};

}
