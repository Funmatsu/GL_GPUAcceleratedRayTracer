#pragma once 
#include "glGeometry.h"
struct glSphere
{
	vec4 center;
	vec4 ambtcol, diffcol, speccol;
	vec4 phongcoef;
	vec4 material;
	glSphere(vec3 centre, float r, vec4 ambient_color, vec4 diffuse_color, vec4 specular_color) {
		center = vec4(centre, r),
		ambtcol = ambient_color,
		diffcol = diffuse_color,
		speccol = specular_color;
	}
};