#pragma once
#include <stdint.h>
#include "vectors.h"

#pragma pack(push, 1)
struct COLOR3
{
	uint8_t r, g, b;

	COLOR3() {}
	COLOR3(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
	vec3 toVec() { return vec3(r / 255.0f, g / 255.0f, b / 255.0f); }
};
#pragma pack(pop)
struct COLOR4
{
	uint8_t r, g, b, a;

	COLOR4() {}
	COLOR4(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
	COLOR4(COLOR3 c, uint8_t a) : r(c.r), g(c.g), b(c.b), a(a) {}
	vec4 toVec() { return vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f); }
};

COLOR3 operator*(COLOR3 v, float f);
bool operator==(COLOR3 c1, COLOR3 c2);

COLOR4 operator*(COLOR4 v, float f);
bool operator==(COLOR4 c1, COLOR4 c2);