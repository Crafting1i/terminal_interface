#pragma once

#include <unordered_map>
#include <keys.h>

namespace ami {
  const std::unordered_map<std::string, const ami::key> KEYS {
		{ "A", "A" },
		{ "B", "B" },
		{ "ARROW_UP", "\u001B[A" },
		{ "ARROW_DOWN", "\u001B[B" },
		{ "ARROW_RIGHT", "\u001B[C" },
		{ "ARROW_LEFT", "\u001B[D" },
		{ "HOME", "\u001B[1~" },
		{ "INSERT", "\u001B[2~" },
		{ "DELETE", "\u001B[3~" },
		{ "END", "\u001B[4~" },
		{ "PAGE_UP", "\u001B[5~" },
		{ "PAGE_DOWN", "\u001B[6~" },
		{ "ESC", "\u001B"  },
		{ "ENTER", "\u2386" },
		{ "BACKSPACE", "\u0008" },
    { "ERR", { -1, -1, -1, -1 } }
	};
}