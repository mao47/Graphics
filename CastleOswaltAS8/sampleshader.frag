//Shader for Phong Illuminations and Phong shading

uniform vec3 AmbientContribution;
uniform vec3 DiffuseContribution;
uniform vec3 SpecularContribution;
uniform float exponent;
varying vec3 vNormal, vLight, vView, vHalfway;
uniform vec2 texCoord;
uniform sampler2D texture;
uniform sampler2D bumpMap;
uniform bool bumpmapMode;
uniform vec2 resolution;
uniform bool envmapMode;
vec3 realNormal;

vec2 EnvMap(void)
{
	float PI = 3.14159265358979323846264;
	vec3 N = normalize(vNormal);
	vec3 L = normalize(vLight);
	vec3 R = normalize(N - 2 * N * dot(N, L));
	vec2 tex = vec2(-(atan(R.z/R.x) / (2*PI) + PI/2), -(acos(R.y)/PI));
	return tex;
}

vec3 AmbientComponent(void)
{
   return vec3(AmbientContribution + 0.1);
}

vec3 DiffuseComponent(void)
{
   vec3 difcolor = texture2D(texture, texCoord).rgb;
   return vec3(difcolor * max(0.0, dot(realNormal, vLight)));
}

vec3 SpecularComponent(void)
{   
      // Approximation to the specular reflection using the halfway vector
      
      return vec3(SpecularContribution * pow(max(0.0, dot(realNormal, vHalfway)), exponent));  
}

vec3 pertNormal(float offset)
{
   //vec2 coords = texCoord.xy / resolution.xy;
   vec2 coords= texCoord.xy;
   float A = texture2D(texture, coords.xy ).x;
   float B = texture2D(texture, coords.xy + vec2(offset, 0) ).x;
   float C = texture2D(texture, coords.xy + vec2(0, offset) ).x;

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
	//texCoord = gl_Multi
	normalize(vNormal);
	normalize(vLight);
	normalize(vView);
	normalize(vHalfway);
   realNormal = vNormal;
   if (bumpmapMode)
   {
      realNormal += pertNormal(0.01);
   }  
   if(envmapMode)
   {
		texCoord = EnvMap();
   }
   
   // Phong Illumination Model
   
   vec3 color = (AmbientComponent() + DiffuseComponent() + SpecularComponent());
   // Final color
   
   
   gl_FragColor = vec4(color, 1.0);
}