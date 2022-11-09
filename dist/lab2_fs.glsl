#version 400
uniform sampler2D diffuse_tex;
uniform float time;
uniform float di; // distance of light source from the object

uniform bool check;

uniform vec4 objectColor;

//ambient terms
uniform vec4 ka; //ambient color of the surface
uniform vec4 la; //ambient light color

//diffuse terms
uniform vec4 kd; // diffuse material color
uniform vec4 ld; // diffuse light color
uniform vec3 Lw = vec3(0.0,1.0,0.0);

//specular terms
uniform vec4 ks; // specular material color
uniform vec4 ls; // speculat light color
uniform float alpha;

in vec2 tex_coord; 

in vec4 pw;
in vec3 nw;//diffuse term
in vec4 vw;

out vec4 fragcolor; //the output color for this fragment    

void main(void)
{   
	
	float d = di*distance(pw.xyz, Lw);
	float atten = 1.0/((d*d)+(1e-6));//attenuation value
	vec3 lw = normalize(Lw-(pw.xyz));
	
	//--------Q6
	vec4 Ia = ka*la;

	//--------Q7
	vec4 Id = atten*kd*ld*max(0.0, dot(nw,lw));

	//--------Q8, Q9, Q10
	vec4 rw = normalize(reflect(vec4(-lw,0.0),vec4(nw,0.0)));
	vec4 Is = atten*ks*ls*pow(max(dot(rw, vw), 0.0), alpha);

	fragcolor = texture(diffuse_tex, tex_coord);

	if(check)
		fragcolor = fragcolor*(Ia + Id + Is);
	else
		//fragcolor = objectColor*(Ia + Id + Is);
		fragcolor = Ia + Id + Is;

}




















