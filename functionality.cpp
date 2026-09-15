
#include "chrono"
#include "functionality.hpp"

uint64_t seed = 0;

uint64_t xorshift64() {
	if (!seed) seed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

	seed ^= seed << 13;
	seed ^= seed >> 7;
	seed ^= seed << 17;
	return seed;
}
