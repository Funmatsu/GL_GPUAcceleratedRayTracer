#pragma once
#include <glm.hpp>

using namespace glm;

struct ray
{
	vec3 origin;
	vec3 direct;
};

struct hit {
	float t;
	bool hit;
};