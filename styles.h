#include <string>

namespace styles {

enum keywords {
	SK_LEFT, SK_RIGHT, SK_CENTER
};

class styles {
public:
	styles() {};

	unsigned int padding_top = 0, padding_right = 0,
		     padding_bottom = 0, padding_left = 0;
	unsigned int color = 0xFFFFFF, background_color = 0x000000;
	unsigned int z_pos = 0;

	bool autotrim = true;

	keywords text_align = keywords::SK_LEFT; // left, right, center
};

}
