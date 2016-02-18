#version 410
layout (location = 0) in vec2 UV;

uniform int Use;
	
uniform sampler2D Position;
uniform sampler2D Diffuse;
uniform sampler2D Normal;
uniform sampler2D GlowMap;	
uniform sampler2D Depth;

vec4 Position0;
vec4 Diffuse0;
vec4 Normal0;
vec4 Depth0;
vec4 glowValue;

uniform float pixeluvX;
uniform float pixeluvY;

struct SpotLight
{
	vec3 Color;
	float DiffuseIntensity;
	vec3 Position;
	float AmbientIntensity;
	vec3 Direction;
	float Cutoff;
	vec4 attenuation;
};

layout (std140) uniform Light
{ 
	SpotLight lights[500];
};

uniform int NumSpotLights;
uniform vec3 eyepos;

float gSpecularPower = 200;
vec4 specularAddetive;
bool isAmbient = true;

out vec4 fragment_color;
					
vec4 CalcLightInternal(SpotLight l, vec3 LightDirection, vec3 Normal)                   
{     
	vec4 DiffuseColor = vec4(0, 0, 0, 0);                                                                                            
	float DiffuseFactor = dot(Normal, -LightDirection);                                                                     
                                                                                           
	if (DiffuseFactor > 0) 
	{                                                            
		DiffuseColor = vec4(l.Color, 1.0f) * (l.DiffuseIntensity * 3) * DiffuseFactor;    
                                                                                           
		vec3 PosToEye = normalize(eyepos - Position0.xyz);                             
		vec3 LightReflect = normalize(reflect(LightDirection, Normal));                     
		float SpecularFactor = dot(PosToEye, LightReflect);                              
		SpecularFactor = pow(SpecularFactor, gSpecularPower);                               
		if (SpecularFactor > 0)
			if(isAmbient == false)
			{
				float Distance = length(Position0.xyz - l.Position);
				float Attenuation = 1.0 + (0.027 * Distance) + (0.0028 * Distance * Distance);
				specularAddetive += (vec4(l.Color, 1.0f) * ( 1 - Normal0.w) * SpecularFactor) / Attenuation;
			}
			else
			{
				specularAddetive += (vec4(l.Color, 1.0f) * ( 1 - Normal0.w) * SpecularFactor);
			}
	}                                                                                                                                                                         
	return (DiffuseColor);                                   
}               

vec4 CalcPointLight(SpotLight l, vec3 lightToPixel, float distance, float lightFactor, vec3 normal)
{
	float attenuation = 1.0 + (0.14 * distance) + (0.07 * distance * distance);
	float specAtt = 1.0 + (0.045 * distance) + (0.0075 * distance * distance);
	vec3 color = l.Color / attenuation;

	vec3 v = normalize(eyepos - Position0.xyz);
	vec3 r = reflect(normalize(lightToPixel), normal);
	vec3 specular = l.Color * pow(max(dot(r, v), 0.1f), gSpecularPower);
	
	return vec4(clamp((color * lightFactor) + (specular * ( 1 - Normal0.w)) / specAtt, 0.0, 1.0), 1.0f);	
}                                                                                           
                                                                                           
vec4 CalcSpotLight(SpotLight l, vec3 lightToPixel, float distance, float lightFactor, vec3 normal)                                                
{                                                                                           
	vec3 LightToPixel = normalize(Position0.xyz - l.Position);                             
	float SpotFactor = dot(LightToPixel, l.Direction);                                      
    	
	if (SpotFactor > l.Cutoff) 
	{                                                            
		vec4 Color = CalcPointLight(l, lightToPixel, distance, lightFactor, normal);                             
		return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - l.Cutoff));                   
	}                                                                                       
	else                                                                                
		return vec4(0,0,0,0);                                                                   
	
}  		

