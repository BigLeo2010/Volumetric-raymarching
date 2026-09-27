#version 330 core
out vec4 FragColor;

in vec3 pos;

uniform float time;

void main(){
	FragColor = vec4(sin(pos.x * time), sin(pos.y * time), 1.0, 1.0);
}
