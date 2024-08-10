#version 330

uniform float time;
in vec3 uvw;
out vec4 frag_color;

float distance(vec3 p){
  
	vec3 n = floor(p);
	vec3 f = fract(p);
  vec3 g = 1.0 - f;
	
	float minf = min(f.x, min(f.y, f.z));
  float ming = min(g.x, min(g.y, g.z));

  return min(minf, ming);
}
  

void main(void)
{
	float f = distance(uvw);
	float nf = 0.5 - f;

  vec3 col = vec3(0.0, nf *nf, nf);

	frag_color = vec4(col, 1.0); 
}