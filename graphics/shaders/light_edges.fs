#version 330

uniform float time;
in vec3 uvw;
out vec4 frag_color;

float distance(vec3 p, vec3 b, float e){
  
  p = abs(p) - b;
  vec3 q = abs(p+e)-e;
	
  float valx = length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0);
  float valy = length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0);
  float valz = length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0);

  return min(valx, min(valy, valz));
}

void main(void)
{
	float f = distance((uvw), vec3(1.0, 1.0, 1.0), 0.0);
	float nf = 0.45 - sqrt(f);

  vec3 col = vec3(0.0, nf * 2.0, nf * 2.0);

	frag_color = vec4(col, 1.0); 
}