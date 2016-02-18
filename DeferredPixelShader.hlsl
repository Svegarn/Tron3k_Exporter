Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D WposMap : register(t2);

SamplerState samplerWrap : register(s0);
SamplerState samplerClamp : register(s1);

struct Light{
	float4 pl_position;
	float4 pl_intensity;
	float4 pl_color;
	float4 pl_radius;
	float4 pl_attenuation;
};

cbuffer cbLight : register(b0)
{
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
	float4 lightDirection;
	float4 mainCamPos;
}

cbuffer cbPointLights : register(b1)
{
	float4 numPointLights;
	Light pointLights[15];
}

cbuffer cbBWLight : register(b2)
{
	float4 bw_position;
	float4 bw_intensity;
	float4 bw_color;
	float4 bw_radius;
	float4 bw_attenuation;
}

cbuffer cbDSLight : register(b3)
{
	float4 ds_position;
	float4 ds_intensity;
	float4 ds_color;
	float4 ds_radius;
	float4 ds_attenuation;
}

cbuffer cbFBWLight : register(b4)
{
	float4 fbw_position;
	float4 fbw_intensity;
	float4 fbw_color;
	float4 fbw_radius;
	float4 fbw_attenuation;
}

struct VS_OUT{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

struct PS_OUT
{
	float4 Bloom;
};

// Declarations
float3 pointLightPhong(float3 color, float3 normal, float3 wpos, float3 f_lightPosition, float3 f_lightColor, float3 f_lightAtt, float f_lightRange, float f_intensity);

// Main
PS_OUT PS_main(VS_OUT input) : SV_Target
{
	PS_OUT output;

	float3 outputColor = { 0.0f, 0.0f, 0.0f };

	float3 color = DiffuseMap.Sample(samplerClamp, input.Tex).rgb;
	float3 normal = NormalMap.Sample(samplerClamp, input.Tex).rgb;
	float3 wpos = WposMap.Sample(samplerClamp, input.Tex).rgb;

	float lightIntensity = max(dot(normal, normalize(-lightDirection.xyz)), 0.2f);
	outputColor += saturate((color + float3(0.15f, 0.15f, 0.25f)) * lightIntensity);

	[unroll]
	for (int i = 0; i < numPointLights.x; i++){
		outputColor += pointLightPhong(color, normal, wpos, pointLights[i].pl_position.xyz, pointLights[i].pl_color.xyz, pointLights[i].pl_attenuation.xyz, pointLights[i].pl_radius.x, pointLights[i].pl_intensity.x);
	}

	[flatten]
	if (bw_radius.x > 0.0f){
		outputColor += pointLightPhong(color, normal, wpos, bw_position.xyz, bw_color.xyz, bw_attenuation.xyz, bw_radius.x, bw_intensity.x);
	}

	[flatten]
	if (ds_radius.x > 0.0f){
		outputColor += pointLightPhong(color, normal, wpos, ds_position.xyz, ds_color.xyz, ds_attenuation.xyz, ds_radius.x, ds_intensity.x);
	}

	[flatten]
	if (fbw_radius.x > 0.0f){
		outputColor += pointLightPhong(color, normal, wpos, fbw_position.xyz, fbw_color.xyz, fbw_attenuation.xyz, fbw_radius.x, fbw_intensity.x);
	}

	output.Bloom = float4(outputColor, 0.0f);

	return output;
}

// Functions
float3 pointLightPhong(float3 color, float3 normal, float3 wpos, float3 f_lightPosition, float3 f_lightColor, float3 f_lightAtt, float f_lightRange, float f_intensity){
	// HardCoded
	float4 specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	float3 finalColor = { 0.0f, 0.0f, 0.0f };
	float3 specularLight;
	float3 lightToPixel = f_lightPosition - wpos;
	float d = length(lightToPixel);

	[flatten]
	if (d > f_lightRange){
		return float3(0.0f, 0.0f, 0.0f);
	}

	float3 n = normalize(normal);

	float howMuchLight = dot(lightToPixel, n);

	[flatten]
	if (howMuchLight > 0.0f){
		finalColor += color * f_lightColor;
		finalColor /= f_lightAtt.x + (f_lightAtt.y * d) + (f_lightAtt.z * (d*d));

		lightToPixel = normalize(lightToPixel);
		float3 v = normalize(mainCamPos.xyz - wpos);
		float3 r = reflect(-lightToPixel, n);
		specularLight = specular.rgb * pow(max(dot(r, v), 1.0f), specular.a);
	}

	return saturate(finalColor * ((howMuchLight / 5) * specularLight));
}