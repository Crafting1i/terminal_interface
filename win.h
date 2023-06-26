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

public:
	styles::styles style;
	std::function<void(void)> callback;

public:
	window(const window&) = delete;
	window& operator=(const window&) = delete;

	window(int width, int height, int x, int y, window* parrent = nullptr)
		: window((uint32_t)abs(width), (uint32_t)abs(height), (uint32_t)abs(x), (uint32_t)abs(y), parrent) {};
	window(uint32_t width, uint32_t height, uint32_t x, uint32_t y, window* parrent = nullptr);

	uint32_t get_x();
	uint32_t get_y();
	uint32_t get_width();
	uint32_t get_height();

	bool print(const char* text);

};

}
