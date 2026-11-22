#pragma once

#include <glm.hpp>
using namespace glm;

struct glPointLight
{
	vec4 point;
	glPointLight(vec3 pt) {
		point = vec4(pt, 1);
	}
};

struct glAreaLight
{
	vec4 p[4];
	glAreaLight(vec4 pt[4]) {
		memcpy(p, pt, sizeof(vec4) * 4);
	}
};