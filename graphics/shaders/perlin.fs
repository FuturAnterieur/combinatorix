#version 330

uniform float time;
in vec2 uv;
in vec4 color;
out vec4 frag_color;

float rand( in vec2 p )
{
	float shape = sin(p.x) * sin(p.y);
	return shape;
}

float fbm(vec2 p)
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

float domainwarp(in vec2 p, out vec2 q, out vec2 r)
{
	float t = time/2.0;
	q = vec2( fbm( p + vec2(sin(t), 0.0) ),      fbm( p + vec2(5.2, 1.3 - sin(t)) ) );
	r = vec2( fbm( p + 4.0*q + vec2(6.0, -9.0)), fbm( p + 4.0*q + vec2(-1.4, 3.6)));

	float f = 0.65 + fbm( p + 4.0*r );

	f = mix( f, f*f, f*abs(q.y) );

	return f;
}

void main(void)
{
	vec2 p = uv ;
	vec2 q = vec2(0.0, 0.0); vec2 r = vec2(0.0, 0.0);

	float f = domainwarp(p, q, r);
	
	//vec3 col = vec3(f*(q.y - r.y), f*(q.y + r.x), f*(q.x + r.y));
  vec3 col = vec3(0.0);
    col = mix( vec3(0.2,0.1,0.4), vec3(0.3,0.05,0.05), f );
    col = mix( col, vec3(0.9,0.9,0.9), dot(q,q) );
    col = mix( col, vec3(0.4,0.3,0.3), 0.2 + 0.5*r.y*r.y );
    col = mix( col, vec3(0.0,0.2,0.4), 0.5*smoothstep(1.2,1.3,abs(r.x)+abs(r.y)) );
    col = clamp( col*f*2.0, 0.0, 1.0 );
  
	frag_color = vec4(col, 1.0); //vec4(f * (q.y + r.x) * color.rgb, 1.0);
}