#pragma once
#include <glm.hpp>
using namespace glm;
struct glCamera
{
	vec4 forward;
	vec4 right;
	vec4 up;
	vec4 position;
	glCamera(vec4 fwd, vec4 rait, vec4 upp, vec4 pos_fov) {
		forward = fwd;
		right = rait;
		up = upp;
		position = pos_fov;
	}
	vec3 getForwardVec(vec2& yawpitch, vec2 mouseChange) {
		float turnspeed = 1;
		yawpitch.x += mouseChange.x * turnspeed;
		yawpitch.y -= mouseChange.y * turnspeed;

		if (yawpitch.y > 89.0f)
			yawpitch.y = 89.0f;
		if (yawpitch.y < -89.0f)
			yawpitch.y = -89.0f;

		return normalize
		(vec3(
			cos(radians(yawpitch.x)) * cos(radians(yawpitch.y)),
			sin(radians(yawpitch).y),
			sin(radians(yawpitch.x)) * cos(radians(yawpitch.y))
		)
		);
	}
};

