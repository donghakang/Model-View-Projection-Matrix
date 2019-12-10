#version 330 core

layout(location=0) out vec4 out_fragcolor;

uniform vec3 eye;
in vec3 vposition;
in vec3 vcolor;
in vec3 vnormal;


//
//	Lights
//
struct DirLight {
	vec3 direction;
	vec3 intensity;
};


//
//	Diffuse color
//
vec3 diffuse(DirLight light, vec3 color, vec3 N){

	vec3 L = -1.f*normalize(light.direction);

	// Material values
	vec3 mamb = color;
	vec3 mdiff = color;

	// Color coeffs
	vec3 ambient = 0.1f * mamb * light.intensity;
	float diff = max( dot(N, L), 0.f );
	vec3 diffuse = diff * mdiff * light.intensity;

	return ( ambient + diffuse );
}

void main(){

	DirLight light;
	light.direction = vposition-eye; // define point light source at eye
	light.intensity = vec3(1,1,1);

	vec3 N = normalize(vnormal);
	vec3 V = normalize(eye-vposition);

	if( dot(N,V) < 0.0 ){ N *= -1.0; } // draw two-sided
	vec3 result = diffuse( light, vcolor, N );
	out_fragcolor = vec4( result, 1.0 );
} 


