#version 330

uniform float time;
in vec3 uvw;
out vec4 frag_color;

float distance(vec3 p, vec3 b, float e){
  
  p = abs(p) - b;
  vec3 q = abs(p+e)-e;
	
  float valx = length(max(vec3(p.x,q.y,q.z),0.0)) + min(max(p.x,max(q.y,q.z)),0.0);
  float valy = length(max(vec3(q.x,p.y,q.z),0.0)) + min(max(q.x,max(p.y,q.z)),0.0);
  float valz = length(max(vec3(q.x,q.y,p.z),0.0)) + min(max(q.x,max(q.y,p.z)),0.0);

  return min(valx, min(valy, valz));
}

float distance_edge_width_0(vec3 p, vec3 b){
  p = abs(p) - b;
  vec3 q = abs(p);

  float valx = length(max(vec3(p.x,q.y,q.z),0.0));
  float valy = length(max(vec3(q.x,p.y,q.z),0.0));
  float valz = length(max(vec3(q.x,q.y,p.z),0.0));

  return min(valx, min(valy, valz));
}

float rand( in vec3 p )
{
	return cos(length(p));
}

float fbm(in vec3 p)
{
	float f = 0.0;
	float w = 0.5;
  //const mat3 m = mat3( 0.80, 0.60, 0.0, -0.60,  0.80, 0.0, 0.0, 0.0, 1.0 );
	//const mat3 m = mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0); //identity

	for (int i = 0; i < 5; i++)
	{
		f += w*rand(p);
		w *= 0.5;
		p = p * 2.0;
	}
	return f;
}

float domainwarp(in vec3 p, out vec3 q, out vec3 r)
{
  float t = time/2.0;
  p += 3.0*sin( vec3(1.0)*t);
	
  q = vec3(fbm(p), fbm(p), fbm(p));
  vec3 disp = p + 4.0 * q;
	r = vec3(fbm(disp), fbm(disp), fbm(disp));

	float f = fbm(q + r);
	return f;
}

void main(void)
{
  vec3 box_quadrant_dims = vec3(1.0, 1.0, 1.0);
	float d = distance_edge_width_0((uvw), box_quadrant_dims);
	

  float nd = 1.0 / (81.0 * d * d);
  vec3 col_edges = vec3(0.05 * nd, 0.2 * nd, 0.25 * nd);

	vec3 q = vec3(0.0, 0.0,0.0); vec3 r = vec3(0.0, 0.0,0.0);
	float f = domainwarp(uvw, q, r);

   vec3 col = vec3(0.0);
    col = mix( vec3(0.7,0.1,0.4), vec3(0.7,0.5,0.5), f );
    //col = mix( col, vec3(0.9,0.9,0.9), dot(q,q) );
    //col = mix( col, vec3(0.9,0.9,0.9), 0.2*r.x + 0.5*r.y*r.z );
    
    //col = mix( col, vec3(0.0,0.2,0.4), smoothstep(1.2,1.3,abs(r.x)+abs(r.y)) );
    //col = mix( col_edges, col * d, smoothstep(0.2, 0.4, d));

    //adding light normals to the fbm part
    /*float e = 0.005; //epsilon
    vec3 nor = normalize( vec3( domainwarp(uvw+vec3(e,0.0, 0.0), q, r)-f, 
                                domainwarp(uvw+vec3(0.0, e, 0.0), q, r)-f,
                                domainwarp(uvw+vec3(0.0, 0.0, e),q, r)-f ) );

    vec3 lig = normalize( vec3( 0.9, 0.2, -0.4 ) );
    float dif = clamp( 0.3+0.7*dot( nor, lig ), 0.0, 1.0 );
    vec3 lin = vec3(0.70,0.90,0.95)*(nor.y*0.5+0.5) + vec3(0.15,0.10,0.05)*dif;
    col *= 1.2 * lin;*/

    col = mix(col_edges, col, d);
    col = clamp( col * 2.0, 0.0, 1.0 );

	frag_color = vec4(col, 1.0); 
}