#include <string>
#include <climits>

namespace styles {

enum keywords {
	SK_LEFT, SK_RIGHT, SK_CENTER, SK_STATIC, SK_FIXED, SK_VERTICAL, SK_HORIZONTAL
};

class styles {
public:
	styles() {};
	unsigned int width = 1, height = 1;
	unsigned int max_width = UINT_MAX, max_height = UINT_MAX;
	unsigned int min_width = 0,        min_height = 0;


	unsigned int padding_top = 0,    padding_right = 0,
		     padding_bottom = 0, padding_left = 0;
	unsigned int margin_top = 0,     margin_right = 0,
		     margin_bottom = 0,  margin_left = 0;

	unsigned int color = 0xFFFFFF, background_color = 0x000000;
	unsigned int pos_z = 0;

	bool autotrim = true;
	bool is_visible = true;

	keywords text_align = keywords::SK_LEFT; // left, right, center
	keywords position = keywords::SK_STATIC;
	keywords align = keywords::SK_VERTICAL;
};

}
