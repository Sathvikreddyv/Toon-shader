#version 400            
uniform mat4 PVM;
uniform mat4 M;
uniform float time;

uniform vec4 eyew = vec4(0.0f,0.0f,3.0f,0.0f);

in vec3 pos_attrib; //this variable holds the position of mesh vertices
in vec2 tex_coord_attrib;
in vec3 normal_attrib;  

out vec2 tex_coord;
out vec3 normal; 
out vec3 p;
out vec4 pw;
out vec3 nw;
out vec4 vw; // view

void main(void)
{
	vec3 P = pos_attrib;
	P.z += 0.02*sin(15.0*P.x+10.0*time);
	gl_Position = PVM*vec4(P, 1.0); //transform vertices and send result into pipeline
	vec4 PW = M*vec4(p,1.0);
	tex_coord = tex_coord_attrib; //send tex_coord to fragment shad er
	vec3 NW = vec3(normalize(M*vec4(normal_attrib,0.0)));

	vec4 View = normalize(eyew - PW);
	
	p = P;
	pw = PW;
	nw = NW;
	vw = View;
}