#version 330 core

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_color;
layout(location=2) in vec3 in_normal;

out vec3 vposition;
out vec3 vcolor;
out vec3 vnormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 pos = projection * model * view * vec4(in_position,1.0);
	vcolor = in_color;
	vnormal = in_normal;
	vposition = vec3( pos );
	gl_Position = pos;
}
