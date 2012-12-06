//Shader for Phong Illuminations and Phong shading

uniform vec3 AmbientContribution,DiffuseContribution,SpecularContribution;
uniform float exponent;
varying vec3 vNormal, vLight, vView, vHalfway;
uniform sampler2D color_texture;
uniform sampler2D bump_map;
uniform bool bumpmapMode;
uniform vec2 resolution;

vec3 AmbientComponent(void)
{
   return vec3(AmbientContribution + 0.1);
}

vec3 DiffuseComponent(void)
{
	vec4 color = texture2D(color_texture, 
   return vec3(color * max(0.0, dot(vNormal, vLight)));
}

vec3 SpecularComponent(void)
{   
      // Approximation to the specular reflection using the halfway vector
      
      return vec3(SpecularContribution * pow(max(0.0, dot(vNormal, vHalfway)), exponent));  
}

vec3 pertNormal(float2 texcoords, float offset)
{
	float A = texture2D(bump_map, texcoords.xy ).x;
	float B = texture2D(bump_map, texcoords.xy + vec2(offset, 0) ).x;
	float C = texture2D(bump_map, texcoords.xy + vec2(0, offset) ).x;

	vec3 N = vec3(B-A,C-A,0.1);
	normalize( N );
	
	return N;
}

/*
===============================================================================
   Phong Shading: Fragment Program
===============================================================================
*/

void main(void)
{
   if (bumpmapMode)
      vNormal += pertNormal(gl_FragCoord.xy / resolution.xy, 0.01);
	  
   // Phong Illumination Model
   
   vec3 color = (AmbientComponent() + DiffuseComponent()) +
                SpecularComponent();  
   // Final color
   
   
   gl_FragColor = vec4(color, 1.0);
}