void main()
{
	fragment_color = vec4(0,0,0,0);
	Diffuse0 = texture(Diffuse, vec2(UV.x, UV.y));
	Position0 = texture(Position, vec2(UV.x, UV.y));
	Normal0 = texture(Normal, vec2(UV.x, UV.y));
	Depth0 = texture(Depth, vec2(UV.x, UV.y));
	glowValue = texture(GlowMap, vec2(UV.x, UV.y));
		
	specularAddetive = vec4(0);
		
	float len = length(Position0.xyz - eyepos);
	if(len < 500)
	{
		// Ambient light (directional)
		fragment_color = CalcLightInternal(lights[0], lights[0].Direction, Normal0.xyz);
		vec4 ambientForce = vec4(lights[0].Color, 1) * lights[0].AmbientIntensity;
		isAmbient = false;
			
		for(int n = 1; n < NumSpotLights; n++)
		{
			vec3 lightToPixel = Position0.xyz - lights[n].Position;
			float distance = length(lightToPixel);
			if (distance < 32)
			{
				vec3 normal = normalize(Normal0.xyz);
				float lightFactor = dot(normal, normalize(-lightToPixel));
				if(lightFactor > 0.0f)
				{
					if(length(lights[n].Direction) < 0.3f)
						fragment_color += CalcPointLight(lights[n], lightToPixel, distance, lightFactor, normal);
					else
						fragment_color += CalcSpotLight(lights[n], lightToPixel, distance, lightFactor, normal);
				}
			}
				
		}
		fragment_color = fragment_color * Diffuse0 + Diffuse0 * ambientForce;
	}
	else
	{
		fragment_color = Diffuse0;
	}
	vec4 sum = vec4(0);
		
	//top left quadrant
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 1 , -pixeluvY * 1 )) * 0.058488;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 1 , -pixeluvY * 2 )) * 0.014662;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 1 , -pixeluvY * 3 )) * 0.001446;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 2 , -pixeluvY * 1 )) * 0.014662;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 2 , -pixeluvY * 2 )) * 0.003676;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 2 , -pixeluvY * 3 )) * 0.000363;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 3 , -pixeluvY * 1 )) * 0.001446;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 3 , -pixeluvY * 2 )) * 0.000363;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 3 , -pixeluvY * 3 )) * 0.000036;

	//top right quadrant   
	sum += texture(GlowMap, UV + vec2( pixeluvX * 1 , -pixeluvY * 1 )) * 0.058488;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 1 , -pixeluvY * 2 )) * 0.014662;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 1 , -pixeluvY * 3 )) * 0.001446;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 2 , -pixeluvY * 1 )) * 0.014662;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 2 , -pixeluvY * 2 )) * 0.003676;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 2 , -pixeluvY * 3 )) * 0.000363;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 3 , -pixeluvY * 1 )) * 0.001446;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 3 , -pixeluvY * 2 )) * 0.000363;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 3 , -pixeluvY * 3 )) * 0.000036;

	//bot left quadrant  
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 1 , pixeluvY * 1 )) * 0.058488;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 1 , pixeluvY * 2 )) * 0.014662;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 1 , pixeluvY * 3 )) * 0.001446;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 2 , pixeluvY * 1 )) * 0.014662;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 2 , pixeluvY * 2 )) * 0.003676;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 2 , pixeluvY * 3 )) * 0.000363;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 3 , pixeluvY * 1 )) * 0.001446;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 3 , pixeluvY * 2 )) * 0.000363;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 3 , pixeluvY * 3 )) * 0.000036;

	//bot left quadrant  
	sum += texture(GlowMap, UV + vec2( pixeluvX * 1 , pixeluvY * 1 )) * 0.058488;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 1 , pixeluvY * 2 )) * 0.014662;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 1 , pixeluvY * 3 )) * 0.001446;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 2 , pixeluvY * 1 )) * 0.014662;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 2 , pixeluvY * 2 )) * 0.003676;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 2 , pixeluvY * 3 )) * 0.000363;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 3 , pixeluvY * 1 )) * 0.001446;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 3 , pixeluvY * 2 )) * 0.000363;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 3 , pixeluvY * 3 )) * 0.000036;

	//Cross samples
	//up
	sum += texture(GlowMap, UV + vec2( 0, -pixeluvY * 1)) * 0.092651;
	sum += texture(GlowMap, UV + vec2( 0, -pixeluvY * 2)) * 0.023226;
	sum += texture(GlowMap, UV + vec2( 0, -pixeluvY * 3)) * 0.002291;

	//left
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 1, 0)) * 0.092651;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 2, 0)) * 0.023226;
	sum += texture(GlowMap, UV + vec2( -pixeluvX * 3, 0)) * 0.002291;

	//right
	sum += texture(GlowMap, UV + vec2( pixeluvX * 1, 0)) * 0.092651;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 2, 0)) * 0.023226;
	sum += texture(GlowMap, UV + vec2( pixeluvX * 3, 0)) * 0.002291;

	//down
	sum += texture(GlowMap, UV + vec2( 0, pixeluvY * 1)) * 0.092651;
	sum += texture(GlowMap, UV + vec2( 0, pixeluvY * 2)) * 0.023226;
	sum += texture(GlowMap, UV + vec2( 0, pixeluvY * 3)) * 0.002291;

	//middle sample
	sum += texture(GlowMap, UV) * 0.146768;

	//mat3 blurMatrix = mat3(
	//	0.077847, 0.123317, 0.077847,
	//	0.123317, 0.195346, 0.123317,
	//	0.077847, 0.123317, 0.077847
	//);
	//
	//vec2 texSize = vec2( 1.0f / 1280.0f, 1.0f / 720.0f );
	//
	//for (int j = 0; j < 3; j++){
	//	for (int i = 0; i < 3; i++){
	//		sum += texture(GlowMap, UV + ((j * i) * texSize)) * blurMatrix[j][i];
	//		sum += texture(GlowMap, UV - ((j * i) * texSize)) * blurMatrix[j][i];
	//	}
	//}
		
	fragment_color += sum + specularAddetive;	
}