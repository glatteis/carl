#pragma once

#include "../SetCover.h"

namespace carl::covering::heuristic {

inline auto greedy(SetCover& sc) {
	Bitset result;
	Bitset uncovered = sc.get_uncovered();
	while (uncovered.any()) {
		auto s = sc.largest_set();
		result.set(s);
		uncovered -= sc.get_set(s);
		sc.select_set(s);
	}
	return result;
}

}