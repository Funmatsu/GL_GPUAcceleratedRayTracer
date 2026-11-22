#version 430 core

layout(location = 0) in vec3 pos;
out vec3 col;

uniform mat4 modlmat;
uniform mat4 projmat;

void main(){
	gl_Position = projmat * modlmat * vec4(pos, 1);
	col = vec3(0, 0.3, 1);
}