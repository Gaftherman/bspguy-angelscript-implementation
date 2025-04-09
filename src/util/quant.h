#include "colors.h"
#include <vector>
#include <unordered_map>

std::vector<COLOR3> median_cut_quantize(COLOR3* pixels, int pixel_count, int k=256);