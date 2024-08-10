#version 330

uniform float time;
in vec3 uvw;
in vec4 color;
out vec4 frag_color;

float rand( in vec3 p )
{
	float shape = sin(p.x) * sin(p.y) * sin(p.z);
	return shape;
}

float fbm(vec3 p)
{
	float f = 0.0;
	float w = 0.5;
	
	for (int i = 0; i < 5; i++)
	{
		f += w*rand(p);
		w *= 0.5;
		p *= 2.0;
	}
	return f;
}

float domainwarp(in vec3 p, out vec3 q, out vec3 r)
{
	float t = time/2.0;
	q = vec3( fbm( p + vec3(sin(2.0 * t), 0.0, 3.4) ),      fbm( p + vec3(5.2, 1.3 - sin(2.0 * t), 3.4) ), fbm(p + vec3(1.0, 2.0, 3.0)) );
	r = vec3( fbm( p + 4.0*q + vec3(6.0, -9.0, 5.0)),      fbm( p + 4.0*q + vec3(-1.4, 3.6, 2.0)),       fbm(p));

	float f = 0.65 + fbm( p + 4.0*r );

	f = mix( f, f*f, f*abs(q.y) );

	return f;
}

void main(void)
{
	vec3 p = uvw ;
	vec3 q = vec3(0.0, 0.0,0.0); vec3 r = vec3(0.0, 0.0,0.0);

	float f = domainwarp(p, q, r);
	
	//vec3 col = vec3(f*(q.y - r.y), f*(q.y + r.x), f*(q.x + r.y));
  vec3 col = vec3(0.0);
    col = mix( vec3(0.2,0.1,0.4), vec3(0.3,0.05,0.05), f );
    col = mix( col, vec3(0.9,0.9,0.9), dot(q,q) );
    col = mix( col, vec3(0.4,0.3,0.3), 0.2 + 0.5*r.y*r.z );
    col = mix( col, vec3(0.0,0.2,0.4), 0.5*smoothstep(1.2,1.3,abs(r.x)+abs(r.y)) );
    col = clamp( col*f*2.0, 0.0, 1.0 );
  
	frag_color = vec4(col.bgr, 1.0); //vec4(f * (q.y + r.x) * color.rgb, 1.0);
}