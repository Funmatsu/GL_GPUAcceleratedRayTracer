#version 430 core

out vec4 color;

uniform sampler2D text;

void main(){
	vec2 uv = gl_FragCoord.xy / vec2(1920, 1060);
	color = texture(text, uv);
}