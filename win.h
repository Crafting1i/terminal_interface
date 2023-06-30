#include <ncurses.h>

#include "styles.h"
#include <functional>
#include <cmath>

namespace win {

class window {
private:
	int width, height, x, y;
	WINDOW* handle;
	window* parrent;
	std::vector<window*> children;

public:
	styles::styles style;
	std::function<void(window*)> callback;

public:
	window(const window&) = delete;
	window& operator=(const window&) = delete;

	window(int width, int height, int x, int y, window* parrent = nullptr);
	~window();

	int get_x();
	int get_y();
	int get_width();
	int get_height();

	bool print(const char* text);
};

}
