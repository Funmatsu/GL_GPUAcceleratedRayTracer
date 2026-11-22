#pragma once
#include "glGeometry.h"

struct glTriad {
	vec4 p[3];
	vec4 ambtcol, diffcol, speccol;
	vec4 phongcoef;
	vec4 material;
	glTriad(vec4 points[3], vec4 ambient_color, vec4 diffuse_color, vec4 specular_color) {
		memcpy(p, points, sizeof(vec4) * 3);
		ambtcol = ambient_color,
			diffcol = diffuse_color,
			speccol = specular_color;
	}
};

struct glQuad
{
	vec4 p[4];
	vec4 ambtcol, diffcol, speccol;
	vec4 phongcoef;
	vec4 material;
	glQuad(vec4 points[4], vec4 ambient_color, vec4 diffuse_color, vec4 specular_color) {
		memcpy(p, points, sizeof(vec4) * 4);
		ambtcol = ambient_color,
		diffcol = diffuse_color,
		speccol = specular_color;
	}
};