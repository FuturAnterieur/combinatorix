#version 330
uniform mat4 mvp;
layout(location=0) in vec3 position;
layout(location=1) in vec3 instance_pos;
out vec4 color;
out vec3 uvw;
void main() {
  vec4 pos = vec4(position + instance_pos, 1.0);
  gl_Position = mvp * pos;
  uvw = position;
}